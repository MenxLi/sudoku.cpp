#!/bin/bash

n_success=0

for i in {1..100}; do
    ./bin/sudoku -i "puzzles_tmp/${i}.txt" -o "output/${i}.txt"
    if [ $? -eq 0 ]; then
        ((n_success++))
    fi
done

echo "Success rate: ${n_success}/100"
