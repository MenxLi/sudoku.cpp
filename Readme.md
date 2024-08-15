
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
Puzzle 1:    5 [us]
Puzzle 2:   12 [us]
Puzzle 3:   74 [us]
```

<details>
<summary>
Run benchmark on larger datasets:
</summary>

```
> python -m scripts.benchmark /Users/monsoon/Code/repo/sudoku-dataset/hard_sudokus.txt
Read 10000 puzzles
Submitting tasks: 100%|████████████████████████████████████████████████████████| 10000/10000 [00:00<00:00, 21433.89it/s]
Collecting results: 100%|██████████████████████████████████████████████████████| 10000/10000 [00:00<00:00, 13791.81it/s]
------------------------------
Solved: 100.00%
Mean time: 40.0826 us
Median time: 35.0 us
Max time: 290 us
Min time: 9 us
1st quartile time: 24.0 us
3rd quartile time: 50.0 us
------------------------------

> python -m scripts.benchmark /Users/monsoon/Code/repo/sudoku-dataset/all_17_clue_sudokus.txt
Read 49151 puzzles
Submitting tasks: 100%|████████████████████████████████████████████████████████| 49151/49151 [00:01<00:00, 45788.44it/s]
Collecting results: 100%|██████████████████████████████████████████████████████| 49151/49151 [00:03<00:00, 15621.89it/s]
------------------------------
Solved: 100.00%
Mean time: 108.90942198531057 us
Median time: 53.0 us
Max time: 11980 us
Min time: 15 us
1st quartile time: 41.0 us
3rd quartile time: 78.0 us
------------------------------
```

</details>

---

Environment variables:
- `SOLVER_USE_GUESS` enable guessing when solving the puzzle. Default is `1`.
- `SOLVER_HEURISTIC_GUESS` enable heuristic choosing of starting cell when guessing. Default is `1`.
- `SOLVER_DETERMINISTIC_GUESS` enable deterministic solving. Default is `0`.
- `SOLVER_USE_DOUBLE` enable naked/hidden double solving. Default is `0`.