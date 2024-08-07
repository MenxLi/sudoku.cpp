"""
To get the board from: https://sugoku.onrender.com/
Refer to: https://github.com/bertoort/sugoku for API documentation
"""

import requests, concurrent.futures, pathlib

def get_board(difficulty: str = 'random') -> list[list[int]]:
    """
    Get the board from the API
    """
    url = f"https://sugoku.onrender.com/board?difficulty={difficulty}"
    response = requests.get(url)
    board = response.json()["board"]
    return board

def dump_board(board: list[list[int]], file_path: str) -> None:
    """
    Dump the board to the console
    """
    with open(file_path, "w") as file:
        for row in board:
            for cell in row:
                file.write(f"{cell} ")
            file.write("\n")

if __name__ == "__main__":
    puzzle_dir = pathlib.Path("puzzles_tmp")
    puzzle_dir.mkdir(exist_ok=True)
    
    def dump_puzzle_to_file(i):
        output_file = puzzle_dir / f"puzzle_{i}.txt"
        if output_file.exists():
            return

        board = get_board("random")
        dump_board(board, output_file)
        print(f"Board dumped to {output_file}")
    
    tasks = []
    with concurrent.futures.ThreadPoolExecutor() as executor:
        for i in range(100):
            tasks.append(executor.submit(dump_puzzle_to_file, i+1))
    for task in tasks:
        task.result()