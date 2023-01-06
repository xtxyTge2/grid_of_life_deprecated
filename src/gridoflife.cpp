#pragma once
#include <iostream>
#include <vector>

#include "imgui.h"
#include <SDL.h>
#include "imgui_impl_sdl.h"
#include "imgui_impl_sdlrenderer.h"


#if !SDL_VERSION_ATLEAST(2,0,17)
#error This backend requires SDL 2.0.17+ because of SDL_RenderGeometry() function
#endif

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
		drawing_event_queue = std::make_unique<DrawingEventQueue>();
		drawing_window = std::make_unique<DrawingWindow>(width, height, rows, columns);
	}


	void init() {
		// Setup SDL
		// (Some versions of SDL before <2.0.10 appears to have performance/stalling issues on a minority of Windows systems,
		// depending on whether SDL_INIT_GAMECONTROLLER is enabled or disabled.. updating to the latest version of SDL is recommended!)
		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
			printf("Error: %s\n", SDL_GetError());
			return;
		}

		// Setup window
		SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
		window = SDL_CreateWindow("Dear ImGui SDL2+SDL_Renderer example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);

		// Setup SDL_Renderer instance
		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
		if (renderer == NULL) {
			SDL_Log("Error creating SDL_Renderer!");
			return;
		}
		//SDL_RendererInfo info;
		//SDL_GetRendererInfo(renderer, &info);
		//SDL_Log("Current SDL_Renderer: %s", info.name);

		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsLight();

		// Setup Platform/Renderer backends
		ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
		ImGui_ImplSDLRenderer_Init(renderer);

		// Load Fonts
		// - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
		// - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
		// - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
		// - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
		// - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
		// - Read 'docs/FONTS.md' for more instructions and details.
		// - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
		//io.Fonts->AddFontDefault();
		//io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
		//io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
		//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
		//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
		//ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
		//IM_ASSERT(font != NULL);

		update();
	}

	void kill() {
		ImGui_ImplSDLRenderer_Shutdown();
		ImGui_ImplSDL2_Shutdown();
		ImGui::DestroyContext();

		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		SDL_Quit();
	}

	bool loop() {
		// Poll and handle events (inputs, window resize, etc.)
		// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
		// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
		// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
		// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			ImGui_ImplSDL2_ProcessEvent(&event);
			if (event.type == SDL_QUIT)
				return false;
			if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
				return false;
		}

		// Start the Dear ImGui frame
		ImGui_ImplSDLRenderer_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
		if (show_demo_window)
			ImGui::ShowDemoWindow(&show_demo_window);

		// 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
		{
			static float f = 0.0f;
			static int counter = 0;

			ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

			ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
			ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
			ImGui::Checkbox("Another Window", &show_another_window);

			ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
			ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

			if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
				counter++;
			ImGui::SameLine();
			ImGui::Text("counter = %d", counter);

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}

		// 3. Show another simple window.
		if (show_another_window) {
			ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
			ImGui::Text("Hello from another window!");
			if (ImGui::Button("Close Me"))
				show_another_window = false;
			ImGui::End();
		}

		// Rendering
		ImGui::Render();
		SDL_SetRenderDrawColor(renderer, (Uint8)(clear_color.x * 255), (Uint8)(clear_color.y * 255), (Uint8)(clear_color.z * 255), (Uint8)(clear_color.w * 255));
		SDL_RenderClear(renderer);
		ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
		SDL_RenderPresent(renderer);
	}

	void update() {
		drawing_window->drawing_grid->updateGrid();
		iteration++;
		std::cout << "Iteration: " << iteration << " , updating grid." << std::endl;
	}

	void draw() {
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		SDL_RenderClear(renderer);

		drawing_window->append_drawing_events(*drawing_event_queue);
		drawing_event_queue->execute_drawing_events(*renderer);
		// Update window
		SDL_RenderPresent(renderer);
	}

	int width;
	int height;
	int rows;
	int columns;
	int iteration;

	bool show_demo_window = true;
	bool show_another_window = false;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);


	std::unique_ptr<DrawingWindow> drawing_window;
	std::unique_ptr<DrawingEventQueue> drawing_event_queue;

	SDL_Window* window;
	SDL_Renderer* renderer;
};



int main(int argc, char** args) {
	State* state = new State(800, 600, 20, 20);
	state->init();

	while (state->loop()) {
		// wait before processing the next frame
		SDL_Delay(10);
	}

	// Cleanup
	state->kill();

	return 0;
}
