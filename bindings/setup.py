import pathlib
from setuptools import setup
from pybind11.setup_helpers import Pybind11Extension, ParallelCompile
ParallelCompile("NPY_NUM_BUILD_JOBS", default=4).install()

__root_dir__ = pathlib.Path(__file__).resolve().parent.parent
src_dir = __root_dir__ / "src"
include_dir = __root_dir__ / "include"
exclude_patterns = ["main.cpp", "benchmark.cpp", "*_test.cpp", "solver_v1*"]
cpp_files = [
    str(f) for f in src_dir.glob("*.cpp") 
    if not any(f.match(p) for p in exclude_patterns)
]

setup(
    name='sudoku-cpp',
    version='0.0.1',
    author='Li, Mengxun', 
    description='A fast sudoku solver written in C++',
    packages=['sudoku_cpp'],
    requires=["pybind11"],
    ext_modules=[
        Pybind11Extension(
            "sudoku_cpp.sudoku",
            sources=cpp_files,
            cxx_std=17,
            include_dirs=[include_dir],
            define_macros=[("STRICT", "1"), ("GRID_SIZE", "3")],
            extra_compile_args=["-O3", "-funroll-loops", "-finline-functions"],
        ),
    ],
)