#include <iostream>
#include <SDL.h>

using namespace std;
class Grid {
public:
	Grid(int rows1, int columns1) {
		rows = rows1;
		columns = columns1;
		grid_data = new bool[rows * columns];
		prev_grid_data = new bool[rows * columns];
		neighbour_count_data = new int[rows * columns];

		for (int r = 0; r < rows; r++) {
			for (int c = 0; c < columns; c++) {
				grid_data[index(r, c)] = false;
				prev_grid_data[index(r, c)] = false;
				neighbour_count_data[index(r, c)] = false;
			}
		}
	};

	void update_neighbour_count() {
		for (int r = 0; r < rows; r++) {
			for (int c = 0; c < columns; c++) {
				neighbour_count_data[index(r, c)] = 0;
				for (int dr = -1; dr <= 1; dr++) {
					for (int dc = -1; dc <= 1; dc++) {
						int n_r = r + dr;
						int n_c = c + dc;
						if (dr == 0 && dc == 0) continue;
						if (n_r < 0 || n_r >= rows || n_c < 0 || n_c >= columns) continue;
						if (grid_data[index(n_r, n_c)]) {
							neighbour_count_data[index(r, c)] += 1;
						}
					}
				}
			}
		}
	}

	void updateGrid() {
		// Copy grid into previous grid for next iteration.
		for (int r = 0; r < rows; r++) {
			for (int c = 0; c < columns; c++) {
				prev_grid_data[index(r, c)] = grid_data[index(r, c)];
			}
		}

		update_neighbour_count();
		for (int r = 0; r < rows; r++) {
			for (int c = 0; c < columns; c++) {
				int neighbours_count = neighbour_count_data[index(r, c)];
				if (prev_grid_data[index(r, c)]) {
					if (neighbours_count == 2 || neighbours_count == 3) {
						grid_data[index(r, c)] = true;
					} else {
						grid_data[index(r, c)] = false;
					}
				} else {
					if (neighbours_count == 3) {
						grid_data[index(r, c)] = true;
					} else {
						grid_data[index(r, c)] = false;
					}
				}
			}
		}
	}

	int index(int r, int c) {
		return r + c * rows;
	}

	bool get(int r, int c) {
		return grid_data[index(r, c)];
	}

	void set(int r, int c, bool value) {
		grid_data[index(r, c)] = value;
	}
	int rows;
	int columns;
	bool* grid_data;
	int* neighbour_count_data;
	bool* prev_grid_data;
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

		grid_rects = new SDL_Rect[rows * columns];

		int rect_width = width / columns;
		int rect_height = height / rows;

		for (int r = 0; r < rows; r++) {
			for (int c = 0; c < columns; c++) {
				grid_rects[index(r, c)].x = grid_top_left_x + c * rect_width;
				grid_rects[index(r, c)].y = grid_top_left_y + r * rect_height;
				grid_rects[index(r, c)].w = rect_width;
				grid_rects[index(r, c)].h = rect_height;
			}
		}
	}

	int index(int r, int c) {
		return r + c * rows;
	}

	SDL_Rect get(int r, int c) {
		return grid_rects[index(r, c)];
	}

	void draw(SDL_Renderer* renderer) {
		for (int r = 0; r < grid->rows; r++) {
			for (int c = 0; c < grid->columns; c++) {
				SDL_Rect currentRect = grid_rects[index(r, c)];
				//cout << "x: " << currentRect.x << " ,y: " << currentRect.y << " ,w: " << currentRect.w << " ,h: " << currentRect.h << endl;
				if (grid->get(r, c)) {
					SDL_SetRenderDrawColor(renderer,255 , 255, 0, 255);
				} else {
					SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
				}
				SDL_RenderFillRect(renderer, &currentRect);
			}
		}
	}
	SDL_Renderer* renderer;
	int grid_top_left_x;
	int grid_top_left_y;
	int width;
	int height;
	int rows;
	int columns;
	Grid* grid;
	SDL_Rect* grid_rects;
};

class DrawingWindow {
public:
	DrawingWindow(int w, int h, Grid* grid);

	void init() {
		SDL_Rect background_rect_temp = { 0, 0, width, height };
		background_rect = &background_rect_temp;

		int grid_top_left_x = background_rect->x + int(0.2 * background_rect->w);
		int grid_top_left_y = background_rect->y + int(0.2 * background_rect->h);

		int grid_bottom_right_x = background_rect->x + int(0.8 * background_rect->w);
		int grid_bottom_right_y = background_rect->y + int(0.8 * background_rect->h);

		int drawing_grid_width = grid_bottom_right_x - grid_top_left_x;
		int drawing_grid_height = grid_bottom_right_y - grid_top_left_y;

		drawing_grid = new DrawingGrid(grid, grid_top_left_x, grid_top_left_y, drawing_grid_width, drawing_grid_height);
	}

	void draw(SDL_Renderer* renderer) {
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		SDL_RenderFillRect(renderer, background_rect);
		drawing_grid->draw(renderer);
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
		SDL_Rect rect;
		// For mouse rectangle (static to persist between function calls)
		static int mx0 = -1, my0 = -1, mx1 = -1, my1 = -1;

		// Clear the window to white

		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		SDL_RenderClear(renderer);

		// Event loop
		while (SDL_PollEvent(&event) != 0) {
			switch (event.type) {
			case SDL_QUIT:
				return false;
			case SDL_MOUSEBUTTONDOWN:
				mx0 = event.button.x;
				my0 = event.button.y;
				break;
			case SDL_MOUSEMOTION:
				mx1 = event.button.x;
				my1 = event.button.y;
				break;
			case SDL_MOUSEBUTTONUP:
				mx0 = my0 = mx1 = my1 = -1;
				break;
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym) {
				case SDLK_UP:
					break;
				case SDLK_DOWN:
					break;
				case SDLK_RIGHT:
					cout << "Updating grid." << endl;
					//grid->updateGrid();
					//drawing_window->draw(renderer);

					// Update window
					//SDL_RenderPresent(renderer);
					break;
				}
				break;
			}
		}

		// Test key states - this could also be done with events
		//if (keys[SDL_SCANCODE_1]) {
		//	updateGrid();
		//}
		drawing_window->draw(renderer);

		// Update window
		SDL_RenderPresent(renderer);
		return true;
	}


	void kill() {
		// Quit
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		SDL_Quit();
	}

	bool init() {
		if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
			cout << "Error initializing SDL: " << SDL_GetError() << endl;
			system("pause");
			return false;
		}

		window = SDL_CreateWindow("Example", SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED, width, height,
			SDL_WINDOW_SHOWN);
		if (!window) {
			cout << "Error creating window: " << SDL_GetError() << endl;
			system("pause");
			return false;
		}

		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
		if (!renderer) {
			cout << "Error creating renderer: " << SDL_GetError() << endl;
			return false;
		}

		SDL_RenderClear(renderer);

		const int grid_rows = rows;
		const int grid_columns = columns;
		Grid* grid = new Grid(grid_rows, grid_columns);
		
		int x = 2;
		int y = 2;
		grid->set(x, y, true);
		grid->set(x - 1, y - 1, true);

		int a = 10;
		int b = 10;

		grid->set(a - 1, b - 1, true);
		grid->set(a - 1, b + 1, true);
		grid->set(a, b - 1, true);
		grid->set(a, b, true);
		grid->set(a + 1, b, true);
		grid->set(a + 1, b + 1, true);


		drawing_window = new DrawingWindow(width, height, grid);
		//SDL_Rect background_rect = { 0, 0, width, height };
		//drawing_window->set_background_rect(&background_rect);
		drawing_window->init();
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
};
State::State(int width, int height, int rows, int columns)
	: width(width), height(height), rows(rows), columns(columns), window(nullptr), renderer(nullptr),
	drawing_window(nullptr), grid(nullptr) {}


int main(int argc, char** args) {
	State* state = new State(800, 600, 20, 30);
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



