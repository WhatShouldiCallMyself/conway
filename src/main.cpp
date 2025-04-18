#include <iostream>
#include <cstdlib>
#include <cstring>

#include <time.h>
#include <chrono>
#include <thread>
#include <atomic>

#ifndef DEFAULT_GRID_SIZE
#define DEFAULT_GRID_SIZE 50
#endif

short grid_size;
char** grid;
std::atomic<char> input_received('0');

std::thread listen_for_input() {
	std::thread listener([]{
		std::cout << "Press Enter to signal change...\n";
    	std::cin.get();
    	input_received.store('1');
	});

	return listener;
}

void free_grid(char** target, const short& size) {
	for (short i = 0; i < size; i++) free(target[i]);
	free(target);
}

char** deep_copy_grid(char** target, const short& size) {
	char** copy = (char**)malloc(size * sizeof(char*));
	if (!copy) {
		std::cerr << "Failed to create deep copy of grid (alloc failed)!\n";
		free(copy);
		return NULL;
	}
	
	for (short i = 0; i < size; i++) {
		copy[i] = (char*)malloc(size * sizeof(char));
		if (!copy[i]) {
			std::cerr << "Failed to create deep copy of grid (alloc failed)!\n";
			free_grid(copy, i);
			return NULL;
		}

		memcpy(copy[i], target[i], size * sizeof(char));
	}
	
	return copy;
}



int init() {
	srand(time(NULL));
	grid = (char**)malloc(grid_size * sizeof(char*));
	if (grid == NULL) {
		std::cerr << "Failed to initialize grid (memory alloc failed for grid)!\n";
		free(grid);
		return 1;
	}

	for (short i = 0; i < grid_size; i++) {
		grid[i] = (char*)malloc(grid_size * sizeof(char));
		if (grid[i] == NULL) {
			std::cerr << "Failed to initialize grid (memory alloc failed for grid elements)!\n";
			for (short j = 0; j < i; j++) free(grid[j]);
			free(grid);
			return 2;
		}

		for (short j = 0; j < grid_size; j++) grid[i][j] = (rand() % 9 == 0) ? '#' : '.';
	}

	return 0;
}

void print_grid() {
	for (short i = 0; i < grid_size; i++) {
		for (short j = 0; j < grid_size; j++) std::cout << ' ' << grid[i][j];
		std::cout << '\n';
	}
	std::cout << '\n';
}

short get_live_neighbors(const short& row, const short& col) {
	short count = 0;

	for (short i = row - 1; i <= row + 1; i++) {
		if ( (i < 0 )|| (i >= grid_size) ) continue;
		for (short j = col - 1; j <= col + 1; j++) {
			if ( (j < 0) || (j >= grid_size) || (i == row && j == col) ) continue;
			if (grid[i][j] == '#') count++;
		}
	}

	return count;
}

void next_iter() {
	char** copy_grid = deep_copy_grid(grid, grid_size);
	for (short i = 0; i < grid_size; i++) {
		for (short j = 0; j < grid_size; j++) {
			char is_live = copy_grid[i][j];
			short neighbors = get_live_neighbors(i, j);
			
			switch (is_live) {
			case '#':
				grid[i][j] = ((neighbors < 2) || (neighbors > 3)) ? '.' : '#';
				break;
			case '.':
				grid[i][j] = neighbors == 3 ? '#' : '.';
				break;
			}
		}
	}
}

void cleanup() { free_grid(grid, grid_size); }

int main(int argc, char *argv[]) {
	grid_size = (argc >= 2 && atoi(argv[1]) > 0 ? atoi(argv[1]) : DEFAULT_GRID_SIZE);
	int err_code = init();
	if (err_code != 0) { return err_code; }

	std::thread listener = listen_for_input();
	while (input_received.load() != '1') {
		print_grid();
		next_iter();
		std::this_thread::sleep_for(std::chrono::nanoseconds(33333333));
	}

	listener.join();
	cleanup();
	return 0;
}
