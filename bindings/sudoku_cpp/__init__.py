from . import sudoku

def fmt_board(board: list[list[int]]) -> str:
    board_size = sudoku.build_config()['BOARD_SIZE']
    grid_size = sudoku.build_config()['GRID_SIZE']
    assert len(board) == board_size
    assert all(len(row) == board_size for row in board)

    output = []
    W = board_size * 2 + 2 + 2 + grid_size
    for i in range(board_size):
        if i % grid_size == 0:
            output.append("-" * W)
        line = []
        for j in range(board_size):
            if j % grid_size == 0:
                line.append("|")
            line.append(str(board[i][j]))
        line.append("|")
        output.append(" ".join(line))
    output.append("-" * W)

    return "\n".join(output)