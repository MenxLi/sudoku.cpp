
A Sudoku game solver. 

Build with:
```sh
make target
```

Then run with:
```sh
mkdir -p output

# solve a puzzle by providing the input and output file
./bin/sudoku -i puzzles/1.txt -o output/1.txt
```

Run benchmarks with:
```
> ./bin/benchmark
Benchmarking 3 puzzles with 10000 repeats...
Puzzle 1:   31 [us]
Puzzle 2:  115 [us]
Puzzle 3:  196 [us]
```

For python bindings:
```sh
make python
python3 python/demo.py
```