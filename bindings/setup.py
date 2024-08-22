import pathlib, subprocess
from sys import platform
from setuptools import setup
from pybind11.setup_helpers import Pybind11Extension, ParallelCompile
ParallelCompile("NPY_NUM_BUILD_JOBS", default=4).install()

BOARD_SIZE = 9
__root_dir__ = pathlib.Path(__file__).resolve().parent.parent
src_dir = __root_dir__ / "src"

# generate indexer implementation
subprocess.check_call([ "python3", str(src_dir / "indexer_gen.py"), str(BOARD_SIZE) ])

include_dir = __root_dir__ / "include"
exclude_patterns = ["main.cpp", "benchmark.cpp", "*_test.cpp", "solver_v1*"]
cpp_files = [
    str(f) for f in src_dir.glob("*.cpp") 
    if not any(f.match(p) for p in exclude_patterns)
]

compile_args=["-O3", "-funroll-loops", "-finline-functions"]
if platform == 'linux' or platform == 'linux2':
    compile_args += ["-pthread"]

setup(
    name='sudoku-cpp',
    version='0.0.1',
    author='Li, Mengxun', 
    description='A fast sudoku solver written in C++',
    packages=['sudoku_cpp'],
    install_requires=["wheel", "pybind11"],
    package_data={'sudoku_cpp': ['sudoku.pyi']},
    ext_modules=[
        Pybind11Extension(
            "sudoku_cpp.sudoku",
            sources=cpp_files,
            cxx_std=17,
            include_dirs=[include_dir],
            define_macros=[
                ("PYBIND11_BUILD", "1"), 
                ("SIZE", BOARD_SIZE),
                ("STRICT", "1"), 
            ],
            extra_compile_args=compile_args,
        ),
    ],
)
