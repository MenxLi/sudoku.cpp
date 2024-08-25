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

@app.get("/generate")
def generate_puzzle(clues: int = -1):
    if clues == -1:
        return {
            "error": "Invalid query string",
            "details": "Please specify a number of clues in the query string, e.g. '?clues=32'"
            }
        
    try:
        gen = generate(clues, max_retries=128, parallel_exec=False)
    except:
        return {
            "error": "Failed to generate puzzle", 
            "details": "The server exceeded the maximum number of retries while randomly generating a puzzle. Please try again."
            }
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
