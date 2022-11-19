#include <SDL2/SDL.h>
#include <vector>
#include <random>
#include <iostream>

void render_grid(auto sample_grid, int res, auto renderer);
void line(auto r, SDL_Point a, SDL_Point b);
void square_march(auto sample_grid, auto grid_res, auto renderer);

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
	std::uniform_real_distribution<float> nd(-1, 1);
	std::vector<std::vector<float>> sample_grid;
	sample_grid.resize(col);
	for(int i = 0; i < col; i++) {
		sample_grid[i].resize(row);
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

		render_grid(sample_grid, grid_res, renderer);
		square_march(sample_grid, grid_res, renderer);

		SDL_RenderPresent(renderer);
		SDL_Delay(50);
	}
}

Uint8 lt_zero(auto v) {
	if (v < 0)
		return 0;
	else
		return 1;
}

Uint8 square_type(auto a, auto b, auto c, auto d) {
	return a * 8 + b * 4 + c * 2 + d;

}
Uint8 square_type(auto g, auto i, auto j) {
	return  g[i][j] * 8 + 
			g[i+1][j] * 4 +
			g[i+1][j+1] * 2 +
			g[i][j+1];
}

void square_march(auto g, auto res, auto renderer) {
	int col = g.size();
	for (auto i = 0; i < col - 1; i++) {
		int row = g[i].size();
		for (auto j = 0; j < row - 1; j++) {
			int x = i * res;
			int y = j * res;
			int half = (int) res * 0.5;
			float p0 = g[i][j];
			float p1 = g[i+1][j];
			float p2 = g[i+1][j+1];
			float p3 = g[i][j+1];
			//top, right, bottom, left
			SDL_Point t = {x + half,     y};
			SDL_Point r = {x + res, y + half};
			SDL_Point b = {x + half,     y + res};
			SDL_Point l = {x, 			 y + half};
			Uint8 sqr_config = square_type(lt_zero(p0), lt_zero(p1), lt_zero(p2), lt_zero(p3));
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

void dot_at_coord(auto r, auto x, auto y, auto s) {
	SDL_Rect p = {x - (int)s/2, y - (int)s/2, s, s};
	SDL_RenderFillRect(r, &p);

}

void render_grid(auto sample_grid, int res, auto renderer) {
	for(int i = 0; i < sample_grid.size(); i++) {
		for(int j = 0; j < sample_grid[i].size(); j++) {
			auto sample_val = sample_grid[i][j];
			if (lt_zero(sample_val)) {
				SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
			}
			else {
				SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
			}
			dot_at_coord(renderer, i * res, j * res, 3);
		}
	}
}










