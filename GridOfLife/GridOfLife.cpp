#include <iostream>
#include <tuple>
#include <SDL.h>

class GridRectangle {
public:
	GridRectangle() {}

	void init(int r, int c) {
		row = r;
		column = c;

		number_of_neighbours = 0;
		is_alive = false;
		prev_is_alive = false;
	}
	int row;
	int column;
	SDL_Rect rect;

	int number_of_neighbours;
	bool is_alive;
	bool prev_is_alive;
};



class Grid {
public:
	Grid(int rows1, int columns1) {
		rows = rows1;
		columns = columns1;

		grid_data = new GridRectangle[rows*columns];

		for (int r = 0; r < rows; r++) {
			for (int c = 0; c < columns; c++) {
				grid_data[index(r, c)].init(r, c);
			}
		}
	};

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

	GridRectangle get(int r, int c) {
		return grid_data[index(r, c)];
	}

	int index(int r, int c) {
		return r + c * rows;
	}

	int rows;
	int columns;
	GridRectangle* grid_data;
	GridRectangle* prev_grid_data;
};
class DrawingGrid {
public:
	DrawingGrid(Grid* grid1, int grid_top_left_x1, int grid_top_left_y1, int width1, int height1) {
		grid = grid1;
		grid_top_left_x = grid_top_left_x1;
		grid_top_left_y = grid_top_left_y1;
		width = width1;
		height = height1;
		rows = grid->rows;
		columns = grid->columns;


		rect_width = width / columns;
		rect_height = height / rows;

		for (int r = 0; r < rows; r++) {
			for (int c = 0; c < columns; c++) {
				grid->grid_data[index(r, c)].rect.x = grid_top_left_x + c * rect_width;
				grid->grid_data[index(r, c)].rect.y = grid_top_left_y + r * rect_height;
				grid->grid_data[index(r, c)].rect.w = rect_width;
				grid->grid_data[index(r, c)].rect.h = rect_height;
			}
		}
	}

	int index(int r, int c) {
		return r + c * rows;
	}


	void draw(SDL_Renderer* renderer) {
		draw_grid_rectangles(renderer);
		draw_grid_lines(renderer);
	}

	void draw_grid_lines(SDL_Renderer* renderer) {
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		for (int r = 1; r < grid->rows; r++) {
			SDL_Rect first_rect_in_row = grid->grid_data[index(r, 0)].rect;
			SDL_Rect last_rect_in_row = grid->grid_data[index(r, grid->columns - 1)].rect;
			SDL_RenderDrawLine(renderer, first_rect_in_row.x, first_rect_in_row.y, last_rect_in_row.x + rect_width, last_rect_in_row.y);
		}

		for (int c = 0; c < grid->columns; c++) {
			SDL_Rect first_rect_in_column = grid->grid_data[index(0, c)].rect;
			SDL_Rect last_rect_in_column = grid->grid_data[index(grid->rows - 1, c)].rect;
			SDL_RenderDrawLine(renderer, first_rect_in_column.x, first_rect_in_column.y, last_rect_in_column.x, last_rect_in_column.y + rect_height);
		}
	}

	void draw_grid_rectangles(SDL_Renderer* renderer) {
		for (int r = 0; r < grid->rows; r++) {
			for (int c = 0; c < grid->columns; c++) {
				SDL_Rect currentRect = grid->grid_data[index(r, c)].rect;
				//cout << "x: " << currentRect.x << " ,y: " << currentRect.y << " ,w: " << currentRect.w << " ,h: " << currentRect.h << endl;
				if (grid->get(r, c).is_alive) {
					SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
				} else {
					SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
				}
				SDL_RenderFillRect(renderer, &currentRect);
			}

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

		return &grid->grid_data[index(r, c)];
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
	Grid* grid;
};

class DrawingWindow {
public:
	DrawingWindow(int w, int h, Grid* grid);

	void init() {
		SDL_Rect background_rect_temp = { 0, 0, width, height };
		background_rect = &background_rect_temp;

		int grid_side_length = std::min(background_rect->w, background_rect->h);

		int grid_top_left_x = background_rect->x + (int)(0.2 * grid_side_length);
		int grid_top_left_y = background_rect->y + (int)(0.2 * grid_side_length);

		int grid_bottom_right_x = background_rect->x + (int)(0.8 * grid_side_length);
		int grid_bottom_right_y = background_rect->y + (int)(0.8 * grid_side_length);


		int drawing_grid_width = grid_bottom_right_x - grid_top_left_x;
		int drawing_grid_height = grid_bottom_right_y - grid_top_left_y;

		drawing_grid = new DrawingGrid(grid, grid_top_left_x, grid_top_left_y, drawing_grid_width, drawing_grid_height);
	}

	void draw(SDL_Renderer* renderer) {
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		SDL_RenderFillRect(renderer, background_rect);
		drawing_grid->draw(renderer);
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

private:
	int width;
	int height;
	Grid* grid;
	SDL_Rect* background_rect;
	DrawingGrid* drawing_grid;
};

DrawingWindow::DrawingWindow(int w, int h, Grid* grid1)
	: width(w), height(h), grid(grid1), background_rect(nullptr), drawing_grid(nullptr) {}


class State {
public:
	State(int width, int height, int rows, int columns);

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
		SDL_Rect mouse_rect;
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
		grid->updateGrid();
		iteration++;
		std::cout << "Iteration: " << iteration << " , updating grid." << std::endl;
	}

	void draw() {
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		SDL_RenderClear(renderer);
		drawing_window->draw(renderer);

		// Update window
		SDL_RenderPresent(renderer);
	}

	void kill() {
		// Quit
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		SDL_Quit();
	}

	bool init() {
		if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
			std::cout << "Error initializing SDL: " << SDL_GetError() << std::endl;
			system("pause");
			return false;
		}

		window = SDL_CreateWindow("Example", SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED, width, height,
			SDL_WINDOW_SHOWN);
		if (!window) {
			std::cout << "Error creating window: " << SDL_GetError() << std::endl;
			system("pause");
			return false;
		}

		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
		if (!renderer) {
			std::cout << "Error creating renderer: " << SDL_GetError() << std::endl;
			return false;
		}

		SDL_RenderClear(renderer);

		const int grid_rows = rows;
		const int grid_columns = columns;

		grid = new Grid(grid_rows, grid_columns);

		drawing_window = new DrawingWindow(width, height, grid);
		drawing_window->init();
		update();
	}
private:
	SDL_Window* window;
	SDL_Renderer* renderer;
	DrawingWindow* drawing_window;
	Grid* grid;
	int width;
	int height;
	int rows;
	int columns;

	int iteration;
};
State::State(int width, int height, int rows, int columns)
	: width(width), height(height), rows(rows), columns(columns), iteration(0), window(nullptr), renderer(nullptr),
	drawing_window(nullptr), grid(nullptr) {}


int main(int argc, char** args) {
	State* state = new State(800, 600, 20, 20);
	if (!state->init()) {
		return 1;
	}

	while (state->loop()) {
		// wait before processing the next frame
		SDL_Delay(10);
	}

	state->kill();
	return 0;
}



