# Algorithm Performance Comparison

Comparative analysis of **AVL Tree**, **Red-Black Tree**, and **Splay Tree** implemented in C.

## Project Structure
```
ADS_CP/
├── src/
│   ├── avl_tree.h / avl_tree.c
│   ├── rb_tree.h  / rb_tree.c
│   ├── splay_tree.h / splay_tree.c
│   └── benchmark.c
├── result/
│   └── results.csv
└── analysis/
    ├── dashboard.html
    ├── plot_results.py
    └── graphs/
```

## How to Run

### 1. Compile & Benchmark
```bash
cd src
gcc -O2 -o benchmark.exe benchmark.c avl_tree.c rb_tree.c splay_tree.c
.\benchmark.exe
```

### 2. Generate Graphs
```bash
cd ../analysis
python plot_results.py
```

### 3. View Dashboard
Open `analysis/dashboard.html` in any browser no server needed.
http://127.0.0.1:5500/analysis/dashboard.html

## Algorithms Compared
| Algorithm | Balance Strategy | Rotations |
|-----------|-----------------|-----------|
| AVL Tree | Strict (BF = -1,0,1) | More |
| Red-Black Tree | Relaxed (color rules) | Fewer |
| Splay Tree | None (amortized) | Most |

## Metrics Measured
- Execution Time
- Rotation Count
- Key Comparisons
- Tree Height

## Input Patterns
- Random
- Sorted
- Reverse Sorted

## Input Sizes
1,000 elements
5,000 elements
10,000 elements
50,000 elements
100,000 elements
