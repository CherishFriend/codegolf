#include "dotsnboxes_symmetries_memo.h"

/* brute-force search of the _entire game tree_.
	at completion, final_value will be the best value for 'maximizer'.
	returns a pointer to the best move. */
turn_t* minimax(board_t* board, int maximizer, int* final_value, long int* turn_count, int depth){
	// only one base case: all the way to the end. careful with large boards!
	if(game_is_over(board)){
		(*final_value) = board->scores[maximizer] - board->scores[1-maximizer];
		return NULL;
	}
	bool max = board->player_turn == maximizer;
	int starting_score = board->scores[maximizer] - board->scores[1-maximizer];
	// check for memoized solution
	memo_t* save = read_memo(board);
	if(save != NULL){
#ifdef DEBUG
		for(int i=0; i<depth; ++i) printf(" ");
		printf("memo: %d\n", save->value);
#endif
		if(max){
			// if maximizing, then we _add_ swing value to the starting score
			(*final_value) = starting_score + save->value;
		} else{
			// if minimizing, then we _subtract_ swing value to the starting score
			(*final_value) = starting_score - save->value;
		}
		return save->best_move;
	}
	// not memoized... compute solution
	turn_t* best_turn = board->sentinel;
	int score, best_score = max ? INT_MIN : INT_MAX;
	bool symmetries[MAX_SYMMETRIES];
	for(int s=1; s<board->n_lists; ++s){
		symmetries[s] = has_symmetry(board, s);
	}
	// loop over all possible turns
	for(turn_t* current_turn = board->sentinel->nexts[0]; current_turn != board->sentinel; current_turn = current_turn->nexts[0]){
		// check if we can prune this turn based on symmetries
		bool current_is_symmetric_to_another_previously_used = false;
		for(int s=1; s<board->n_lists; ++s){
			if(symmetries[s] && turn_in_list(current_turn, s)){
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
		execute_turn(current_turn, board);
		prepare_symmetry_lists_for_recursion(current_turn, board);
		// we count all calls of execute_turn for stats on pruning factor
		(*turn_count)++;
#ifdef DEBUG
		for(int i=0; i<depth; ++i) printf(" ");
		printf("%d %d %d\n", current_turn->row, current_turn->col, current_turn->wall);
#endif
		// recurse to next level of the tree (without current_turn as an option anymore)
		minimax(board, maximizer, &score, turn_count, depth+1);
		// recursion done; undo move
		unexecute_turn(current_turn, board);
		// undo splice from 0th list (others will have to come outside this loop)
		repair_list(current_turn, 0);
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
	// repair lists to state they were in before this function call
	for(turn_t* current_turn = board->sentinel->nexts[0]; current_turn != board->sentinel; current_turn = current_turn->nexts[0]){
		// undo prepare_symmetry_lists_for_recursion for symmetry lists (list 0 already repaired)
		for(int s=1; s<board->n_lists; ++s){
			repair_list(current_turn, s);
		}
	}
	(*final_value) = best_score;
	// how many total points can be gained from here?
	int swing = max ? best_score - starting_score : starting_score - best_score;
	// memoize
	write_memo(board, swing, best_turn);
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
