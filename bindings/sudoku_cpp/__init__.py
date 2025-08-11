from .sudoku import *

def fmt_board(b: Board) -> str:
    board = b.to_list2d()
    board_size = sudoku.build_config()['BOARD_SIZE']
    grid_size = sudoku.build_config()['GRID_SIZE']
    assert len(board) == board_size
    assert all(len(row) == board_size for row in board)
    max_digit_len = max(len(str(d)) for row in board for d in row)

    def _fmt_digit(d: int) -> str:
        return str(d).rjust(max_digit_len)

    output = []
    W = (board_size * max_digit_len) + board_size + (grid_size-1)*2 + 1
    output.append('┌' + "─" * W + '┐')
    for i in range(board_size):
        if i % grid_size == 0 and i != 0:
            line = '├' + "─" * W + '┤'
            output.append(line)
        line = []
        for j in range(board_size):
            if j % grid_size == 0:
                line.append("│")
            line.append(_fmt_digit(board[i][j]))
        line.append("│")
        output.append(" ".join(line))

    line = '└' + "─" * W + '┘'
    output.append(line)

    return "\n".join(output)