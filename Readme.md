
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
Puzzle 1:    6 [us]
Puzzle 2:   25 [us]
Puzzle 3:  377 [us]
```

---

Environment variables:
- `SOLVER_USE_GUESS` enable guessing when solving the puzzle. Default is `1`.
- `SOLVER_HEURISTIC_GUESS` enable heuristic choosing of starting cell when guessing. Default is `1`.
- `SOLVER_DETERMINISTIC_GUESS` enable deterministic solving. Default is `0`.