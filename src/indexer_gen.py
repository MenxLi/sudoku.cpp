"""
Generate ./indexer.cpp from the given N
"""

import os, math
from io import TextIOWrapper
from typing import TypeVar

def as_cpp_array(
    dtype: str, 
    name: str,
    arr: list[int, list], 
):
    """ Turn a multi-dimensional array into a C++ array """
    def _get_dims(arr: list):
        dims = []
        while True:
            dims.append(len(arr))
            if isinstance(arr[0], list):
                arr = arr[0]
            else:
                break
        return dims

    def _pure_array_to_str(arr: list):
        if isinstance(arr[0], int):
            return "{" + ", ".join(str(x) for x in arr) + "}"
        elif isinstance(arr[0], list):
            return "{" + ", ".join(_pure_array_to_str(sub_arr) for sub_arr in arr) + "}"
        else:
            raise ValueError("Invalid array")
    
    array_str = _pure_array_to_str(arr)
    dim_str = ''.join(f'[{dim}]' for dim in _get_dims(arr))
    return f"{dtype} Indexer::{name} {dim_str} = {array_str}; "

T = TypeVar('T')
def n_combinations(k: int, arr: list[T]) -> list[list[T]]:
    """ Compute the number of combinations """
    n = len(arr)
    assert n >= k
    assert k >= 1

    if n == k: return [arr]
    if k == 1: return [[x] for x in arr]
    
    result = []
    first = arr[0]

    for sub_comb in n_combinations(k-1, arr[1:]):
        result.append([first] + sub_comb)
    
    for sub_comb in n_combinations(k, arr[1:]):
        result.append(sub_comb)
    
    return result

def test_n_combinations():
    arr = [1, 2, 3, 4]
    for k in range(1, 5):
        print(n_combinations(k, arr))
    

def generate_indexer_impl(N: int):

    src_dir = os.path.dirname(os.path.abspath(__file__))
    dst_file = os.path.join(src_dir, "indexer.cpp")

    NV = N  # number of values
    NG = int(math.sqrt(N))
    assert NG * NG == N

    def _init_lookup(fp: TextIOWrapper):
        grid_lookup = []            # N, N, 2
        offset_coord_lookup = []    # NxN, 2
        coord_offset_lookup = []    # N, N

        for i in range(N):
            coord_offset_lookup.append([])
            for j in range(N):
                offset = i*N + j
                offset_coord_lookup.append([i, j])
                coord_offset_lookup[i].append(offset)
        
        for i in range(N):
            grid_lookup.append([])
            for j in range(N):
                grid_i = i // NG
                grid_j = j // NG
                grid_lookup[i].append([grid_i, grid_j])
        
        fp.write(as_cpp_array("const unsigned int", "grid_lookup", grid_lookup))
        fp.write(as_cpp_array("const unsigned int", "offset_coord_lookup", offset_coord_lookup))
        fp.write(as_cpp_array("const unsigned int", "coord_offset_lookup", coord_offset_lookup))

    def _init_index(fp: TextIOWrapper):
        row_index = []          # N, N
        col_index = []          # N, N
        grid_index = []         # NG, NG, N
        grid_coord_index = []   # N, N, N
        neighbor_index = []     # N, N, N_NEIGHBORS

        for i in range(N):
            row_index.append([])
            for j in range(N):
                offset = i*N + j
                row_index[i].append(offset)
        for j in range(N):
            col_index.append([])
            for i in range(N):
                offset = i*N + j
                col_index[j].append(offset)
        
        def _get_offsets_of_grid(g_i, g_j):
            offsets = []
            for i in range(g_i*NG, (g_i+1)*NG):
                for j in range(g_j*NG, (g_j+1)*NG):
                    offsets.append(i*N + j)
            return offsets
        
        for i in range(NG):
            grid_index.append([])
            for j in range(NG):
                grid_index[i].append(_get_offsets_of_grid(i, j))
        
        for i in range(N):
            grid_coord_index.append([])
            for j in range(N):
                grid_coord_index[i].append(_get_offsets_of_grid(i // NG, j // NG))
        
        def _get_neighbors(i, j):
            neighbors = []
            n_neighbors = 2 * (N - NG) + NG * NG - 1

            # first count the row neighbors
            for k in range(N):
                if k != j:
                    neighbors.append(i*N + k)
            assert len(neighbors) == N - 1
            
            # then count the column neighbors
            for k in range(N):
                if k != i:
                    neighbors.append(k*N + j)
            assert len(neighbors) == 2 * (N - 1)

            # finally count the grid neighbors
            grid_i = i // NG
            grid_j = j // NG
            for k in range(grid_i*NG, (grid_i+1)*NG):
                for l in range(grid_j*NG, (grid_j+1)*NG):
                    if k != i and l != j:
                        neighbors.append(k*N + l)
            assert len(neighbors) == n_neighbors

            return neighbors
        
        for i in range(N):
            neighbor_index.append([])
            for j in range(N):
                neighbor_index[i].append(_get_neighbors(i, j))
        
        fp.write(as_cpp_array("const unsigned int", "row_index", row_index))
        fp.write(as_cpp_array("const unsigned int", "col_index", col_index))
        fp.write(as_cpp_array("const unsigned int", "grid_index", grid_index))
        fp.write(as_cpp_array("const unsigned int", "grid_coord_index", grid_coord_index))
        fp.write(as_cpp_array("const unsigned int", "neighbor_index", neighbor_index))

    def _init_combinations(fp: TextIOWrapper):
        fp.write(as_cpp_array( "const unsigned int", "subunit_combinations_2", n_combinations(2, list(range(N)))))
        fp.write(as_cpp_array( "const unsigned int", "subvalue_combinations_2", n_combinations(2, list(range(NV)))))

    with open(dst_file, "w") as f:
        f.write("// This is a generated file, do not edit it directly\n")
        f.write("// N: {}\n".format(N))
        f.write('#include "indexer.h"\n')
        _init_lookup(f)
        _init_index(f)
        _init_combinations(f)

if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser()
    parser.add_argument("N", type=int)

    args = parser.parse_args()
    generate_indexer_impl(args.N)

    print("\033[1;32mindexer.cpp generated successfully (N={})\033[0m".format(args.N))
