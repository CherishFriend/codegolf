#include "dotsnboxes_symmetries.h"

turn_t* minimax_ab(board_t* board, int maximizer, int* final_value, long int* turn_count, int alpha, int beta){
	// only one base case: all the way to the end. careful with large boards!
	if(game_is_over(board)){
		(*final_value) = board->scores[maximizer] - board->scores[1-maximizer];
		return NULL;
	}
	turn_t* sentinel = board->valid_turns;
	turn_t* best_turn = sentinel;
	bool max = board->player_turn == maximizer;
	int score, best_score = max ? INT_MIN : INT_MAX;
	// loop over all possible turns
	int t = 0;
	for(turn_t* current_turn = sentinel->next; current_turn != sentinel; current_turn = current_turn->next){
		// perform turn, remove it from DLL
		execute_turn(current_turn, board);
		turn_t* memo = splice_turn_dll(current_turn);
		(*turn_count)++;
		// recurse to next level of the tree
		minimax_ab(board, maximizer, &score, turn_count, alpha, beta);
		// recursion done; undo move
		add_turn_dll(memo, current_turn);
		unexecute_turn(current_turn, board);
		if(max){
			// MAX algorithm
			best_turn = score > best_score ? current_turn : best_turn;
			best_score = max(best_score, score);
			alpha = best_score;
		} else{
			// MIN algorithm
			best_turn = score < best_score ? current_turn : best_turn;
			best_score = min(best_score, score);
			beta = best_score;
		}
		if(beta <= alpha) break;
		t++;
	}
	(*final_value) = best_score;
	return best_turn;
}

int main(){
	board_t board;
	stdin_to_board(&board);

	long int count = 0;
	int best_outcome;
	turn_t* best_turn = minimax_ab(&board, 0, &best_outcome, &count, INT_MIN, INT_MAX);

	stats(&board, best_turn, best_outcome, count);

	cleanup(&board);

	return 0;
}