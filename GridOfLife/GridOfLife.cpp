#include <iostream>
#include <SDL.h>

using namespace std;

bool init();
void kill();
bool loop();
void initGrid();
void initGridRects();
void updateGrid();
void drawGrid();

SDL_Window* window;
SDL_Renderer* renderer;

const int grid_rows = 20;
const int grid_columns = 30;

bool grid[grid_rows][grid_columns] = {};
bool prev_grid[grid_rows][grid_columns] = {};

int grid_count_neighbours[grid_rows][grid_columns] = { 0 };

SDL_Rect background_rect;
SDL_Rect total_grid_rect;
SDL_Rect grid_rects[grid_rows][grid_columns];

int main(int argc, char** args) {
	if (!init()) {
		return 1;
	}

	initGrid();
	initGridRects();
	while (loop()) {
		// wait before processing the next frame
		SDL_Delay(10);
	}

	kill();
	return 0;
}

void drawGrid() {
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_RenderFillRect(renderer, &background_rect);

	SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
	SDL_RenderFillRect(renderer, &total_grid_rect);
	for (int r = 0; r < grid_rows; r++) {
		for (int c = 0; c < grid_columns; c++) {
			SDL_Rect currentRect = grid_rects[r][c];
			//cout << "x: " << currentRect.x << " ,y: " << currentRect.y << " ,w: " << currentRect.w << " ,h: " << currentRect.h << endl;
			if (grid[r][c]) {
				SDL_SetRenderDrawColor(renderer, 255, 255, 128, 255);
			} else {
				SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
			}
			SDL_RenderFillRect(renderer, &currentRect);
		}
	}
}

void initGridRects() {
	int rect_width = total_grid_rect.w / grid_columns;
	int rect_height = total_grid_rect.h / grid_rows;
	for (int r = 0; r < grid_rows; r++) {
		for (int c = 0; c < grid_columns; c++) {
			grid_rects[r][c].x = total_grid_rect.x + c * rect_width;
			grid_rects[r][c].y = total_grid_rect.y + r * rect_height;
			grid_rects[r][c].w = rect_width;
			grid_rects[r][c].h = rect_height;
		}
	}
}

void initGrid() {
	for (int r = 0; r < grid_rows; r++) {
		for (int c = 0; c < grid_columns; c++) {
			prev_grid[r][c] = grid[r][c];
		}
	}
	int x = 2;
	int y = 2;
	grid[x][y] = true;
	grid[x - 1][y - 1] = true;

	int a = 10;
	int b = 10;

	grid[a - 1][b - 1] = true;
	grid[a - 1][b + 1] = true;
	grid[a][b - 1] = true;
	grid[a][b] = true;
	grid[a + 1][b] = true;
	grid[a + 1][b + 1] = true;
}


void update_grid_count_neighbours() {
	for (int r = 0; r < grid_rows; r++) {
		for (int c = 0; c < grid_columns; c++) {
			grid_count_neighbours[r][c] = 0;
			for (int dr = -1; dr <= 1; dr++) {
				for (int dc = -1; dc <= 1; dc++) {
					int n_r = r + dr;
					int n_c = c + dc;
					if (dr == 0 && dc == 0) continue;
					if (n_r < 0 || n_r >= grid_rows || n_c < 0 || n_c >= grid_columns) continue;
					if (grid[n_r][n_c]) {
						grid_count_neighbours[r][c] += 1;
					}
				}
			}
		}
	}
}


void updateGrid() {
	// Copy grid into previous grid for next iteration.
	for (int r = 0; r < grid_rows; r++) {
		for (int c = 0; c < grid_columns; c++) {
			prev_grid[r][c] = grid[r][c];
		}
	}

	update_grid_count_neighbours();
	for (int r = 0; r < grid_rows; r++) {
		for (int c = 0; c < grid_columns; c++) {
			int neighbours_count = grid_count_neighbours[r][c];
			if (prev_grid[r][c]) {
				if (neighbours_count == 2 || neighbours_count == 3) {
					grid[r][c] = true;
				} else {
					grid[r][c] = false;
				}
			} else {
				if (neighbours_count == 3) {
					grid[r][c] = true;
				} else {
					grid[r][c] = false;
				}
			}
		}
	}
}

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
				updateGrid();
				drawGrid();

				// Update window
				SDL_RenderPresent(renderer);
				break;
			}
			break;
		}
	}

	// Test key states - this could also be done with events
	//if (keys[SDL_SCANCODE_1]) {
	//	updateGrid();
	//}
	drawGrid();

	// Update window
	SDL_RenderPresent(renderer);
	return true;
}

bool init() {
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		cout << "Error initializing SDL: " << SDL_GetError() << endl;
		system("pause");
		return false;
	}

	window = SDL_CreateWindow("Example", SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED, 800, 600,
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

	background_rect.x = 0;
	background_rect.y = 0;
	background_rect.w = 800;
	background_rect.h = 600;

	int grid_top_left_x = background_rect.x + int(0.2 * background_rect.w);
	int grid_top_left_y = background_rect.y + int(0.2 * background_rect.h);

	int grid_bottom_right_x = background_rect.x + int(0.8 * background_rect.w);
	int grid_bottom_right_y = background_rect.y + int(0.8 * background_rect.h);

	total_grid_rect.x = grid_top_left_x;
	total_grid_rect.y = grid_top_left_y;
	total_grid_rect.w = grid_bottom_right_x - grid_top_left_x;
	total_grid_rect.h = grid_bottom_right_y - grid_top_left_y;
	return true;
}

void kill() {
	// Quit
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}