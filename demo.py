from sudoku_cpp import solve, generate, fmt_board

gen = generate(18)
puzzle = gen['data']
print(gen, end='\n\n')

solution = solve(puzzle)
solved_puzzle = solution['data']
print(solution, end='\n\n')

print("Puzzle:")
print(fmt_board(puzzle), end='\n\n')
print("Solution:")
print(fmt_board(solved_puzzle), end='\n\n')
