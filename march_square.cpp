#include <SDL2/SDL.h>
#include <vector>
#include <random>
#include <iostream>
#include <cmath>

void render_grid(auto &sample_grid, int res, auto renderer);
void line(auto r, SDL_Point a, SDL_Point b);
enum class InterpType {None, Linear};
void square_march(auto &g, auto thresh, auto res, InterpType it, auto renderer);
void populate_grid(auto &g, auto &balls, auto res);

struct ball {
	float cx;
	float cy;
	float r;
	float dx;
	float dy;
};

class ScalarEnv2D {
	public:
		virtual void physics_step(int t) = 0;
		virtual void populate_grid(std::vector<std::vector<float>>  &g, int res) = 0;
};

class MetaballEnv : public ScalarEnv2D {
	public:
		std::vector<struct ball> all_ball;
		MetaballEnv(std::random_device &dev, int b_x, int b_y) {
			base_x = b_x;
			base_y = b_y;
			std::uniform_real_distribution<float> rand_x(0, base_x);
			std::uniform_real_distribution<float> rand_y(0, base_y);
			std::uniform_real_distribution<float> rand_r(10, 50);
			std::uniform_real_distribution<float> rand_d(-2, 2);
			for(int i = 0; i < 5; i++){
				struct ball new_ball = {rand_x(dev), rand_y(dev), rand_r(dev), rand_d(dev), rand_d(dev)};
				std::cout << new_ball.cx << "," << new_ball.cy << std::endl;
				all_ball.push_back(new_ball);
			}
		}

		void physics_step(int t) {
			for(int i = 0; i < all_ball.size(); i++) {
				struct ball *b = &all_ball[i];
				b->cx += b->dx;
				b->cy += b->dy;
				if (b->cx < 0 || b->cx > base_x)
					b->dx *= -1;
				if (b->cy < 0 || b->cy > base_y)
					b->dy *= -1;
			}
		}

		void populate_grid(std::vector<std::vector<float>> &g, int res) {
			for (int i = 0; i < g.size(); i++) {
				auto m = g[i];
				for (int j = 0; j < m.size(); j++) {
					float x0 = i * res;
					float y0 = j * res;
					float d = 0;
					for(int k = 0; k < all_ball.size(); k++) {
						struct ball b = all_ball[k];
						d += 1/ball_dist(x0, y0, b);
					}
					d = d * 40 - 1;
					g[i][j] = d;
				}
			}

		}
	private:
		int base_x;
		int base_y;
		static float ball_dist(float x, float y, struct ball m) {
			float a = x - m.cx;
			float b = y - m.cy;
			return std::sqrt(a*a + b*b) + m.r;
	}
};

//following https://www.youtube.com/watch?v=T46nu5e4pNI
int main()
{
	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;
	SDL_Event e;
	/* SDL_Init(SDL_INIT_EVERYTHING); */
	bool running = true;

	//screen dimensions and SDL initialization
	int scale_x = 1;
	int scale_y = 1;
	int base_x = 500;
	int base_y = 500;
	SDL_CreateWindowAndRenderer(base_x * scale_x, base_y * scale_y, 0, &window, &renderer);
	SDL_RenderSetScale(renderer, scale_x, scale_y);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	//dimensions of sample grid
	int grid_res = 20;
	int col = 1 + base_x/grid_res;
	int row = 1 + base_y/grid_res;

	std::random_device dev;
	std::uniform_real_distribution<float> nd(-1, 1);
	std::vector<std::vector<float>> sample_grid(col);
	for(int i = 0; i < col; i++) {
		sample_grid[i].resize(row);
	}

	ScalarEnv2D *cur_env = new MetaballEnv(dev, base_x, base_y);
	//populate grid
	cur_env->populate_grid(sample_grid, grid_res);

	//type of interpolation to use
	std::vector<InterpType> types = {InterpType::None, InterpType::Linear};
	auto cur_interp = types.begin()+1;

	float thresh_default = 0.0;
	float thresh = thresh_default;
	float d_thresh = 0.125;

	while(running)
	{
		while (SDL_PollEvent(&e))
		{
			if (e.type == SDL_QUIT)
				running = false;
			else if (e.type == SDL_KEYDOWN) {
				switch (e.key.keysym.sym) {
					case SDLK_i:
						cur_interp++;
						if (cur_interp == types.end())
							cur_interp = types.begin();
						break;
					case SDLK_EQUALS:
						thresh += d_thresh;
						break;
					case SDLK_MINUS:
						thresh -= d_thresh;
						break;
					case SDLK_0:
						thresh = thresh_default;
						break;
				}
			}
		}
		//clear the screen
		SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
		SDL_RenderClear(renderer);

		cur_env->physics_step(1);

		cur_env->populate_grid(sample_grid, grid_res);
		render_grid(sample_grid, grid_res, renderer);
		square_march(sample_grid, thresh, grid_res, *cur_interp, renderer);

		SDL_RenderPresent(renderer);
		SDL_Delay(50);
	}
}

Uint8 gte_thresh(auto v, auto h) {
	return (v >= h) ? (1):(0);
}

Uint8 square_type(auto a, auto b, auto c, auto d) {
	return a * 8 + b * 4 + c * 2 + d;

}

void square_march(auto &g, auto thresh, auto res, InterpType it, auto renderer) {
	int col = g.size();
	std::vector<float> int_term;
	for (auto i = 0; i < col - 1; i++) {
		int row = g[i].size();
		for (auto j = 0; j < row - 1; j++) {
			int x = i * res;
			int y = j * res;
			float p0 = g[i][j];
			float p1 = g[i+1][j];
			float p2 = g[i+1][j+1];
			float p3 = g[i][j+1];

			//interpolation terms
			if (it == InterpType::Linear) {
				int_term = {
					(thresh-p0) / (p1 - p0),
					(thresh-p1) / (p2 - p1),
					(thresh-p3) / (p2 - p3),
					(thresh-p0) / (p3 - p0)
				};
			}
			else {
				int_term = {0.5, 0.5, 0.5, 0.5};
			}

			//top, right, bottom, left
			SDL_Point t = {x + (int)(res * int_term[0]), y};
			SDL_Point r = {x + res, y + (int)(res *int_term[1])};
			SDL_Point b = {x + (int)(res * int_term[2]), y + res};
			SDL_Point l = {x,       y + (int)(res *int_term[3])};
			Uint8 sqr_config = square_type(gte_thresh(p0, thresh), gte_thresh(p1, thresh), gte_thresh(p2, thresh), gte_thresh(p3, thresh));
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

void render_grid(auto &sample_grid, int res, auto renderer) {
	for(int i = 0; i < sample_grid.size(); i++) {
		for(int j = 0; j < sample_grid[i].size(); j++) {
			auto val = sample_grid[i][j];
			auto val_clamp = (val <= -1.0) ? -1.0:val;
			val_clamp = (val_clamp >= 1.0) ? 1.0:val_clamp;
			Uint8 col = (val_clamp + 1)/2 * 255;
			SDL_SetRenderDrawColor(renderer, col, col, col, 255);
			dot_at_coord(renderer, i * res, j * res, 3);
		}
	}
}
