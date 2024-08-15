from sudoku_cpp import sudoku, fmt_board

gen = sudoku.generate(21, 128)
puzzle = gen['data']

solution = sudoku.solve(puzzle)
solved_puzzle = solution['data']

print("Puzzle:")
print(fmt_board(puzzle), end='\n\n')
print("Solution:")
print(fmt_board(solved_puzzle), end='\n\n')
