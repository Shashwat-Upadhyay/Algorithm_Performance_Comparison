# Algorithm Performance Comparison — ADS College Project

A benchmarking and visualization suite that compares the performance of **5 tree data structures** across insert, search, delete, and range-query operations, over multiple input sizes and patterns.

---

## Table of Contents

- [Project Structure](#project-structure)
- [Trees Implemented](#trees-implemented)
- [Metrics Measured](#metrics-measured)
- [Part 1 — BST Family Benchmark (C)](#part-1--bst-family-benchmark-c)
- [Part 2 — B-Tree Family Benchmark (C)](#part-2--b-tree-family-benchmark-c)
- [Part 3 — Dashboard (React)](#part-3--dashboard-react)
- [How to Run Everything](#how-to-run-everything)
- [CSV Format Reference](#csv-format-reference)
- [Known Issues](#known-issues)

---

## Project Structure

```
ADS_CP/
├── bst/
│   ├── src/
│   │   ├── avl_tree.c / avl_tree.h      # AVL Tree implementation
│   │   ├── rb_tree.c  / rb_tree.h       # Red-Black Tree implementation
│   │   ├── splay_tree.c / splay_tree.h  # Splay Tree implementation
│   │   └── benchmark.c                  # BST benchmark runner
│   ├── result/
│   │   └── results.csv                  # Generated BST results
│   └── analysis/
│       └── graphs/                      # Generated graph images
│
├── btrees/
│   ├── src/
│   │   ├── btree.c   / btree.h          # B-Tree (order 5) implementation
│   │   ├── bplustree.c / bplustree.h    # B+ Tree (order 5) implementation
│   │   └── benchmark_btrees.c           # B-Tree benchmark runner
│   ├── result/
│   │   └── btree_results.csv            # Generated B-Tree results
│   └── analysis/
│       └── graphs/                      # Generated graph images
│
└── analysis/
    └── ads-dashboard/                   # React + Vite dashboard
        ├── src/
        │   ├── dashboard.jsx            # Main dashboard component
        │   └── App.jsx
        └── package.json
```

---

## Trees Implemented

### BST Family

| Tree | Order | Key Property |
|------|-------|--------------|
| **AVL Tree** | — | Strictly height-balanced. Balance factor of every node kept within {-1, 0, 1} via rotations after every insert/delete. |
| **Red-Black Tree** | — | Loosely height-balanced using node colouring (RED/BLACK) rules. Fewer rotations than AVL on average, faster inserts. |
| **Splay Tree** | — | Self-adjusting BST. Recently accessed nodes are moved to the root via splaying. No explicit balance property — amortized O(log n). |

### B-Tree Family

| Tree | Order | Key Property |
|------|-------|--------------|
| **B-Tree** | 5 | Generalised balanced search tree. Each node holds up to 4 keys and 5 children. Data stored in all nodes (internal + leaf). |
| **B+ Tree** | 5 | Variant of B-Tree where all actual data lives only in leaf nodes. Internal nodes store only separator keys. Leaves are linked for efficient range queries. |

---

## Metrics Measured

### BST Metrics (per operation, per size, per pattern)

| Column | Meaning |
|--------|---------|
| `time` | Wall-clock time in seconds for the full operation over n keys |
| `rotations` | Total tree rotations performed (insert/delete only; 0 for search) |
| `comparisons` | Total key comparisons made |
| `height` | Tree height after the operation completes |

### B-Tree Metrics (per operation, per size, per pattern)

| Column | Meaning |
|--------|---------|
| `time` | Wall-clock time in seconds |
| `splits` | Number of node splits (insert only) |
| `merges` | Number of node merges (delete only) |
| `comparisons` | Total key comparisons |
| `node_accesses` | Total nodes visited/accessed |
| `height` | Tree height after the operation |
| `fill_factor` | Average node utilisation as a percentage (higher = better space usage) |

---

## Part 1 — BST Family Benchmark (C)

### What it does

`benchmark.c` runs insert → search → delete for all three BST types (AVL, Red-Black, Splay) across:

- **5 input sizes:** 1,000 · 5,000 · 10,000 · 50,000 · 100,000
- **3 input patterns:** random · sorted · reverse-sorted

Each combination is timed and the relevant counters (rotations, comparisons, height) are recorded to `results.csv`.

### How to compile

Navigate to the BST source folder:

```bash
cd bst/src
```

**Linux / macOS:**
```bash
gcc -O2 -o benchmark benchmark.c avl_tree.c rb_tree.c splay_tree.c -lm
```

**Windows (MinGW / GCC):**
```bash
gcc -O2 -o benchmark.exe benchmark.c avl_tree.c rb_tree.c splay_tree.c -lm
```

### How to run

```bash
# Linux / macOS
./benchmark

# Windows
./benchmark.exe
```

Output is saved to `bst/result/results.csv`.

You will see progress printed to the terminal as each (n, pattern) combination completes:

```
Running n=1000     pattern=random ...
Running n=1000     pattern=sorted ...
...
Benchmark finished.
Results saved to results/results.csv
```

---

## Part 2 — B-Tree Family Benchmark (C)

### What it does

`benchmark_btrees.c` runs insert → search → delete → range_query for B-Tree and B+ Tree across the same 5 sizes and 3 patterns. Additionally, after insertion, a **range query** is run on B+ Tree over a 10% window around the median value to demonstrate the leaf-linked range scan advantage.

### How to compile

Navigate to the B-Tree source folder:

```bash
cd btrees/src
```

**Linux / macOS:**
```bash
gcc -O2 -o benchmark_btrees benchmark_btrees.c btree.c bplustree.c -lm
```

**Windows (MinGW / GCC):**
```bash
gcc -O2 -o benchmark_btrees.exe benchmark_btrees.c btree.c bplustree.c -lm
```

### How to run

```bash
# Linux / macOS
./benchmark_btrees

# Windows
./benchmark_btrees.exe
```

Output is saved to `btrees/result/btree_results.csv`.

Terminal output looks like:

```
Running n=1000     pattern=random ...
  B+ range [450,550]: 101 keys found  (leaf-accesses=3)
Running n=1000     pattern=sorted ...
...
Benchmark finished.
Results saved to btree/result/btree_results.csv
```

---

## Part 3 — Dashboard (React)

### What it does

An interactive browser-based dashboard built with **React 19 + Vite + Recharts**. You upload either CSV (BST or B-Tree) and the dashboard auto-detects the format and renders:

**BST Dashboard:**
- Line chart — any metric (rotations / comparisons / height / time) across all sizes, for any operation
- Height vs log₂(n) theoretical bound
- Rotations bar chart (insert vs delete at n=100K)
- Summary stat cards (★ = best / lowest at n=100K)
- Full scrollable data table

**B-Tree Dashboard:**
- Line chart — any metric across all sizes, for any operation (including B+ Tree delete once implemented)
- Height vs log₅(n) theoretical bound (order 5)
- Fill factor % across sizes
- Splits bar chart (insert)
- B+ Tree range query — leaf accesses and comparisons
- Summary stat cards
- Full scrollable data table

**Controls:**
- Switch between BST Family / B-Tree Family views
- Toggle input pattern: Random / Sorted / Reverse
- Upload CSV button (auto-detects format from CSV header)

### Prerequisites

Node.js 18+ must be installed. Check with:

```bash
node --version
npm --version
```

### How to run

```bash
cd analysis/ads-dashboard
npm install
npm run dev
```

Then open your browser at: **http://localhost:5173**

### How to build for production

```bash
npm run build
```

Output goes to `analysis/ads-dashboard/dist/`. Serve with any static file server.

### How to use the dashboard

1. Run either benchmark (Part 1 or Part 2) to generate the CSV
2. Open the dashboard in browser
3. Click **Upload CSV**
4. Select `bst/result/results.csv` → switches to BST view automatically
5. Select `btrees/result/btree_results.csv` → switches to B-Tree view automatically
6. Use the Pattern tabs (Random / Sorted / Reverse) and Operation/Metric tabs to explore

---

## How to Run Everything (Quick Reference)

```bash
# Step 1 — BST Benchmark
cd bst/src
gcc -O2 -o benchmark benchmark.c avl_tree.c rb_tree.c splay_tree.c -lm
./benchmark

# Step 2 — B-Tree Benchmark
cd ../../btrees/src
gcc -O2 -o benchmark_btrees benchmark_btrees.c btree.c bplustree.c -lm
./benchmark_btrees

# Step 3 — Dashboard
cd ../../analysis/ads-dashboard
npm install
npm run dev
# Open http://localhost:5173 and upload the CSVs
```

---

## CSV Format Reference

### BST — `results.csv`

```
tree,operation,pattern,n,time,rotations,comparisons,height
AVL,insert,random,1000,0.001200,312,8423,14
AVL,search,random,1000,0.000800,0,9100,14
AVL,delete,random,1000,0.001100,204,7800,14
RB,insert,random,1000,...
Splay,insert,random,1000,...
```

- `tree`: `AVL` | `RB` | `Splay`
- `operation`: `insert` | `search` | `delete`
- `pattern`: `random` | `sorted` | `reverse`
- `n`: one of 1000, 5000, 10000, 50000, 100000

### B-Tree — `btree_results.csv`

```
tree,operation,pattern,n,time,splits,merges,comparisons,node_accesses,height,fill_factor
BTree,insert,random,1000,0.001000,424,0,9344,5290,6,58.14
BTree,search,random,1000,...
BTree,delete,random,1000,...
BPlusTree,insert,random,1000,...
BPlusTree,search,random,1000,...
BPlusTree,range_query,random,1000,...
BPlusTree,delete,random,1000,...
```

- `tree`: `BTree` | `BPlusTree`
- `operation`: `insert` | `search` | `delete` | `range_query`
- `splits`: non-zero only during insert
- `merges`: non-zero only during delete
- `fill_factor`: percentage (e.g. `64.88` means 64.88%)

---

## Known Issues

### Windows `mkdir -p` error (Low Impact)

**Symptom:** Running `benchmark.exe` or `benchmark_btrees.exe` prints:
```
The syntax of the command is incorrect.
```

**Cause:** The C code calls `system("mkdir -p ../result")`. Windows CMD/PowerShell does not support the `-p` flag for `mkdir`.

**Impact:** None. The program falls back through multiple `fopen` paths and successfully creates the CSV file regardless.

**Fix (optional):** In `benchmark.c` and `benchmark_btrees.c`, remove the `system(...)` line entirely — the fallback `fopen` chain handles directory creation on its own.

---

## Dependencies Summary

| Component | Dependency | Version |
|-----------|-----------|---------|
| C Benchmarks | GCC | Any modern (supports C99) |
| Dashboard | Node.js | 18+ |
| Dashboard | React | 19 |
| Dashboard | Recharts | 3.x |
| Dashboard | Vite | 8.x |

---

## GitHub

[github.com/Shashwat-Upadhyay/Algorithm_Performance_Comparison](https://github.com/Shashwat-Upadhyay/Algorithm_Performance_Comparison)