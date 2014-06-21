#include "dotsnboxes_symmetries.h"

/* brute-force search of the _entire game tree_.
	at completion, final_value will be the best value for 'maximizer'.
	returns a pointer to the best move. */
turn_t* minimax(board_t* board, int maximizer, int* final_value, long int* turn_count, int depth){
	// only one base case: all the way to the end. careful with large boards!
	if(game_is_over(board)){
		(*final_value) = board->scores[maximizer] - board->scores[1-maximizer];
		return NULL;
	}
	turn_t* best_turn = board->sentinel;
	bool max = board->player_turn == maximizer;
	int score, best_score = max ? INT_MIN : INT_MAX;
	bool symmetries[MAX_SYMMETRIES];
	turn_t* memos[MAX_SYMMETRIES];
	for(int s=1; s<MAX_SYMMETRIES; ++s){
		symmetries[s] = has_symmetry(board, s);
		memos[s] = NULL;
	}
#ifdef DEBUG
	for(int s=0; s<MAX_SYMMETRIES; ++s){
		printf("%d : [", s);
		int i=0;
		for(turn_t* t=board->sentinel->nexts[s]; t != board->sentinel && i<8; t = t->nexts[s], i++){
			printf("(%d,%d,%d), ", t->row, t->col, t->wall);
			if(t->nexts[s] == t){
				printf(" -problem- ");
				break;
			}
		}
		printf("]\n");
	}
#endif
	// loop over all possible turns
	for(turn_t* current_turn = board->sentinel->nexts[0]; current_turn != board->sentinel; current_turn = current_turn->nexts[0]){
		// check if we can prune this turn based on symmetries
		bool current_is_symmetric_to_another_previously_used = false;
		for(int s=1; s<MAX_SYMMETRIES; ++s){
			if(symmetries[s] && turn_in_list(current_turn, s) && current_turn->pairs[s] != NULL){
				// the s'th list started out as empty. if now current_turn is in it, then its symmetry has been played already
#ifdef DEBUG
				turn_t* pair = current_turn->pairs[s];
				for(int i=0; i<depth; ++i) printf(" ");
				printf("~(%d %d %d)~ <=%d=> %d %d %d\n", current_turn->row, current_turn->col, current_turn->wall, s, pair->row, pair->col, pair->wall);
#endif
				current_is_symmetric_to_another_previously_used = true;
				break;
			}
		}
		// opportunity to prune the rest of this subtree if a symmetry has already been played
		if(current_is_symmetric_to_another_previously_used) continue;
		// perform turn, remove it from DLLs
		execute_and_update_lists(current_turn, board, memos);
		// we count all calls of execute_turn for stats on pruning factor
		(*turn_count)++;
#ifdef DEBUG
		for(int i=0; i<depth; ++i) printf(" ");
		printf("%d %d %d\n", current_turn->row, current_turn->col, current_turn->wall);
#endif
		// recurse to next level of the tree (without current_turn as an option anymore)
		minimax(board, maximizer, &score, turn_count, depth+1);
		// recursion done; undo move
		unexecute_and_update_lists(current_turn, board, memos);
		if(max){
			// MAX algorithm
			best_turn = score > best_score ? current_turn : best_turn;
			best_score = max(best_score, score);
		} else{
			// MIN algorithm
			best_turn = score < best_score ? current_turn : best_turn;
			best_score = min(best_score, score);
		}
	}
	(*final_value) = best_score;
	return best_turn;
}

int main(){
	board_t board;
	stdin_to_board(&board);

	long int count = 0;

	int best_outcome;
	turn_t* best_turn = minimax(&board, 0, &best_outcome, &count, 0);

	stats(&board, best_turn, best_outcome, count);

	cleanup(&board);
	
	return 0;
}