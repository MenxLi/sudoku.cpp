
A Sudoku game solver and generator.

For C++ only, build with:
```sh
# specify SIZE=9 for 9x9 board
make target -j SIZE=9
```

Then run with:
```sh
./bin/sudoku solve -i puzzles/1.txt     # solve a puzzle
./bin/sudoku generate -c 24             # generate a puzzle with 24 clues
```

Run benchmarks (time varies depending on difficulties):
```
> ./bin/benchmark
Benchmarking 3 puzzles with 100 repeats...
Puzzle 1:    5 [us]
Puzzle 2:   12 [us]
Puzzle 3:   74 [us]
```

Build with `pybind11`:
```sh
pip install pybind11 && pip install ./bindings
python demo.py
```
For binding usage, please refer to `demo.py`.

<details>
<summary>
Run benchmark on larger datasets:
</summary>

```
> python -m scripts.benchmark /Users/monsoon/Code/repo/sudoku-dataset/hard_sudokus.txt
------------------------------
Solved: 100.00%
Mean time: 36.4472 us
Median time: 32.0 us
Max time: 462 us
Min time: 8 us
1st quartile time: 23.0 us
3rd quartile time: 45.0 us
Mean number of guesses: 1.1677
Median number of guesses: 1.0
------------------------------

> python -m scripts.benchmark /Users/monsoon/Code/repo/sudoku-dataset/all_17_clue_sudokus.txt
------------------------------
Solved: 100.00%
Mean time: 86.91444731541576 us
Median time: 47.0 us
Max time: 25594 us
Min time: 16 us
1st quartile time: 38.0 us
3rd quartile time: 66.0 us
Mean number of guesses: 6.239913735224105
Median number of guesses: 1.0
------------------------------
```

</details>

---

Environment variables:
- `SOLVER_USE_GUESS` enable guessing when solving the puzzle. Default is `1`.
- `SOLVER_HEURISTIC_GUESS` enable heuristic choosing of starting cell when guessing. Default is `1`.
- `SOLVER_DETERMINISTIC_GUESS` enable deterministic solving. Default is `0`.
- `SOLVER_USE_DOUBLE` enable naked/hidden double solving. Default is `0`.