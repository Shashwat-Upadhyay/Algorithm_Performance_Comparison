import csv
import os
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np
from collections import defaultdict

CSV_PATH   = "../result/results.csv"
OUT_DIR    = "graphs"
os.makedirs(OUT_DIR, exist_ok=True)

SIZES      = [1000, 5000, 10000, 50000, 100000]
SIZE_LABELS= ["1K", "5K", "10K", "50K", "100K"]
PATTERNS   = ["random", "sorted", "reverse"]
OPS        = ["insert", "search", "delete"]
TREES      = ["AVL", "RB", "Splay"]

COLORS  = {"AVL": "#2196F3", "RB": "#F44336", "Splay": "#9C27B0"}
MARKERS = {"AVL": "o",       "RB": "s",       "Splay": "^"}
LABELS  = {"AVL": "AVL Tree","RB": "Red-Black","Splay": "Splay Tree"}
LW, MS  = 2.2, 7

plt.rcParams.update({
    "figure.facecolor": "#fafafa",
    "axes.facecolor":   "#ffffff",
    "axes.grid":        True,
    "grid.linestyle":   "--",
    "grid.alpha":       0.4,
    "font.size":        10,
})

# ── Load CSV ──────────────────────────────────────────────────────────────────
# data[pattern][operation][tree][size] = {time, rotations, comparisons, height}
data = defaultdict(lambda: defaultdict(lambda: defaultdict(dict)))

with open(CSV_PATH, newline='', encoding='utf-8-sig') as f:
    for row in csv.DictReader(f):
        # strip whitespace from all values (fixes Windows line endings)
        row = {k.strip(): v.strip() if v else "0" for k, v in row.items()}
        try:
            data[row["pattern"]][row["operation"]][row["tree"]][int(row["n"])] = {
                "time":        float(row["time"]        or 0),
                "rotations":   int(row["rotations"]     or 0),
                "comparisons": int(row["comparisons"]   or 0),
                "height":      int(row["height"]        or 0),
            }
        except (ValueError, KeyError):
            continue   # skip malformed rows

def get(pattern, op, tree, metric):
    d = data[pattern][op][tree]
    return [d.get(s, {}).get(metric, 0) for s in SIZES]

def plot_tree(ax, pattern, op, metric):
    for tree in TREES:
        ax.plot(SIZE_LABELS, get(pattern, op, tree, metric),
                color=COLORS[tree], marker=MARKERS[tree],
                linewidth=LW, markersize=MS, label=LABELS[tree])

# ── GRAPH 1 : Rotations — Insert & Delete ────────────────────────────────────
fig, axes = plt.subplots(2, 3, figsize=(16, 9))
fig.suptitle("AVL vs Red-Black vs Splay — Rotations", fontsize=15, fontweight="bold")

for col, pattern in enumerate(PATTERNS):
    for row, op in enumerate(["insert", "delete"]):
        ax = axes[row][col]
        plot_tree(ax, pattern, op, "rotations")
        ax.set_title(f"{op.capitalize()} — {pattern}", fontweight="bold")
        ax.set_xlabel("Input Size")
        ax.set_ylabel("Rotations")
        ax.legend(fontsize=8)

plt.tight_layout()
plt.savefig(f"{OUT_DIR}/graph_rotations.png", dpi=150, bbox_inches="tight")
plt.close()
print("Saved: graph_rotations.png")

# ── GRAPH 2 : Comparisons — All operations ───────────────────────────────────
fig, axes = plt.subplots(3, 3, figsize=(16, 13))
fig.suptitle("AVL vs Red-Black vs Splay — Comparisons", fontsize=15, fontweight="bold")

for row, pattern in enumerate(PATTERNS):
    for col, op in enumerate(OPS):
        ax = axes[row][col]
        plot_tree(ax, pattern, op, "comparisons")
        ax.set_title(f"{op.capitalize()} — {pattern}", fontweight="bold")
        ax.set_xlabel("Input Size")
        ax.set_ylabel("Comparisons")
        ax.legend(fontsize=8)

plt.tight_layout()
plt.savefig(f"{OUT_DIR}/graph_comparisons.png", dpi=150, bbox_inches="tight")
plt.close()
print("Saved: graph_comparisons.png")

# ── GRAPH 3 : Tree Height after Insert ───────────────────────────────────────
fig, axes = plt.subplots(1, 3, figsize=(16, 5))
fig.suptitle("AVL vs Red-Black vs Splay — Tree Height After Insert", fontsize=15, fontweight="bold")

for col, pattern in enumerate(PATTERNS):
    ax = axes[col]
    plot_tree(ax, pattern, "insert", "height")
    log_vals = [round(np.log2(s)) for s in SIZES]
    ax.plot(SIZE_LABELS, log_vals, "k--", linewidth=1.2, alpha=0.5, label="log₂(n)")
    ax.set_title(f"Height — {pattern}", fontweight="bold")
    ax.set_xlabel("Input Size")
    ax.set_ylabel("Height")
    ax.legend(fontsize=8)

plt.tight_layout()
plt.savefig(f"{OUT_DIR}/graph_height.png", dpi=150, bbox_inches="tight")
plt.close()
print("Saved: graph_height.png")

# ── GRAPH 4 : Execution Time — All operations ─────────────────────────────────
fig, axes = plt.subplots(3, 3, figsize=(16, 13))
fig.suptitle("AVL vs Red-Black vs Splay — Execution Time (s)", fontsize=15, fontweight="bold")

for row, pattern in enumerate(PATTERNS):
    for col, op in enumerate(OPS):
        ax = axes[row][col]
        plot_tree(ax, pattern, op, "time")
        ax.set_title(f"{op.capitalize()} — {pattern}", fontweight="bold")
        ax.set_xlabel("Input Size")
        ax.set_ylabel("Time (s)")
        ax.legend(fontsize=8)

plt.tight_layout()
plt.savefig(f"{OUT_DIR}/graph_time.png", dpi=150, bbox_inches="tight")
plt.close()
print("Saved: graph_time.png")

# ── GRAPH 5 : Summary bar chart at n=100,000 random ──────────────────────────
fig, axes = plt.subplots(1, 4, figsize=(18, 5))
fig.suptitle("Summary at n=100,000 — Random Input", fontsize=15, fontweight="bold")

metrics = [
    ("rotations",   "insert", "Insert Rotations"),
    ("comparisons", "search", "Search Comparisons"),
    ("height",      "insert", "Tree Height"),
    ("time",        "insert", "Insert Time (s)"),
]

for i, (metric, op, title) in enumerate(metrics):
    ax = axes[i]
    vals  = [data["random"][op][t].get(100000, {}).get(metric, 0) for t in TREES]
    colors = [COLORS[t] for t in TREES]
    bars  = ax.bar(TREES, vals, color=colors, width=0.5, edgecolor="white", linewidth=1.5)

    for bar, val in zip(bars, vals):
        label = f"{val:.3f}" if metric == "time" else f"{val:,}"
        ax.text(bar.get_x() + bar.get_width()/2, bar.get_height() * 1.02,
                label, ha="center", va="bottom", fontsize=9, fontweight="bold")

    ax.set_title(title, fontweight="bold")
    ax.set_ylabel(title)
    ax.spines["top"].set_visible(False)
    ax.spines["right"].set_visible(False)

plt.tight_layout()
plt.savefig(f"{OUT_DIR}/graph_summary.png", dpi=150, bbox_inches="tight")
plt.close()
print("Saved: graph_summary.png")

print(f"\nAll 5 graphs saved to {OUT_DIR}/")