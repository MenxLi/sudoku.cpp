
A simple Sudoku game solver. 

Build with:
```sh
make target
```

Then run with:
```sh
mkdir -p output

# for easy puzzles, use the simple method
./bin/sudoku -i puzzles/1.txt -o output/1.txt

# for harder puzzles or non-unique solutions, use the guessing method
USE_GUESS=1 ./bin/sudoku -i puzzles/2.txt -o output/2.txt
```

Run batch with:
```
> USE_GUESS=1 sh run.sh
Time elapsed: 42 [µs] Puzzle: puzzles/1.txt Solved! Output saved to: output/1.txt
Time elapsed: 225 [µs] Puzzle: puzzles/2.txt Solved! Output saved to: output/2.txt
Time elapsed: 440 [µs] Puzzle: puzzles/3.txt Solved! Output saved to: output/3.txt
```

For python bindings:
```sh
make python
python3 python/demo.py
```