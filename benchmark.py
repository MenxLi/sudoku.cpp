from concurrent.futures import ProcessPoolExecutor
from typing import Tuple
import argparse
import numpy
import tqdm
import os

from sudoku_cpp import sudoku

def solve_puzzle(puzzle_str: str) -> Tuple[int, bool]:
    def format_puzzle(puzzle_str: str):
        output = []
        for i in range(9):
            output.append([int(puzzle_str[i*9+j]) for j in range(9)])
        return output
    try:
        res = sudoku.solve(format_puzzle(puzzle_str))
    except Exception as e:
        print(f"Error while solving puzzle: {puzzle_str}, error: {e}")
        return -1, False
    return res['time_us'], res['solved']

def read_input_file(input_file: str)->list[str]:
    """
    The file is supposed to contain a sudoku puzzle compactly represented as a string, 
    where each row is a string of 81 characters.
    013400...
    408109...
    """
    def test_puzzle(puzzle_str: str)->bool:
        puzzle_str = puzzle_str.strip()
        if len(puzzle_str) != 81: return False
        return all(c.isdigit() for c in puzzle_str)

    with open(input_file, 'r') as f:
        puzzle_str = f.read()
        all_puzzles = [l for l in puzzle_str.strip().split('\n') if test_puzzle(l)]
    return all_puzzles

def test_txt(input_file: str):
    all_puzzles = read_input_file(input_file)
    print(f"Read {len(all_puzzles)} puzzles")
    
    all_res = []
    n_workers = os.cpu_count() // 2
    with ProcessPoolExecutor(n_workers) as executor:
        for puzzle_str in tqdm.tqdm(all_puzzles, desc="Submitting tasks"):
            all_res.append(executor.submit(solve_puzzle, puzzle_str))
        all_res = [t.result() for t in tqdm.tqdm(all_res, desc="Collecting results")]
    
    all_times = [r[0] for r in all_res if r[1]]
    all_solved = [r[1] for r in all_res]

    time_mean = numpy.mean(all_times)
    time_median = numpy.median(all_times)
    time_max = numpy.max(all_times)
    time_min = numpy.min(all_times)
    time_1q = numpy.percentile(all_times, 25)
    time_3q = numpy.percentile(all_times, 75)
    print("-"*30)
    print(f"Solved: {sum(all_solved)/len(all_solved)*100:.2f}%")
    print(f"Mean time: {time_mean} us")
    print(f"Median time: {time_median} us")
    print(f"Max time: {time_max} us")
    print(f"Min time: {time_min} us")
    print(f"1st quartile time: {time_1q} us")
    print(f"3rd quartile time: {time_3q} us")
    print("-"*30)

if __name__ == "__main__":
    description = """
    Test sudoku solver, reading puzzles from a file that contains one puzzle per line, 
    where each row is a string of 81 characters, 0 for empty cells. 
    The program uses half of the available CPUs to solve the puzzles in parallel.
    """
    parser = argparse.ArgumentParser(description=description)
    parser.add_argument("input_file", type=str, help="Input file containing sudoku puzzles")
    args = parser.parse_args()
    test_txt(args.input_file)