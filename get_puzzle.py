"""
To get the board from: https://sugoku.onrender.com/
Refer to: https://github.com/bertoort/sugoku for API documentation
"""

import requests

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
    board = get_board("easy")
    dump_board(board, "puzzles/board.txt")
    print("Board dumped to board.txt")