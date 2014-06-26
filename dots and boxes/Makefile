EXECS = solver_brute solver_ab solver_brute_sym solver_ab_sym solver_brute_memo solver_ab_memo solver_sym_memo solver_ab_sym_memo
CC = gcc
ARGS = -Wall -pedantic -std=c99 -O3

all: $(EXECS)
.PHONY: all

debug: ARGS += -DDEBUG -g
debug: all
.PHONY: debug

%: %.c
	@echo "[compiling $<]"
	$(CC) $(ARGS) -o $@ $<

clean:
	rm $(EXECS)
.PHONY: clean