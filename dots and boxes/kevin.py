from collections import defaultdict

VERTICAL, HORIZONTAL = 0, 1

#represents a single line segment that can be drawn on the board.
class Line(object):
    def __init__(self, x, y, orientation):
        self.x = x
        self.y = y
        self.orientation = orientation
    def __hash__(self):
        return hash((self.x, self.y, self.orientation))
    def __eq__(self, other):
        if not isinstance(other, Line): return False
        return self.x == other.x and self.y == other.y and self.orientation == other.orientation
    def __repr__(self):
        return "Line({}, {}, {})".format(self.x, self.y, "HORIZONTAL" if self.orientation == HORIZONTAL else "VERTICAL")

class State(object):
    def __init__(self, width, height):
        self.width = width
        self.height = height
        self.whose_turn = 0
        self.scores = {0:0, 1:0}
        self.lines = set()
    def copy(self):
        ret = State(self.width, self.height)
        ret.whose_turn = self.whose_turn
        ret.scores = self.scores.copy()
        ret.lines = self.lines.copy()
        return ret
    #iterate through all lines that can be placed on a blank board.
    def iter_all_lines(self):
        #horizontal lines
        for x in range(self.width):
            for y in range(self.height+1):
                yield Line(x, y, HORIZONTAL)
        #vertical lines
        for x in range(self.width+1):
            for y in range(self.height):
                yield Line(x, y, VERTICAL)
    #iterate through all lines that can be placed on this board, 
    #that haven't already been placed.
    def iter_available_lines(self):
        for line in self.iter_all_lines():
            if line not in self.lines:
                yield line

    #returns the number of points that would be earned by a player placing the line.
    def value(self, line):
        assert line not in self.lines
        all_placed = lambda seq: all(l in self.lines for l in seq)
        if line.orientation == HORIZONTAL:
            #lines composing the box above the line
            lines_above = [
                Line(line.x,   line.y+1, HORIZONTAL), #top
                Line(line.x,   line.y,   VERTICAL),   #left
                Line(line.x+1, line.y,   VERTICAL),   #right
            ]
            #lines composing the box below the line
            lines_below = [
                Line(line.x,   line.y-1, HORIZONTAL), #bottom
                Line(line.x,   line.y-1, VERTICAL),   #left
                Line(line.x+1, line.y-1, VERTICAL),   #right
            ]
            return all_placed(lines_above) + all_placed(lines_below)
        else:
            #lines composing the box to the left of the line
            lines_left = [
                Line(line.x-1, line.y+1, HORIZONTAL), #top
                Line(line.x-1, line.y,   HORIZONTAL), #bottom
                Line(line.x-1, line.y,   VERTICAL),   #left
            ]
            #lines composing the box to the right of the line
            lines_right = [
                Line(line.x,   line.y+1, HORIZONTAL), #top
                Line(line.x,   line.y,   HORIZONTAL), #bottom
                Line(line.x+1, line.y,   VERTICAL),   #right
            ]
            return all_placed(lines_left) + all_placed(lines_right)

    def is_game_over(self):
        #the game is over when no more moves can be made.
        return len(list(self.iter_available_lines())) == 0

    #iterates through all possible moves the current player could make.
    #Because scoring a point lets a player go again, a move can consist of a collection of multiple lines.
    def possible_moves(self):
        for line in self.iter_available_lines():
            if self.value(line) > 0:
                #this line would give us an extra turn.
                #so we create a hypothetical future state with this line already placed, and see what other moves can be made.
                future = self.copy()
                future.lines.add(line)
                if future.is_game_over(): 
                    yield [line]
                else:
                    for future_move in future.possible_moves():
                        yield [line] + future_move
            else:
                yield [line]

    def make_move(self, move):
        for line in move:
            self.scores[self.whose_turn] += self.value(line)
            self.lines.add(line)
        self.whose_turn = 1 - self.whose_turn

    def tuple(self):
        return (tuple(self.lines), tuple(self.scores.items()), self.whose_turn)
    def __hash__(self):
        return hash(self.tuple())
    def __eq__(self, other):
        if not isinstance(other, State): return False
        return self.tuple() == other.tuple()

#function decorator which memorizes previously calculated values.
def memoized(fn):
    answers = {}
    def mem_fn(*args):
        if args not in answers:
            answers[args] = fn(*args)
        return answers[args]
    return mem_fn

#finds the best possible move for the current player.
#returns a (move, value) tuple.
@memoized
def get_best_move(state):
    cur_player = state.whose_turn
    next_player = 1 - state.whose_turn
    if state.is_game_over():
        return (None, state.scores[cur_player] - state.scores[next_player])
    best_move = None
    best_score = float("inf")
    #choose the move that gives our opponent the lowest score
    for move in state.possible_moves():
        future = state.copy()
        future.make_move(move)
        _, score = get_best_move(future)
        if score < best_score:
            best_move = move
            best_score = score
    return [best_move, -best_score]

n = 2
m = 2
s = State(n,m)
best_move, relative_value = get_best_move(s)
if relative_value > 0:
    print("win")
elif relative_value == 0:
    print("draw")
else:
    print("lose")
print best_move
