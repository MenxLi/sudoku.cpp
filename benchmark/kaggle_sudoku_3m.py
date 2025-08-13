"""
This is for benchmarking the Kaggle Sudoku dataset with 3 million entries: 
https://www.kaggle.com/datasets/radcliffe/3-million-sudoku-puzzles-with-ratings/data
"""

import argparse, csv
from pathlib import Path
from typing import Iterable
import functools
import tqdm

import sudoku_cpp as sudoku

def load_sudoku_data(file_path: Path) -> Iterable[tuple[sudoku.Board, sudoku.Board]]:
    as_board = functools.partial(sudoku.Board.from_str, sp='', nl='')
    with open(file_path, 'r') as file:
        reader = csv.reader(file)
        for row in reader:
            if row[0] == 'id': continue    # Skip header row
            yield as_board(row[1]), as_board(row[2])

def benchmark_sudoku(file_path: Path) -> dict:
    """
    Benchmark the Sudoku solver on a dataset of puzzles.
    - file_path: Path to the CSV file containing Sudoku puzzles.
    Returns a dictionary with statistics about the solving process.
    """
    stats = {
        'total_puzzles': 0,
        'solved': 0,
        'calculation_time_us': 0,
        'n_guesses': 0,
    }

    for puzzle, solution in tqdm.tqdm(load_sudoku_data(file_path)):
        stats['total_puzzles'] += 1
        result = sudoku.solve(puzzle)
        success = result['board'] == solution

        stats['solved'] += 1 if success else 0
        stats['calculation_time_us'] += result['time_us']
        stats['n_guesses'] += result['n_guesses']

    return stats

if __name__ == "__main__":

    parser = argparse.ArgumentParser(description="Benchmark Sudoku solver on Kaggle dataset.")
    parser.add_argument('file_path', type=str, help='Path to the CSV file containing Sudoku puzzles.')
    args = parser.parse_args()
    file_path = Path(args.file_path)

    if file_path.exists():
        stats = benchmark_sudoku(file_path)
        print(f"Success rate: {stats['solved'] / stats['total_puzzles'] * 100:.2f}% ({stats['solved']} out of {stats['total_puzzles']})")
        print(f"Average time per puzzle: {stats['calculation_time_us'] / stats['total_puzzles']:.2f}us")
        print(f"Average guesses per puzzle: {stats['n_guesses'] / stats['total_puzzles']:.2f}")
    else:
        print(f"File not found: {file_path}")