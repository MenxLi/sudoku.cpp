
A Sudoku game solver. 

Build with `pybind11`:
```sh
pip install pybind11 && pip install ./bindings
python demo.py
```

For C++ only, build with:
```sh
make target
```

Then run with:
```sh
mkdir -p output

# solve a puzzle by providing the input and output file
./bin/sudoku -i puzzles/1.txt -o output/1.txt
```

Run benchmarks (time varies depending on difficulties):
```
> ./bin/benchmark
Benchmarking 3 puzzles with 100 repeats...
Puzzle 1:   17 [us]
Puzzle 2:   72 [us]
Puzzle 3:   56 [us]
```

<details>
<summary>
Run benchmark on larger datasets:
</summary>

```
> python benchmark.py /Users/monsoon/Code/repo/sudoku-dataset/hard_sudokus.txt
Read 10000 puzzles
Submitting tasks: 100%|████████████████████████████████████████████████████████| 10000/10000 [00:00<00:00, 22998.09it/s]
Collecting results: 100%|██████████████████████████████████████████████████████| 10000/10000 [00:00<00:00, 13878.64it/s]
------------------------------
Solved: 100.00%
Mean time: 71.9201 us
Median time: 39.0 us
Max time: 1225 us
Min time: 13 us
1st quartile time: 28.0 us
3rd quartile time: 94.25 us
------------------------------

> python benchmark.py /Users/monsoon/Code/repo/sudoku-dataset/all_17_clue_sudokus.txt
Read 49151 puzzles
Submitting tasks: 100%|████████████████████████████████████████████████████████| 49151/49151 [00:00<00:00, 49341.55it/s]
Collecting results: 100%|███████████████████████████████████████████████████████| 49151/49151 [00:05<00:00, 8346.97it/s]
------------------------------
Solved: 100.00%
Mean time: 446.5060120852068 us
Median time: 55.0 us
Max time: 308269 us
Min time: 16 us
1st quartile time: 43.0 us
3rd quartile time: 162.0 us
------------------------------
```

</details>

---

Environment variables:
- `SOLVER_USE_GUESS` enable guessing when solving the puzzle. Default is `1`.
- `SOLVER_HEURISTIC_GUESS` enable heuristic choosing of starting cell when guessing. Default is `1`.
- `SOLVER_DETERMINISTIC_GUESS` enable deterministic solving. Default is `0`.