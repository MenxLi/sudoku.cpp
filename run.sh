#!/bin/bash

n_success=0
n_total=0

for i in {1..100}; do
    if [ ! -f "puzzles/${i}.txt" ]; then
        continue
    fi

    ./bin/sudoku -i "puzzles/${i}.txt" -o "output/${i}.txt"
    if [ $? -eq 0 ]; then
        ((n_success++))
    fi
    ((n_total++))
done

echo "Success rate: ${n_success}/${n_total}"
