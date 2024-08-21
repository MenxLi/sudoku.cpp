from sudoku_cpp import solve, generate, fmt_board

gen = generate(24, parallel_exec=True)
print(gen, end='\n\n')
puzzle = gen['data']
print("Puzzle:")
print(fmt_board(puzzle), end='\n\n')

solution = solve(puzzle)
print(solution, end='\n\n')
solved_puzzle = solution['data']
print("Solution:")
print(fmt_board(solved_puzzle), end='\n\n')