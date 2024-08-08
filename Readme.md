
A Sudoku game solver. 

Build with:
```sh
make target
```

Then run with:
```sh
mkdir -p output

# for easy puzzles, use the simple rule-based method
./bin/sudoku -i puzzles/1.txt -o output/1.txt

# for harder puzzles or non-unique solutions, use the guessing method
GUESS=1 ./bin/sudoku -i puzzles/2.txt -o output/2.txt
```

Run benchmarks with:
```
> GUESS=1 ./bin/benchmark
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