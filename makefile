main: march_square.cpp
	g++ -lSDL2 -std=c++20 march_square.cpp -o squares_marching
clean:
	rm squares_marching
