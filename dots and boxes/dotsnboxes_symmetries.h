#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include <assert.h>

// types of walls
#define TOP 0x1
#define BOTTOM 0x2
#define LEFT 0x4
#define RIGHT 0x8
#define TOP_OR_BOTTOM   (TOP   | BOTTOM)
#define LEFT_OR_RIGHT   (LEFT  | RIGHT)
#define LEFT_OR_TOP     (LEFT  | TOP)
#define RIGHT_OR_BOTTOM (RIGHT | BOTTOM)
#define RIGHT_OR_TOP    (RIGHT | TOP)
#define LEFT_OR_BOTTOM  (LEFT  | BOTTOM)

// types of symmetries (also indexes into DLL arrays)
// note that index 0 is reserved for the valid-moves list

// first 3 are valid for any shape
#define HORIZONTAL 1
#define VERTICAL   2
#define ROT_180    3
// these 4 are only valid on square boards
#define ROT_90     4
#define ROT_270    5
#define DIAG_TL_BR 6
#define DIAG_TR_BL 7
#define MAX_SYMMETRIES 8

typedef short square_t;
typedef short wall_t;
typedef struct Turn turn_t;
struct Turn{
	int row, col;
	wall_t wall;
	// arrays holding doubly linked list of turns
	turn_t** prevs;
	turn_t** nexts;
	// array of symmetry-pairs
	turn_t** pairs;
	turn_t** inverse_pairs;
};
typedef struct Board{
	square_t* squares;
	turn_t* sentinel; // pointer to the sentinel of _all_ DLLs
	int n_lists;
	int rows;
	int cols;
	int player_turn;
	int scores[2];
} board_t;

// usually bad practice, but ok for small code
#define max(a,b) (a) > (b) ? (a) : (b)
#define min(a,b) (a) < (b) ? (a) : (b)

turn_t* make_turn_dll(int r, int c, wall_t wall, int n_lists){
	turn_t* new_turn = (turn_t*) malloc(sizeof(turn_t));
	new_turn->wall = wall;
	new_turn->row = r;
	new_turn->col = c;
	// make space for each dll pointer
	new_turn->prevs = (turn_t**) malloc(sizeof(turn_t*) * n_lists);
	new_turn->nexts = (turn_t**) malloc(sizeof(turn_t*) * n_lists);
	// make space for pointers to symmetric-pairs
	new_turn->pairs = (turn_t**) malloc(sizeof(turn_t*) * n_lists);
	new_turn->inverse_pairs = (turn_t**) malloc(sizeof(turn_t*) * n_lists);
	// link to itself in each list. set pairs to default (null)
	for(int l=0; l<n_lists; ++l){
		new_turn->prevs[l] = new_turn;
		new_turn->nexts[l] = new_turn;
		new_turn->pairs[l] = NULL;
		new_turn->inverse_pairs[l] = NULL;
	}
	return new_turn;
}

/* insert the 'new' dll node between 'after' and 'after->next' */
void add_turn_dll(int list_id, turn_t* after, turn_t* new){
	new->nexts[list_id] = after->nexts[list_id];
	new->nexts[list_id]->prevs[list_id] = new;
	after->nexts[list_id] = new;
	new->prevs[list_id] = after;
}

/* remove the given dll entry from the specified list. return the node before 'turn' such that

	add_turn_dll(l, splice_turn_dll(l, turn), turn);

has net-zero-effect */
turn_t* splice_turn_dll(int list_id, turn_t* turn){
	turn_t* set_to = turn->prevs[list_id];
	// bypass
	turn->prevs[list_id]->nexts[list_id] = turn->nexts[list_id];
	turn->nexts[list_id]->prevs[list_id] = turn->prevs[list_id];
	// loop to self (for later splicing)
	turn->nexts[list_id] = turn;
	turn->prevs[list_id] = turn;
	return set_to;
}

bool turn_equals(turn_t* a, turn_t* b){
	if(a->row == b->row && a->col == b->col && a->wall == b->wall) return true;
	if(abs(a->row - b->row) + abs(a->col - b->col) > 1) return false;
	// they're not a perfect match, but they're one apart. now we need to check wall directions
	if(a->row < b->row && a->wall == BOTTOM && b->wall == TOP) return true;
	if(a->row > b->row && a->wall == TOP && b->wall == BOTTOM) return true;
	if(a->col < b->col && a->wall == RIGHT && b->wall == LEFT) return true;
	if(a->col > b->col && a->wall == LEFT && b->wall == RIGHT) return true;
	return false;
}

// Symmetries function (writes symmetry into dest)
void sym_horizontal(turn_t* turn, turn_t* dest, board_t* board){
	dest->row = turn->row;
	dest->col = board->cols - turn->col - 1;
	// if mirroring one of top/bottom, copy same.
	// if mirroring one of left/right, flip it
	dest->wall = turn->wall & TOP_OR_BOTTOM ? turn->wall : turn->wall ^ LEFT_OR_RIGHT;
}
void sym_vertical(turn_t* turn, turn_t* dest, board_t* board){
	dest->col = turn->col;
	dest->row = board->rows - turn->row - 1;
	// if mirroring one of left/right, copy same.
	// if mirroring one of top/bottom, flip it
	dest->wall = turn->wall & LEFT_OR_RIGHT ? turn->wall : turn->wall ^ TOP_OR_BOTTOM;
}
void sym_rot_180(turn_t* turn, turn_t* dest, board_t* board){
	dest->row = board->rows - turn->row - 1;
	dest->col = board->cols - turn->col - 1;
	// flip Left/Right or Top/Bottom
	dest->wall = turn->wall & LEFT_OR_RIGHT ? turn->wall ^ LEFT_OR_RIGHT : turn->wall ^ TOP_OR_BOTTOM;
}
void sym_diag_tl_br(turn_t* turn, turn_t* dest, board_t* board){
	dest->row = turn->col;
	dest->col = turn->row;
	// flip Left/Top or Right/Bottom
	dest->wall = turn->wall & LEFT_OR_TOP ? turn->wall ^ LEFT_OR_TOP : turn->wall ^ RIGHT_OR_BOTTOM;
}
void sym_diag_tr_bl(turn_t* turn, turn_t* dest, board_t* board){
	dest->row = board->cols - turn->col - 1;
	dest->col = board->rows - turn->row - 1;
	// flip Left/Bottom or Right/Top
	dest->wall = turn->wall & LEFT_OR_BOTTOM ? turn->wall ^ LEFT_OR_BOTTOM : turn->wall ^ RIGHT_OR_TOP;
}
void sym_rot_90(turn_t* turn, turn_t* dest, board_t* board){
	// composition of diagonal tl/br, then vertical flip
	turn_t temp;
	sym_diag_tl_br(turn, &temp, board);
	sym_vertical(&temp, dest, board);
}
void sym_rot_270(turn_t* turn, turn_t* dest, board_t* board){
	// composition of vertical flip, then diagonal tl/br
	turn_t temp;
	sym_vertical(turn, &temp, board);
	sym_diag_tl_br(&temp, dest, board);
}
void symmetry(int type, turn_t* turn, turn_t* dest, board_t* board){
	switch(type){
	case HORIZONTAL:
		sym_horizontal(turn, dest, board);
		break;
	case VERTICAL:
		sym_vertical(turn, dest, board);
		break;
	case ROT_180:
		sym_rot_180(turn, dest, board);
		break;
	case ROT_90:
		sym_rot_90(turn, dest, board);
		break;
	case ROT_270:
		sym_rot_270(turn, dest, board);
		break;
	case DIAG_TL_BR:
		sym_diag_tl_br(turn, dest, board);
		break;
	case DIAG_TR_BL:
		sym_diag_tr_bl(turn, dest, board);
		break;
	}
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

	empty_board->rows = rows;
	empty_board->cols = cols;
	empty_board->player_turn = 0;
	empty_board->scores[0] = 0;
	empty_board->scores[1] = 0;

	int n_squares = rows * cols;
	empty_board->squares = (square_t*) malloc(sizeof(square_t) * n_squares);
	memset(empty_board->squares, 0, n_squares);

	// there is always the 0th list.
	// square boards have 7 symmetries
	// non-square boards have 3
	empty_board->n_lists = rows == cols ? 8 : 4;

	// create sentinel DLL node
	// (marked as sentinel by having zero as its wall)
	empty_board->sentinel =  make_turn_dll(0, 0, 0, MAX_SYMMETRIES);

	// create all other valid turns
	// step 1: left/top for all grid spaces
	for(int r=0; r<rows; r++){
		for(int c=0; c<cols; c++){
			add_turn_dll(0, empty_board->sentinel, make_turn_dll(r, c, LEFT, MAX_SYMMETRIES));
			add_turn_dll(0, empty_board->sentinel, make_turn_dll(r, c, TOP, MAX_SYMMETRIES));
		}
	}
	// step 2: fill in the rightmost walls
	for(int r=0; r<rows; r++)
		add_turn_dll(0, empty_board->sentinel, make_turn_dll(r, cols-1, RIGHT, MAX_SYMMETRIES));
	// step 3: fill in the bottommost walls
	for(int c=0; c<cols; c++)
		add_turn_dll(0, empty_board->sentinel, make_turn_dll(rows-1, c, BOTTOM, MAX_SYMMETRIES));

	// set up symmetric pairs
	turn_t dummy;
	for(turn_t* t=empty_board -> sentinel->nexts[0]; t != empty_board->sentinel; t = t->nexts[0]){
		for(int sym=1; sym < empty_board->n_lists; ++sym){
			symmetry(sym, t, &dummy, empty_board);
#ifdef DEBUG
			printf("    (%d,%d,%d) <=%d=> (%d,%d,%d)\n", t->row, t->col, t->wall, sym, dummy.row, dummy.col, dummy.wall);
#endif
			// center of the board, things get symmetrical with themselves. check for it here
			if(turn_equals(t, &dummy)){
#ifdef DEBUG
				printf("    self symmetric\n");
#endif
			} else{
				// find its pair (slow, but this function only gets called once, and n^2 << n!)
				for(turn_t* cmp = empty_board -> sentinel->nexts[0]; cmp != empty_board->sentinel; cmp = cmp->nexts[0]){
					if(turn_equals(cmp, &dummy)){
						t->pairs[sym] = cmp;
						cmp->inverse_pairs[sym] = t;
						break;
					}
				}
			}
		}
	}

#ifdef DEBUG
	// sanity-check: assert that all symmetries are their own inverse
	for(turn_t* t=empty_board->sentinel->nexts[0]; t != empty_board->sentinel; t = t->nexts[0]){
		for(int sym=1; sym < empty_board->n_lists; ++sym){
			if(t->pairs[sym] == NULL)
				fprintf(stdout, "(%d,%d,%d) <=%d=> ~NULL~\n", t->row, t->col, t->wall, sym);
			else if(t->pairs[sym]->pairs[sym] == t)
				fprintf(stdout, "(%d,%d,%d) <=%d=> (%d,%d,%d)\n", t->row, t->col, t->wall, sym, t->pairs[sym]->row, t->pairs[sym]->col, t->pairs[sym]->wall);
			else if(t->pairs[sym]->inverse_pairs[sym] == t)
				fprintf(stdout, "(%d,%d,%d)   %d=> (%d,%d,%d)\n", t->row, t->col, t->wall, sym, t->pairs[sym]->row, t->pairs[sym]->col, t->pairs[sym]->wall);
			else
				fprintf(stderr, "SYMMETRY ERROR: (%d,%d,%d) <=%d=> (%d,%d,%d)\n", t->row, t->col, t->wall, sym, t->pairs[sym]->row, t->pairs[sym]->col, t->pairs[sym]->wall);
		}
	}
#endif
}

bool has_symmetry(board_t* board, int sym){
	// here's the key idea - if the list of remaining walls for a given symmetry is empty,
	// then the board currently has that symmetry
	return board->sentinel->nexts[sym] == board->sentinel;
}

bool turn_in_list(turn_t* turn, int list_id){
	// loop to self if it's isolated and hence not in the list
	return turn->nexts[list_id] != turn;
}

bool turn_played(turn_t* turn){
	return !turn_in_list(turn, 0);
}

bool game_is_over(board_t* board){
	// game is over iff total scores is the size of the board
	return board->rows * board->cols == board->scores[0] + board->scores[1];
}

/* get the index of the square on the other side of the specified wall (or -1 if it would be out of bounds) */
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

/* add a wall (make a mark in board->squares) and return the number of completed boxes*/
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
	return board->squares[i] == 0xF + (j > -1 && board->squares[j] == 0xF);
}

/* remove wall (undo add_wall) and return the number of un-done boxes */
int remove_wall(int r, int c, wall_t typ, board_t* board){
	int i = r*board->cols + c; // flat index
	int j = opposite(r, c, typ, board);
	int undone_boxes = board->squares[i] == 0xF + (j > -1 && board->squares[j] == 0xF);
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

void execute_and_update_lists(turn_t* turn, board_t* board, turn_t** memos){
	execute_turn(turn, board);
	// three things: 1) remove this turn from list of turns required for each symmetry
	//               2) remember, for this stack frame, where the current turn fell in each DLL (so it can be undone after recursion)
	//               3) add this turn's symmetric pair(s) to the lists of pairs which need to be played to regain symmetry (if that pair is yet unplayed)
	for(int s=0; s<MAX_SYMMETRIES; ++s){
		if(turn_in_list(turn, s)){
			memos[s] = splice_turn_dll(s, turn);
		}
		if(s > 0 && turn->pairs[s] != NULL && !turn_played(turn->pairs[s])){
			add_turn_dll(s, board->sentinel, turn->pairs[s]);
		}
	}
}

void unexecute_and_update_lists(turn_t* turn, board_t* board, turn_t** memos){
	unexecute_turn(turn, board);
	for(int s=0; s<MAX_SYMMETRIES; ++s){
		if(memos[s] != NULL){
			add_turn_dll(s, memos[s], turn);
		}
		else if(s > 0 && turn->inverse_pairs[s] != NULL && turn_played(turn->inverse_pairs[s])){
			add_turn_dll(s, board->sentinel, turn);
		}
		if(s > 0 && turn->pairs[s] != NULL){
			splice_turn_dll(s, turn->pairs[s]);
		}
	}
}

void cleanup(board_t* board){
	free(board->squares);
	// free DLL
	while(board->sentinel->nexts[0] != board->sentinel){
		turn_t* rem = board->sentinel->nexts[0];
		for(int l=0; l<board->n_lists; ++l)
			splice_turn_dll(l, rem);
		free(rem->nexts);
		free(rem->prevs);
		free(rem->pairs);
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
	printf("%ld visited\n", count_turns);
}