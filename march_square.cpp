#include <SDL2/SDL.h>
#include <random>
#include <iostream>

void render_grid(auto sample_grid, int c, int r, int res, auto renderer);
void line(auto r, SDL_Point a, SDL_Point b);
void square_march(auto sample_grid, auto col, auto row, auto grid_res, auto renderer);

//following https://www.youtube.com/watch?v=T46nu5e4pNI
int main()
{
	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;
	SDL_Event e;
	/* SDL_Init(SDL_INIT_EVERYTHING); */
	bool running = true;

	//screen dimensions and SDL initialization
	int scaleX = 1;
	int scaleY = 1;
	int baseX = 500;
	int baseY = 500;
	SDL_CreateWindowAndRenderer(baseX * scaleX, baseY * scaleY, 0, &window, &renderer);
	SDL_RenderSetScale(renderer, scaleX, scaleY);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	//dimensions of sample grid
	int grid_res = 20;
	int col = 1 + baseX/grid_res;
	int row = 1 + baseY/grid_res;

	std::random_device dev;
	std::uniform_int_distribution<int> nd(0, 1);
	int **sample_grid = new int *[col];
	for(int i = 0; i < col; i++) {
		sample_grid[i] = new int [row];
	}
	//populate grid
	for (int i = 0; i < col; i++) {
		for (int j = 0; j < row; j++) {
			sample_grid[i][j] = nd(dev);
		}
	}

	while(running)
	{
		while (SDL_PollEvent(&e))
		{
			if (e.type == SDL_QUIT)
				running = false;
		}
		//clear the screen
		SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
		SDL_RenderClear(renderer);

		render_grid(sample_grid, col, row, grid_res, renderer);
	square_march(sample_grid, col, row, grid_res, renderer);

		//Test rendering.
		/* SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); */
		/* SDL_RenderDrawPoint(renderer, 0, 0); */

		SDL_RenderPresent(renderer);
		SDL_Delay(50);
	}
}

Uint8 square_type(auto g, auto i, auto j) {
	return  g[i][j] * 8 + 
			g[i+1][j] * 4 +
			g[i+1][j+1] * 2 +
			g[i][j+1];
}

void square_march(auto sample_grid, auto col, auto row, auto grid_res, auto renderer) {
	for (auto i = 0; i < col - 1; i++) {
		for (auto j = 0; j < row - 1; j++) {
			int x = i * grid_res;
			int y = j * grid_res;
			int half = (int) grid_res*0.5;
			//top, right, bottom, left
			SDL_Point t = {x + half,     y};
			SDL_Point r = {x + grid_res, y + half};
			SDL_Point b = {x + half,     y + grid_res};
			SDL_Point l = {x, 			 y + half};
			Uint8 sqr_config = square_type(sample_grid, i, j);
			/* std::cout << (int)sqr_config << std::endl; */
			switch (sqr_config) {
				case 0:
					break;
				case 1:
				case 14:
					line(renderer, l, b);
					break;
				case 2:
				case 13:
					line(renderer, b, r);
					break;
				case 3:
				case 12:
					line(renderer, l, r);
					break;
				case 4:
				case 11:
					line(renderer, t, r);
					break;
				case 5:
					line(renderer, l, t);
					line(renderer, b, r);
					break;
				case 6:
				case 9:
					line(renderer, t, b);
					break;
				case 7:
				case 8:
					line(renderer, l, t);
					break;
				case 10:
					line(renderer, t, r);
					line(renderer, l, b);
					break;
			}
		}
	}
}

void line(auto r, SDL_Point a, SDL_Point b) {
	SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
	SDL_RenderDrawLine(r, a.x, a.y, b.x, b.y);
}

void render_grid(auto sample_grid, int c, int r, int res, auto renderer) {
	for(int i = 0; i < c; i++) {
		for(int j = 0; j < r; j++) {
			auto sample_val = sample_grid[i][j];
			if (sample_val) {
				SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
			}
			else {
				SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
			}
			SDL_RenderDrawPoint(renderer, i * res, j * res);
		}
	}
}









