
A Sudoku game solver and generator, 
it deals with puzzles of any size (e.g. 4x4, 9x9, 16x16, etc.).

Build with size 9 (default):
```sh
make target -j SIZE=9
```

Then run with:
```sh
./bin/sudoku solve -i puzzles/1.txt     # solve a puzzle
./bin/sudoku generate -c 24             # generate a puzzle with 24 clues
```

Run benchmarks on included puzzles (time varies depending on difficulties):
```
> ./bin/benchmark
Benchmarking 3 puzzles with 100 repeats...
Puzzle 1:    5 [us]
Puzzle 2:   12 [us]
Puzzle 3:   74 [us]
```

<details>
<summary>
Run on larger datasets.
</summary>

```
> ./bin/benchmark ~/repo/sudoku-dataset/hard_sudokus.txt
Finished on 10000 cases
Success rate: 100%
Mean time: 32 [us]
Median time: 27 [us]
1st quartile time: 20 [us]
3rd quartile time: 39 [us]
Average guesses: 1.9085
Median guesses: 2

> ./bin/benchmark ~/repo/sudoku-dataset/all_17_clue_sudokus.txt
Finished on 49151 cases
Success rate: 100%
Mean time: 92 [us]
Median time: 41 [us]
1st quartile time: 33 [us]
3rd quartile time: 63 [us]
Average guesses: 11.7719
Median guesses: 1

> ./bin/benchmark ~/Downloads/16x16Dataset.csv
Finished on 3000 cases
Success rate: 100%
Mean time: 1105 [us]
Median time: 215 [us]
1st quartile time: 79 [us]
3rd quartile time: 300 [us]
Average guesses: 29.205
Median guesses: 0
```
</details>


Build with `pybind11`:
```sh
SIZE=9 pip install ./bindings
python demo.py -c 24
```

---

Environment variables:
- `SOLVER_USE_GUESS` enable guessing when solving the puzzle. Default is `1`.
- `SOLVER_HEURISTIC_GUESS` enable heuristic choosing of starting cell when guessing. Default is `1`.
- `SOLVER_DETERMINISTIC_GUESS` enable deterministic solving. Default is `0`.
- `SOLVER_USE_DOUBLE` enable naked/hidden double solving. Default is `0`.