import fastapi
from fastapi.middleware.cors import CORSMiddleware
import json
from sudoku_cpp import solve, generate, build_config

BOARD_SIZE = build_config()['BOARD_SIZE']
app = fastapi.FastAPI()

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

__difficulty_n_map = {
    "easy": 32,
    "medium": 28,
    "hard": 25,
    "extreme": 21,
}

@app.get("/generate")
def generate_puzzle(level: str):
    if level not in __difficulty_n_map:
        return {"error": f"Invalid difficulty level: {level}"}

    n_clues = __difficulty_n_map.get(level, 32)
    gen = generate(n_clues, max_retries=128, parallel_exec=False)
    return gen['data']

def solve_puzzle(puzzle: list[list[int]]):
    if len(puzzle) != BOARD_SIZE:
        return {"error": f"Invalid board size: {len(puzzle)}"}
    if any(len(row) != BOARD_SIZE for row in puzzle):
        return {"error": f"Invalid board size: {len(puzzle)}"}
    return solve(puzzle)['data']

@app.post("/solve")
def solve_puzzle_post_api(puzzle: list[list[int]]):
    return solve_puzzle(puzzle)
@app.get("/solve")
def solve_puzzle_get_api(puzzle: str):
    return solve_puzzle(json.loads(puzzle))

if __name__ == "__main__":
    import uvicorn, argparse
    parser = argparse.ArgumentParser()
    parser.add_argument("--host", type=str, default="127.0.0.1", help="Host address")
    parser.add_argument("--port", type=int, default=8888, help="Port number")
    parser.add_argument("--workers", type=int, default=1, help="Number of workers")
    args = parser.parse_args()

    uvicorn.run("__main__:app", host=args.host, port=args.port, workers=args.workers)
