
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
Run benchmark on larger dataset:
</summary>

```
# this is a very hard dataset...
> python benchmark.py /Users/monsoon/Code/repo/sudoku-dataset/all_17_clue_sudokus.txt
Read 49151 puzzles
Submitting tasks: 100%|████████████████████████████████████████████████████| 49151/49151 [00:01<00:00, 47697.24it/s]
Collecting results: 100%|███████████████████████████████████████████████████| 49151/49151 [00:08<00:00, 5551.66it/s]
------------------------------
Solved: 97.97%
Mean time: 676.5101549196329 us
Median time: 120.0 us
Max time: 302416 us
Min time: 18 us
1st quartile time: 53.0 us
3rd quartile time: 301.0 us
------------------------------
```

</details>

---

Environment variables:
- `SOLVER_USE_GUESS` enable guessing when solving the puzzle. Default is `1`.
- `SOLVER_HEURISTIC_GUESS` enable heuristic choosing of starting cell when guessing. Default is `1`.
- `SOLVER_DETERMINISTIC_GUESS` enable deterministic solving. Default is `0`.