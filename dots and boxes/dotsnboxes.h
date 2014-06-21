#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include <assert.h>

#define TOP 0x1
#define BOTTOM 0x2
#define LEFT 0x4
#define RIGHT 0x8

typedef short square_t;
typedef short wall_t;
typedef struct Turn turn_t;
struct Turn{
	int row, col;
	wall_t wall;
	// linked list of turns (which are valid)
	turn_t* prev;
	turn_t* next;
};
typedef struct Board{
	square_t* squares;
	turn_t* sentinel; // pointer to the sentinel of the doubly linked list of turns
	int rows;
	int cols;
	int player_turn;
	int scores[2];
} board_t;

// usually bad practice, but ok for small code
#define max(a,b) (a) > (b) ? (a) : (b)
#define min(a,b) (a) < (b) ? (a) : (b)

turn_t* make_turn_dll(int r, int c, wall_t wall){
	turn_t* new_turn = (turn_t*) malloc(sizeof(turn_t));
	new_turn->wall = wall; // sentinel value
	new_turn->row = r;
	new_turn->col = c;
	// link to itself
	new_turn->prev = new_turn;
	new_turn->next = new_turn;
	return new_turn;
}

/* insert the 'new' dll node between 'after' and 'after->next' */
void add_turn_dll(turn_t* after, turn_t* new){
	new->next = after->next;
	after->next->prev = new;
	after->next = new;
	new->prev = after;
}

/* remove the given dll entry from the list. return the node before 'turn' such that

	add_turn_dll(remove_turn_dll(turn), turn);

has net-zero-effect */
turn_t* remove_turn_dll(turn_t* turn){
	turn_t* set_to = turn->prev;
	// bypass
	turn->prev->next = turn->next;
	turn->next->prev = turn->prev;
	// loop to self (for later adding)
	turn->next = turn;
	turn->prev = turn;
	return set_to;
}

void stdin_to_board(board_t* empty_board){
	// assuming well-formed inputs
	int rows = 0, cols = 0;
	char c = fgetc(stdin);
	while('0' <= c && c <= '9'){
		rows *= 10;
		rows += c-'0';
		c = fgetc(stdin);
	}
	c = fgetc(stdin);
	while('0' <= c && c <= '9'){
		cols *= 10;
		cols += c-'0';
		c = fgetc(stdin);
	}

	int n_squares = rows * cols;
	empty_board->squares = (square_t*) malloc(sizeof(square_t) * n_squares);
	memset(empty_board->squares, 0, n_squares);

	// create sentinel DLL node
	empty_board->sentinel = make_turn_dll(0, 0, 0);

	// create all other valid turns
	// step 1: left/top for all grid spaces
	for(int r=0; r<rows; r++){
		for(int c=0; c<cols; c++){
			add_turn_dll(empty_board->sentinel, make_turn_dll(r, c, LEFT));
			add_turn_dll(empty_board->sentinel, make_turn_dll(r, c, TOP));
		}
	}
	// step 2: fill in the rightmost walls
	for(int r=0; r<rows; r++)
		add_turn_dll(empty_board->sentinel, make_turn_dll(r, cols-1, RIGHT));
	// step 3: fill in the bottommost walls
	for(int c=0; c<cols; c++)
		add_turn_dll(empty_board->sentinel, make_turn_dll(rows-1, c, BOTTOM));

	empty_board->rows = rows;
	empty_board->cols = cols;
	empty_board->player_turn = 0;
	empty_board->scores[0] = 0;
	empty_board->scores[1] = 0;
}

bool game_is_over(board_t* board){
	// game is over iff only the sentinel is left
	return board->sentinel->next == board->sentinel;
}

int opposite(int r, int c, wall_t typ, board_t* board){
	int i = r*board->cols + c; // flat index
	switch(typ){
	case TOP:
		if(r > 0) return i-board->cols;
		break;
	case BOTTOM:
		if(r < board->rows-1) return i+board->cols;
		break;
	case LEFT:
		if(c > 0) return i-1;
		break;
	case RIGHT:
		if(c < board->cols-1) return i+1;
		break;
	}
	return -1;
}

/* add a wall and return the number of completed boxes*/
int add_wall(int r, int c, wall_t typ, board_t* board){
	int i = r*board->cols + c; // flat index
	board->squares[i] |= typ;

	int j = opposite(r, c, typ, board);
	if(j > -1){
		switch(typ){
		case TOP:
			board->squares[j] |= BOTTOM;
			break;
		case BOTTOM:
			board->squares[j] |= TOP;
			break;
		case LEFT:
			board->squares[j] |= RIGHT;
			break;
		case RIGHT:
			board->squares[j] |= LEFT;
			break;
		}
	}
	return (int)(board->squares[i] == 0xF) + (int) (j > -1 && board->squares[j] == 0xF);
}

/* remove wall and return the number of un-done boxes */
int remove_wall(int r, int c, wall_t typ, board_t* board){
	int i = r*board->cols + c; // flat index
	int j = opposite(r, c, typ, board);
	int undone_boxes = (int)(board->squares[i] == 0xF) + (int)(j > -1 && board->squares[j] == 0xF);
	if(j > -1){
		switch(typ){
		case TOP:
			board->squares[j] &= ~BOTTOM;
			break;
		case BOTTOM:
			board->squares[j] &= ~TOP;
			break;
		case LEFT:
			board->squares[j] &= ~RIGHT;
			break;
		case RIGHT:
			board->squares[j] &= ~LEFT;
			break;
		}
	}
	board->squares[i] &= ~typ;
	return undone_boxes;
}

void print_board(board_t* board){
	for(int r=0; r<board->rows; r++){
		for(int c=0; c<board->cols; c++){
			int i = r*board->cols + c; // flat index
			printf("%d ", board->squares[i]);
		}
		printf("\n");
	}
	printf("%d : %d\n", board->scores[0], board->scores[1]);
}

void execute_turn(turn_t* turn, board_t* board){
	int closed_boxes = add_wall(turn->row, turn->col, turn->wall, board);
	if(closed_boxes > 0){
		board->scores[board->player_turn] += closed_boxes;
	} else{
		board->player_turn = 1 - board->player_turn;
	}
}

void unexecute_turn(turn_t* turn, board_t* board){
	int opened_boxes = remove_wall(turn->row, turn->col, turn->wall, board);
	if(opened_boxes > 0){
		board->scores[board->player_turn] -= opened_boxes;
	} else{
		board->player_turn = 1 - board->player_turn;
	}
}

void cleanup(board_t* board){
	free(board->squares);
	// free DLL
	while(board->sentinel->next != board->sentinel){
		turn_t* rem = board->sentinel->next;
		remove_turn_dll(rem);
		free(rem);
	}
	free(board->sentinel);
}

/* generic printouts at end */
void stats(board_t* board, turn_t* best_turn, int best_outcome, long int count_turns){
	if(best_outcome > 0)
		printf("win\n");
	else if(best_outcome < 0)
		printf("lose\n");
	else
		printf("draw\n");

	char* typ = "";
	switch(best_turn->wall){
	case TOP:
		typ = "TOP";
		break;
	case BOTTOM:
		typ = "BOTTOM";
		break;
	case LEFT:
		typ = "LEFT";
		break;
	case RIGHT:
		typ = "RIGHT";
		break;
	}
	printf("best option: %d %d %s\n", best_turn->row, best_turn->col, typ);
	printf("with score %d\n", best_outcome);

	long int nwalls = board->rows*board->cols*2 + board->rows + board->cols;
	long int fact = 1, s = 0;
	for(long int i=nwalls; i>0; --i){fact *= i; s += fact; }
	printf("%ld walls\n", nwalls);
	printf("%ld search-space branches\n", s);
	printf("%ld turns taken\n", count_turns);
}
