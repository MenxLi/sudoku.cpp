
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