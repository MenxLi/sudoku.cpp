"""
This is for benchmarking the Kaggle Sudoku dataset with 3 million entries: 
https://www.kaggle.com/datasets/radcliffe/3-million-sudoku-puzzles-with-ratings/data
"""

import argparse, csv
from pathlib import Path
from typing import Iterable
import tqdm
import sudoku_cpp

def load_sudoku_data(file_path: str):
    def fmt_puzzle(puzzle_str: str) -> Iterable[list[list[int]]]:
        """Convert a string representation of a Sudoku puzzle into a 2D list."""
        t = []
        for i in range(0, 81, 9):
            r = [int(digit) if digit != '.' else 0 for digit in puzzle_str[i:i+9]]
            t.append(r)
        return t

    with open(file_path, 'r') as file:
        reader = csv.reader(file)
        for row in reader:
            if row[0] == 'id':
                continue    # Skip header row
            # Return puzzle and solution
            yield fmt_puzzle(row[1]), fmt_puzzle(row[2])

def benchmark_sudoku(file_path: str):
    """
    Benchmark the Sudoku solver on a dataset of puzzles.
    - file_path: Path to the CSV file containing Sudoku puzzles.
    Returns a dictionary with statistics about the solving process.
    """
    stats = {
        'total_puzzles': 0,
        'solved': 0,
        'unsolved': 0,
    }

    for puzzle, solution in tqdm.tqdm(load_sudoku_data(file_path)):
        stats['total_puzzles'] += 1
        try:
            result = sudoku_cpp.solve(puzzle)
            if result['solved']:
                stats['solved'] += 1
            else:
                stats['unsolved'] += 1
        except Exception as e:
            print(f"Error solving puzzle: {e}")
            stats['unsolved'] += 1

    return stats

if __name__ == "__main__":

    parser = argparse.ArgumentParser(description="Benchmark Sudoku solver on Kaggle dataset.")
    parser.add_argument('file_path', type=str, help='Path to the CSV file containing Sudoku puzzles.')
    args = parser.parse_args()
    file_path = Path(args.file_path)

    if file_path.exists():
        stats = benchmark_sudoku(file_path)
        print(f"Total puzzles: {stats['total_puzzles']}")
        print(f"Solved: {stats['solved']}")
        print(f"Unsolved: {stats['unsolved']}")
    else:
        print(f"File not found: {file_path}")