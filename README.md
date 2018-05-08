# Polyominoes

Polyominoes puzzle solver that is a generalization of my Kanoodle program: https://github.com/HoustonWeHaveABug/Kanoodle.

This program may handle puzzles with holes, and accepts degenerate solutions including only a part of the pieces. It displays the first solution found and counts the total number of solutions (including rotated/flipped).

It is using Knuth's DLX algorithm to search for solutions, it takes 6.5 seconds on my current computer (Windows 7 Professional - i3 4150 CPU) to find all solutions for 12 pentominoes and a 6x10 grid.

The puzzle layout is defined in input just after the grid size, a dot corresponding to a hole cell and a star to the other cells.

Sample inputs are provided in the repository, these are the files named polyominoes_*.txt.
