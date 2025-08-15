#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef _WIN32
#include <conio.h>
#include <io.h>
#define write _write
#define STDOUT_FILENO 1
#define kbhit _kbhit
#define getch _getch

void usleep(int useconds) {
	const int mseconds = (int)((float)useconds * 0.001f);
	Sleep(mseconds);
}
#else
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

int kbhit() {
	struct termios oldt, newt;
	int ch;
	int oldf;
	
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
	
	ch = getchar();
	
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);
	
	if (ch != EOF) {
		ungetc(ch, stdin);
		return 1;
	}
	
	return 0;
}
#define getch getchar
#endif

static short grid_size = 32;
static char** board;

short get_live_neighbors(const short* ptr_row, const short* ptr_col) {
	short count = 0;
	const short row = *ptr_row;
	const short col = *ptr_col;

	for (short i = row - 1; i <= row + 1; i++) {
		if ( (i < 0 )|| (i >= grid_size) ) continue;
		for (short j = col - 1; j <= col + 1; j++) {
			if ( (j < 0) || (j >= grid_size) || (i == row && j == col) ) continue;
			if ((board[i][j] & 1) == 1) count++;  // Compare with numeric 1
		}
	}

	return count;
}



void init() {
	srand(time(NULL));
	const size_t size = sizeof(char*) * grid_size;
	board = malloc(size);
	if (!board) {
		puts("[conway]: failed to allocate memory!");
		return;
	}

	for (short row = 0; row < grid_size; row++) {
		char* new_column = malloc(sizeof(char) * grid_size); // Fixed: was using 'size'
		if (!new_column) {
			puts("[conway]: failed to allocate memory!");
			return;
		}

		for (short i = 0; i < grid_size; i++) { 
			new_column[i] = (rand() % 3 == 0) ? 1 : 0;  // Changed: '1'/'0' to 1/0
		}
		board[row] = new_column;
	}
}

void step() {
	for (short row = 0; row < grid_size; row++) {
		for (short col = 0; col < grid_size; col++) {
			char current = board[row][col] & 1;
			short neighbors = get_live_neighbors(&row, &col);
			
			char next_state;
			if (current == 0) { 
				next_state = neighbors == 3 ? 1 : 0; 
			} else { 
				next_state = ((neighbors < 2) || (neighbors > 3)) ? 0 : 1; 
			}
			
			board[row][col] = current | (next_state << 1);
		}
	}
	
	for (short row = 0; row < grid_size; row++) {
		for (short col = 0; col < grid_size; col++) {
			board[row][col] >>= 1;
		}
	}
}

void print_board() {
	const int len_output = grid_size * (grid_size * 2 + 1) + 1;
	char output[len_output];

	int index = 0;
	for (short row = 0; row < grid_size; row++) {
		for (short col = 0; col < grid_size; col++) {
			char val = board[row][col];
			output[index++] = val == 0 ? ' ' : '#';  // Changed: '0' to 0
			output[index++] = ' ';
		}
		output[index++] = '\n';
	}

	output[index] = '\0';
	fwrite(output, sizeof(char), len_output - 1, stdout);
}

void cleanup() {
	for (short row = 0; row < grid_size; row++) { free(board[row]); }
	free(board);
}



int main(int argc, char** argv) {
	if (argc >= 2 && atoi(argv[1]) > 0)
		grid_size = atoi(argv[1]);

	init();
	if (!board) { return 1; }

	while (1) {
		if (kbhit()) {
			char key = getch();
			if (key == 27)  // ESC key
				break;
		}

		step();
		print_board();
		usleep(33333);
	}

	cleanup();
	putchar('\n');
	return 0;
}