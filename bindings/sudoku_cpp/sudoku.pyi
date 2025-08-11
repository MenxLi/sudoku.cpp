from typing import TypedDict

class Board:
    @staticmethod
    def from_list1d(data: list[int]) -> 'Board': ...
    @staticmethod
    def from_list2d(data: list[list[int]]) -> 'Board': ...
    @staticmethod
    def from_str(data: str, sp: str = " ", nl: str = "\n") -> 'Board': ...
    def to_list1d(self) -> list[int]: ...
    def to_list2d(self) -> list[list[int]]: ...
    def get(self, row: int, col: int) -> int: ...
    def set(self, row: int, col: int, value: int) -> None: ...

class SolveResult(TypedDict):
    board: Board
    solved: bool
    iterations: int
    iteration_limit: int
    n_guesses: int
    time_us: int
def solve(b: Board)->SolveResult:...

class GenerateResult(TypedDict):
    board: Board
    time_us: int
def generate(
    n_clues: int, 
    max_retries: int = 1024,
    verbose: bool = True
    )->GenerateResult:...

class BuildConfig(TypedDict):
    BOARD_SIZE: int
    GRID_SIZE: int
    CELL_COUNT: int
def build_config()->BuildConfig:...