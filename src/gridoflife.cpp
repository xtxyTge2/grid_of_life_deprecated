#pragma once
#include <iostream>
#include <vector>

#include "imgui.h"

#include <SDL.h>


#include "internal_sdl_state.cpp"

class DrawingRectangleEvent {
public:
	// Todo: dont store the rect here but instead pass it as a reference in the draw call. This will eventually be moved elsewhere anyway!
	DrawingRectangleEvent(SDL_Rect* rect, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
		: rectangle(rect), r(r), g(g), b(b), a(a) {};

	void execute_drawing_event(SDL_Renderer& renderer) {
		SDL_SetRenderDrawColor(&renderer, r, g, b, a);
		SDL_RenderFillRect(&renderer, rectangle);
	}

	Uint8 r;
	Uint8 g;
	Uint8 b;
	Uint8 a;

	SDL_Rect* rectangle;
};


class DrawingLineEvent {
public:
	DrawingLineEvent(int x1, int y1, int x2, int y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
		: x1(x1), y1(y1), x2(x2), y2(y2), r(r), g(g), b(b), a(a) {};

	void execute_drawing_event(SDL_Renderer& renderer) {
		SDL_SetRenderDrawColor(&renderer, r, g, b, a);
		SDL_RenderDrawLine(&renderer, x1, y1, x2, y2);
	}

	int x1;
	int y1;

	int x2;
	int y2;

	Uint8 r;
	Uint8 g;
	Uint8 b;
	Uint8 a;
};


class DrawingEventQueue {
public:
	DrawingEventQueue() {
		rectangle_events = new std::vector<DrawingRectangleEvent>;
		line_events = new std::vector<DrawingLineEvent>;
	}
	void execute_drawing_events(SDL_Renderer& renderer) {
		execute_drawing_rectangle_events(renderer);
		execute_drawing_line_events(renderer);
	}
	std::vector<DrawingRectangleEvent>* rectangle_events;
	std::vector<DrawingLineEvent>* line_events;
private:
	void execute_drawing_rectangle_events(SDL_Renderer& renderer) {
		while (!rectangle_events->empty()) {
			DrawingRectangleEvent e = rectangle_events->back();
			rectangle_events->pop_back();
			e.execute_drawing_event(renderer);
		}
	}
	void execute_drawing_line_events(SDL_Renderer& renderer) {
		while (!line_events->empty()) {
			DrawingLineEvent e = line_events->back();
			line_events->pop_back();
			e.execute_drawing_event(renderer);
		}
	}
};

class GridRectangle {
public:
	void init(int r, int c) {
		row = r;
		column = c;
		rect = { 0, 0, 0, 0 };
		number_of_neighbours = 0;
		is_alive = false;
		prev_is_alive = false;
	}

	void append_drawing_events(DrawingEventQueue& event_queue) {
		DrawingRectangleEvent* draw_grid_rect_event = new DrawingRectangleEvent(&rect, 0, 0, 0, 0);;
		if (is_alive) {
			draw_grid_rect_event->r = 255;
			draw_grid_rect_event->g = 255;
			draw_grid_rect_event->b = 0;
			draw_grid_rect_event->a = 255;
		} else {
			draw_grid_rect_event->r = 128;
			draw_grid_rect_event->g = 128;
			draw_grid_rect_event->b = 128;
			draw_grid_rect_event->a = 255;
		}
		event_queue.rectangle_events->push_back(*draw_grid_rect_event);
	}

	int row{ 0 };
	int column{ 0 };
	SDL_Rect rect{ 0, 0, 0, 0 };

	int number_of_neighbours{ 0 };
	bool is_alive{ false };
	bool prev_is_alive{ false };
};

class DrawingGrid {
public:
	DrawingGrid(int x, int y, int width1, int height1, int rows1, int columns1) {
		grid_top_left_x = x;
		grid_top_left_y = y;
		width = width1;
		height = height1;

		rows = rows1;
		columns = columns1;
		grid_data = new GridRectangle[rows * columns];

		rect_width = width / columns;
		rect_height = height / rows;
		for (int r = 0; r < rows; r++) {
			for (int c = 0; c < columns; c++) {
				grid_data[index(r, c)].init(r, c);

				grid_data[index(r, c)].rect.x = grid_top_left_x + c * rect_width;
				grid_data[index(r, c)].rect.y = grid_top_left_y + r * rect_height;
				grid_data[index(r, c)].rect.w = rect_width;
				grid_data[index(r, c)].rect.h = rect_height;
			}
		}
	}

	void update_neighbour_count() {
		for (int r = 0; r < rows; r++) {
			for (int c = 0; c < columns; c++) {
				grid_data[index(r, c)].number_of_neighbours = 0;
				for (int dr = -1; dr <= 1; dr++) {
					for (int dc = -1; dc <= 1; dc++) {
						int n_r = r + dr;
						int n_c = c + dc;
						if (dr == 0 && dc == 0) continue;
						if (n_r < 0 || n_r >= rows || n_c < 0 || n_c >= columns) continue;
						if (grid_data[index(n_r, n_c)].is_alive) {
							grid_data[index(r, c)].number_of_neighbours += 1;
						}
					}
				}
			}
		}
	}

	void flip_state(int r, int c) {
		grid_data[index(r, c)].is_alive = !grid_data[index(r, c)].is_alive;
	}

	void updateGrid() {
		// Copy grid into previous grid for next iteration.
		for (int r = 0; r < rows; r++) {
			for (int c = 0; c < columns; c++) {
				grid_data[index(r, c)].prev_is_alive = grid_data[index(r, c)].is_alive;
			}
		}

		update_neighbour_count();
		for (int r = 0; r < rows; r++) {
			for (int c = 0; c < columns; c++) {
				int neighbours_count = grid_data[index(r, c)].number_of_neighbours;
				if (grid_data[index(r, c)].prev_is_alive) {
					if (neighbours_count == 2 || neighbours_count == 3) {
						grid_data[index(r, c)].is_alive = true;
					} else {
						grid_data[index(r, c)].is_alive = false;
					}
				} else {
					if (neighbours_count == 3) {
						grid_data[index(r, c)].is_alive = true;
					} else {
						grid_data[index(r, c)].is_alive = false;
					}
				}
			}
		}
	}


	GridRectangle* get(int r, int c) {
		return &grid_data[index(r, c)];
	}

	int index(int r, int c) {
		return r + c * rows;
	}

	void append_drawing_events(DrawingEventQueue& event_queue) {
		GridRectangle* current_grid_rectangle = nullptr;
		for (int r = 0; r < rows; r++) {
			for (int c = 0; c < columns; c++) {
				current_grid_rectangle = get(r, c);
				current_grid_rectangle->append_drawing_events(event_queue);
			}
		}

		for (int r = 1; r < rows; r++) {
			SDL_Rect first_rect_in_row = get(r, 0)->rect;
			SDL_Rect last_rect_in_row = get(r, columns - 1)->rect;

			int x1 = first_rect_in_row.x;
			int y1 = first_rect_in_row.y;
			int x2 = last_rect_in_row.x + rect_width;
			int y2 = last_rect_in_row.y;

			DrawingLineEvent* event = new DrawingLineEvent(x1, y1, x2, y2, 0, 0, 0, 255);
			event_queue.line_events->push_back(*event);
		}

		for (int c = 1; c < columns; c++) {
			SDL_Rect first_rect_in_column = get(0, c)->rect;
			SDL_Rect last_rect_in_column = get(rows - 1, c)->rect;

			int x1 = first_rect_in_column.x;
			int y1 = first_rect_in_column.y;
			int x2 = last_rect_in_column.x;
			int y2 = last_rect_in_column.y + rect_height;

			DrawingLineEvent* event = new DrawingLineEvent(x1, y1, x2, y2, 0, 0, 0, 255);
			event_queue.line_events->push_back(*event);
		}
	}

	bool is_inside(int x, int y) {
		return (x >= grid_top_left_x && x <= grid_top_left_x + width) && (y >= grid_top_left_y && y <= grid_top_left_y + height);
	}

	GridRectangle* get_rectangle(int x, int y) {
		if (!is_inside(x, y)) {
			return nullptr;
		}
		int relative_x = x - grid_top_left_x;
		int relative_y = y - grid_top_left_y;

		int r = (int)relative_y / rect_height;
		int c = (int)relative_x / rect_width;

		std::cout << "r: " << r << " c: " << c << std::endl;

		return &grid_data[index(r, c)];
	}

	SDL_Renderer* renderer;
	int grid_top_left_x;
	int grid_top_left_y;

	int width;
	int height;
	int rows;
	int columns;

	int rect_width;
	int rect_height;

	GridRectangle* grid_data;
};

class DrawingWindow {
public:
	DrawingWindow(int width1, int height1, int rows1, int columns1) {
		width = width1;
		height = height1;
		rows = rows1;
		columns = columns1; 
		
		SDL_Rect background_rect_temp = { 0, 0, width, height };
		background_rect = &background_rect_temp;
		

		int grid_side_length = std::min(background_rect->w, background_rect->h);

		int grid_top_left_x = background_rect->x + (int)(0.2 * grid_side_length);
		int grid_top_left_y = background_rect->y + (int)(0.2 * grid_side_length);

		int grid_bottom_right_x = background_rect->x + (int)(0.8 * grid_side_length);
		int grid_bottom_right_y = background_rect->y + (int)(0.8 * grid_side_length);


		int drawing_grid_width = grid_bottom_right_x - grid_top_left_x;
		int drawing_grid_height = grid_bottom_right_y - grid_top_left_y;

		drawing_grid = std::make_unique<DrawingGrid>(grid_top_left_x, grid_top_left_y, drawing_grid_width, drawing_grid_height, rows, columns);
	}

	bool is_inside(int x, int y) {
		return (x >= background_rect->x && x <= background_rect->x + width) && (y >= background_rect->y && y <= background_rect->y + height);
	}

	bool is_inside_grid(int x, int y) {
		return drawing_grid->is_inside(x, y);
	}

	GridRectangle* get_rectangle(int x, int y) {
		if (!is_inside_grid(x, y)) {
			return nullptr;
		}
		return drawing_grid->get_rectangle(x, y);
	}

	void append_drawing_events(DrawingEventQueue& event_queue) {
		std::unique_ptr<DrawingRectangleEvent> draw_background_rect_event = std::make_unique<DrawingRectangleEvent>(background_rect, 255, 255, 255, 255);
		event_queue.rectangle_events->push_back(*draw_background_rect_event);

		drawing_grid->append_drawing_events(event_queue);
	}

	int width;
	int height;
	int rows;
	int columns;
	SDL_Rect* background_rect;
	std::unique_ptr<DrawingGrid> drawing_grid;
};


class State {
public:
	State(int width, int height, int rows, int columns) {
		iteration = 0;
		internal_sdl_state = std::make_unique<InternalSDLState>(width, height);
		drawing_event_queue = std::make_unique<DrawingEventQueue>();
		drawing_window = std::make_unique<DrawingWindow>(width, height, rows, columns);
	}

	~State() {

	}

	void init() {
		update();
	}



	bool loop() {
		static const unsigned char* keys = SDL_GetKeyboardState(NULL);

		SDL_Event event;

		// Clear the window to white
		bool is_inside = false;
		int mouse_x = -1;
		int mouse_y = -1;
		GridRectangle* clickedRectangle = nullptr;

		int r = -1;
		int c = -1;
		// Event loop
		while (SDL_PollEvent(&event) != 0) {
			switch (event.type) {
			case SDL_QUIT:
				return false;
			case SDL_MOUSEBUTTONDOWN:
				break;
			case SDL_MOUSEMOTION:
				break;
			case SDL_MOUSEBUTTONUP:
				mouse_x = event.button.x;
				mouse_y = event.button.y;

				clickedRectangle = drawing_window->get_rectangle(mouse_x, mouse_y);
				if (clickedRectangle) {
					clickedRectangle->is_alive = !clickedRectangle->is_alive;
				}
				draw();
				break;
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym) {
				case SDLK_UP:
					break;
				case SDLK_DOWN:
					break;
				case SDLK_RIGHT:
					update();
					break;
				}
				break;
			}
		}
		draw();
		return true;
	}

	void update() {
		drawing_window->drawing_grid->updateGrid();
		iteration++;
		std::cout << "Iteration: " << iteration << " , updating grid." << std::endl;
	}

	void draw() {
		SDL_SetRenderDrawColor(internal_sdl_state->renderer, 255, 255, 255, 255);
		SDL_RenderClear(internal_sdl_state->renderer);

		drawing_window->append_drawing_events(*drawing_event_queue);
		drawing_event_queue->execute_drawing_events(*internal_sdl_state->renderer);
		// Update window
		SDL_RenderPresent(internal_sdl_state->renderer);
	}


private:
	int width;
	int height;
	int rows;
	int columns;
	int iteration;
	std::unique_ptr<InternalSDLState> internal_sdl_state;
	std::unique_ptr<DrawingWindow> drawing_window;
	std::unique_ptr<DrawingEventQueue> drawing_event_queue;
};


int main(int argc, char** args) {
	State* state = new State(800, 600, 20, 20);
	state->init();

	while (state->loop()) {
		// wait before processing the next frame
		SDL_Delay(10);
	}
	ImGui::ShowDemoWindow();
	return 0;
}

