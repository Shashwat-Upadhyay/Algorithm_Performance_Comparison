
# Algorithm Performance Comparison

Comparative analysis of self-balancing BSTs and disk-oriented B-Tree variants, implemented in C.

## Why two separate benchmarks?

BSTs (AVL, Red-Black, Splay) and B-Trees are optimised for fundamentally different things.
Mixing them in a single benchmark produces misleading comparisons:

| Concern            | BST Family              | B-Tree Family                  |
|--------------------|-------------------------|--------------------------------|
| Structure          | Binary, one key/node    | Multi-way, many keys/node      |
| Primary metric     | Rotations, height       | Splits, node accesses          |
| Range queries      | O(k + log n) with scan  | O(log n + k) via leaf list     |
| Fill factor        | N/A                     | ≥ 50% guaranteed               |

## Project Structure

```
ADS_CP/
├── bst/                         ← BST family (AVL, Red-Black, Splay)
│   ├── src/
│   │   ├── avl_tree.h / avl_tree.c
│   │   ├── rb_tree.h  / rb_tree.c
│   │   ├── splay_tree.h / splay_tree.c
│   │   └── benchmark.c
│   ├── result/
│   │   └── results.csv
│   └── analysis/
│       ├── plot_results.py
│       └── graphs/
│
├── btree/                       ← B-Tree family (B-Tree, B+ Tree)
│   ├── src/
│   │   ├── btree.h / btree.c       (order 5)
│   │   ├── bplustree.h / bplustree.c (order 4)
│   │   └── benchmark_btree.c
│   ├── result/
│   │   └── btree_results.csv
│   └── analysis/
│       ├── plot_btree.py
│       └── graphs/
│
└── analysis/
    └── dashboard.html           ← unified two-tab dashboard (no server needed)
```

## How to Run

### BST Benchmark

```bash
cd bst/src
gcc -O2 -o benchmark benchmark.c avl_tree.c rb_tree.c splay_tree.c
./benchmark                      # writes bst/result/results.csv
cd ../analysis
python3 plot_results.py          # writes graphs/*.png
```

### B-Tree Benchmark

```bash
cd btree/src
gcc -O2 -o benchmark_btree benchmark_btree.c btree.c bplustree.c
./benchmark_btree                # writes btree/result/btree_results.csv
cd ../analysis
python3 plot_btree.py            # writes graphs/*.png
```

### Dashboard

Open `analysis/dashboard.html` in any browser — no server needed.
Load `bst/result/results.csv` in the **BST Family** tab,
load `btree/result/btree_results.csv` in the **B-Tree Family** tab.
The dashboard auto-detects which CSV you upload by its header row.

## Algorithms Compared

### BST Family

| Algorithm      | Balance Strategy        | Rotation style     |
|----------------|-------------------------|--------------------|
| AVL Tree       | Strict (BF = −1, 0, 1)  | More, predictable  |
| Red-Black Tree | Relaxed (colour rules)  | Fewer              |
| Splay Tree     | None (amortised)        | Most, access-driven|

### B-Tree Family

| Algorithm  | Order | Data in internal nodes | Leaf linking |
|------------|-------|------------------------|--------------|
| B-Tree     | 5     | Yes                    | No           |
| B+ Tree    | 4     | No (routing keys only) | Yes (→ range)|

## Metrics Measured

### BST
- Execution time
- Rotation count (insert, delete)
- Key comparisons
- Tree height

### B-Tree
- Execution time
- Node splits (insert)
- Node merges (delete)
- Key comparisons
- Node accesses (simulated I/O)
- Tree height
- Fill factor % (space utilisation)
- Range query leaf-accesses (B+ tree only)

## Input Patterns
- **Random** — shuffled integers via `rand()`
- **Sorted** — ascending sequence 0, 1, 2, …
- **Reverse sorted** — descending sequence n, n−1, …

## Input Sizes
1,000 · 5,000 · 10,000 · 50,000 · 100,000 elements

http://localhost:5173/

<img width="1398" height="888" alt="image" src="https://github.com/user-attachments/assets/da4f2aa2-16c3-4a33-80d0-4dd62b99d5d1" />

