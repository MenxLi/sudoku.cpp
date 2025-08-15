"""
This script generates Sudoku puzzles and their solutions continuously,
yielding them as <puzzle, solution> pairs.
"""

import sudoku_cpp as sudoku
import random

def stream_gen_puzzles(min_clues: int, max_clues: int):
    while(True):
        clues = random.randint(min_clues, max_clues)
        r = sudoku.generate(clues, verbose=False, max_retries=8)
        if r['success']:
            a = sudoku.solve(r['board'])
            yield r['board'], a['board']

if __name__ == "__main__":
    for puzzle, solution in stream_gen_puzzles(18, 36):
        print(f"{puzzle.to_str(sp='', nl='', empty='.')},{solution.to_str(sp='', nl='', empty='.')}")
