#!/bin/sh

echo "== BRUTE FORCE =="
echo "\ntest 1x1"
time echo "1 1" | ./solver_brute
echo "\ntest 1x2"
time echo "1 2" | ./solver_brute
echo "\ntest 2x2"
time echo "2 2" | ./solver_brute
#time echo "2 3" | ./solver_brute

echo "\n\n== ALPHA BETA =="
echo "\ntest 1x1"
time echo "1 1" | ./solver_ab
echo "\ntest 1x2"
time echo "1 2" | ./solver_ab
echo "\ntest 2x2"
time echo "2 2" | ./solver_ab
echo "\ntest 2x3"
time echo "2 3" | ./solver_ab

echo "\n\n== SIMPLE SYMMETRIES =="
echo "\ntest 1x1"
time echo "1 1" | ./solver_brute_sym
echo "\ntest 1x2"
time echo "1 2" | ./solver_brute_sym
echo "\ntest 2x2"
time echo "2 2" | ./solver_brute_sym
#time echo "2 3" | ./solver_brute_sym

echo "\n\n== ALPHA BETA + SYMMETRIES =="
echo "\ntest 1x1"
time echo "1 1" | ./solver_ab_sym
echo "\ntest 1x2"
time echo "1 2" | ./solver_ab_sym
echo "\ntest 2x2"
time echo "2 2" | ./solver_ab_sym
echo "\ntest 2x3"
time echo "2 3" | ./solver_ab_sym

echo "\n\n== BRUTE FORCE + MEMOIZATION =="
echo "\ntest 1x1"
time echo "1 1" | ./solver_brute_memo
echo "\ntest 1x2"
time echo "1 2" | ./solver_brute_memo
echo "\ntest 2x2"
time echo "2 2" | ./solver_brute_memo
echo "\ntest 2x3"
time echo "2 3" | ./solver_brute_memo

echo "\n\n== ALPHA BETA + MEMOIZATION =="
echo "\ntest 1x1"
time echo "1 1" | ./solver_ab_memo
echo "\ntest 1x2"
time echo "1 2" | ./solver_ab_memo
echo "\ntest 2x2"
time echo "2 2" | ./solver_ab_memo
echo "\ntest 2x3"
time echo "2 3" | ./solver_ab_memo
echo "\ntest 3x3"
time echo "3 3" | ./solver_ab_memo

echo "\n\n== SIMPLE SYMMETRIES + MEMOIZATION =="
echo "\ntest 1x1"
time echo "1 1" | ./solver_brute_sym_memo
echo "\ntest 1x2"
time echo "1 2" | ./solver_brute_sym_memo
echo "\ntest 2x2"
time echo "2 2" | ./solver_brute_sym_memo
echo "\ntest 2x3"
time echo "2 3" | ./solver_brute_sym_memo

echo "\n\n== ALPHA BETA + SYMMETRIES + MEMOIZATION =="
echo "\ntest 1x1"
time echo "1 1" | ./solver_ab_sym_memo
echo "\ntest 1x2"
time echo "1 2" | ./solver_ab_sym_memo
echo "\ntest 2x2"
time echo "2 2" | ./solver_ab_sym_memo
echo "\ntest 2x3"
time echo "2 3" | ./solver_ab_sym_memo
echo "\ntest 3x3"
time echo "3 3" | ./solver_ab_sym_memo
