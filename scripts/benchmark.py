from concurrent.futures import ProcessPoolExecutor
from typing import Tuple
import argparse
import numpy
import tqdm
import os

from sudoku_cpp import build_config, solve

def solve_puzzle(puzzle_str: str) -> Tuple[int, bool]:
    def hex2int(hex_str: str)->int:
        return int(hex_str, 24)

    def format_puzzle(puzzle_str: str)->list[list[int]]:
        output = []
        board_size = build_config()['BOARD_SIZE']
        for i in range(board_size):
            output.append([hex2int(puzzle_str[i*board_size+j]) for j in range(board_size)])
        return output
    try:
        res = solve(format_puzzle(puzzle_str))
    except Exception as e:
        print(f"Error while solving puzzle: {puzzle_str}, error: {e}")
        return -1, False
    return res['time_us'], res['solved'], res['n_guesses']

def read_input_file(input_file: str)->list[str]:
    """
    The file is supposed to contain a sudoku puzzle compactly represented as a hex string.
    where each row is a string of more than CELL_COUNT characters, 
    we only take the first CELL_COUNT characters of each row.
    013400...
    408109...
    """
    BOARD_SIZE = build_config()['BOARD_SIZE']
    cell_count = BOARD_SIZE*BOARD_SIZE
    def fmt_puzzle_str(puzzle_str: str)->str:
        puzzle_str = puzzle_str.strip()
        if len(puzzle_str) < cell_count: return ""
        puzzle_str = puzzle_str[:cell_count].replace('.', '0')
        return puzzle_str

    with open(input_file, 'r') as f:
        puzzle_str = f.read()
        puzzle_str = puzzle_str.replace('.', '0')
        all_puzzles = [f_l for l in puzzle_str.strip().split('\n') if (f_l:=fmt_puzzle_str(l))]
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
    all_n_guesses = [r[2] for r in all_res]

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
    print(f"Mean number of guesses: {numpy.mean(all_n_guesses)}")
    print(f"Median number of guesses: {numpy.median(all_n_guesses)}")
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