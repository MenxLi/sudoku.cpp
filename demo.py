from bin.lib import sudoku

puzzle_str = """
8 9 0 2 0 0 0 0 3 
0 0 0 0 7 0 0 8 0 
0 7 5 8 6 9 2 0 0 
0 5 9 0 0 8 0 0 6 
0 0 0 4 0 0 1 9 0 
1 0 0 0 0 0 7 5 0 
0 3 2 0 8 0 0 0 1 
0 8 4 0 2 0 3 0 7 
9 1 7 0 0 6 0 2 0
"""

puzzle = [[int(c) for c in line.split()] for line in puzzle_str.strip().split('\n')]
print(sudoku.solve(puzzle))
