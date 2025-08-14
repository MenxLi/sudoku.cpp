from sudoku_cpp import solve, generate, fmt_board, build_config
import argparse

if __name__ == "__main__":

    cell_count = build_config()['BOARD_SIZE']**2

    parser = argparse.ArgumentParser()
    parser.add_argument("-c", "--clues", type=int, default=cell_count//2, help="Number of clues")
    args = parser.parse_args()

    gen = generate(args.clues, verbose=True)
    if (not gen['success']):
        exit(f"Failed to generate puzzle with {args.clues} clues.")
    puzzle = gen['board']
    print("Puzzle:")
    print(fmt_board(puzzle))

    solution = solve(puzzle)
    solved_puzzle = solution['board']
    print("Solution:")
    print(fmt_board(solved_puzzle))