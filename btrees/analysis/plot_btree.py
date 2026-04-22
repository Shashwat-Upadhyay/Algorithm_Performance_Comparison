# import csv
# import os
# import matplotlib
# matplotlib.use('Agg')
# import matplotlib.pyplot as plt
# import numpy as np
# from collections import defaultdict

# CSV_PATH  = "../result/btree_results.csv"
# OUT_DIR   = "graphs"
# os.makedirs(OUT_DIR, exist_ok=True)

# SIZES       = [1000, 5000, 10000, 50000, 100000]
# SIZE_LABELS = ["1K", "5K", "10K", "50K", "100K"]
# PATTERNS    = ["random", "sorted", "reverse"]
# TREES       = ["BTree", "BPlusTree"]

# COLORS  = {"BTree": "#7C3AED", "BPlusTree": "#D97706"}
# MARKERS = {"BTree": "o",       "BPlusTree": "s"}
# LABELS  = {"BTree": "B-Tree",  "BPlusTree": "B+ Tree"}
# LW, MS  = 2.2, 7

# plt.rcParams.update({
#     "figure.facecolor": "#fafafa",
#     "axes.facecolor":   "#ffffff",
#     "axes.grid":        True,
#     "grid.linestyle":   "--",
#     "grid.alpha":       0.4,
#     "font.size":        10,
# })

# # ── Load CSV ──────────────────────────────────────────────────────────────────
# # data[pattern][operation][tree][size] = {time, splits, merges, comparisons,
# #                                          node_accesses, height, fill_factor}
# data = defaultdict(lambda: defaultdict(lambda: defaultdict(dict)))

# with open(CSV_PATH, newline='', encoding='utf-8-sig') as f:
#     for row in csv.DictReader(f):
#         row = {k.strip(): v.strip() if v else "0" for k, v in row.items()}
#         try:
#             data[row["pattern"]][row["operation"]][row["tree"]][int(row["n"])] = {
#                 "time":         float(row["time"]         or 0),
#                 "splits":       int(row["splits"]         or 0),
#                 "merges":       int(row["merges"]         or 0),
#                 "comparisons":  int(row["comparisons"]    or 0),
#                 "node_accesses":int(row["node_accesses"]  or 0),
#                 "height":       int(row["height"]         or 0),
#                 "fill_factor":  float(row["fill_factor"]  or 0),
#             }
#         except (ValueError, KeyError):
#             continue

# def get(pattern, op, tree, metric):
#     d = data[pattern][op][tree]
#     return [d.get(s, {}).get(metric, 0) for s in SIZES]

# def plot_trees(ax, pattern, op, metric):
#     for tree in TREES:
#         vals = get(pattern, op, tree, metric)
#         ax.plot(SIZE_LABELS, vals,
#                 color=COLORS[tree], marker=MARKERS[tree],
#                 linewidth=LW, markersize=MS, label=LABELS[tree])

# # ── GRAPH 1: Node Splits — Insert ─────────────────────────────────────────────
# fig, axes = plt.subplots(1, 3, figsize=(16, 5))
# fig.suptitle("B-Tree vs B+ Tree — Node Splits (Insert)", fontsize=15, fontweight="bold")

# for col, pattern in enumerate(PATTERNS):
#     ax = axes[col]
#     plot_trees(ax, pattern, "insert", "splits")
#     ax.set_title(f"Insert — {pattern}", fontweight="bold")
#     ax.set_xlabel("Input Size")
#     ax.set_ylabel("Splits")
#     ax.legend(fontsize=9)

# plt.tight_layout()
# plt.savefig(f"{OUT_DIR}/btree_splits.png", dpi=150, bbox_inches="tight")
# plt.close()
# print("Saved: btree_splits.png")

# # ── GRAPH 2: Node Accesses — Insert, Search, Delete ──────────────────────────
# fig, axes = plt.subplots(3, 3, figsize=(16, 13))
# fig.suptitle("B-Tree vs B+ Tree — Node Accesses", fontsize=15, fontweight="bold")

# ops = ["insert", "search", "delete"]
# for row, pattern in enumerate(PATTERNS):
#     for col, op in enumerate(ops):
#         ax = axes[row][col]
#         plot_trees(ax, pattern, op, "node_accesses")
#         ax.set_title(f"{op.capitalize()} — {pattern}", fontweight="bold")
#         ax.set_xlabel("Input Size")
#         ax.set_ylabel("Node Accesses")
#         ax.legend(fontsize=8)

# plt.tight_layout()
# plt.savefig(f"{OUT_DIR}/btree_node_accesses.png", dpi=150, bbox_inches="tight")
# plt.close()
# print("Saved: btree_node_accesses.png")

# # ── GRAPH 3: Comparisons — Insert, Search, Delete ────────────────────────────
# # fig, axes = plt.subplots(3, 3, figsize=(16, 13))
# # fig.suptitle("B-Tree vs B+ Tree — Key Comparisons", fontsize=15, fontweight="bold")

# # for row, pattern in enumerate(PATTERNS):
# #     for col, op in enumerate(ops):
# #         ax = axes[row][col]
# #         plot_trees(ax, pattern, op, "comparisons")
# #         ax.set_title(f"{op.capitalize()} — {pattern}", fontweight="bold")
# #         ax.set_xlabel("Input Size")
# #         ax.set_ylabel("Comparisons")
# #         ax.legend(fontsize=8)

# # plt.tight_layout()
# # plt.savefig(f"{OUT_DIR}/btree_comparisons.png", dpi=150, bbox_inches="tight")
# # plt.close()
# # print("Saved: btree_comparisons.png")

# # ── GRAPH 4: Tree Height After Insert ────────────────────────────────────────
# fig, axes = plt.subplots(1, 3, figsize=(16, 5))
# fig.suptitle("B-Tree vs B+ Tree — Tree Height After Insert", fontsize=15, fontweight="bold")

# for col, pattern in enumerate(PATTERNS):
#     ax = axes[col]
#     plot_trees(ax, pattern, "insert", "height")
#     # theoretical log_t(n) reference lines
#     for t, ls in [(5, "--"), (4, ":")]:
#         log_vals = [round(np.log(s) / np.log(t)) for s in SIZES]
#         ax.plot(SIZE_LABELS, log_vals, color="#999", linestyle=ls,
#                 linewidth=1.2, alpha=0.6, label=f"log_{t}(n)")
#     ax.set_title(f"Height — {pattern}", fontweight="bold")
#     ax.set_xlabel("Input Size")
#     ax.set_ylabel("Height")
#     ax.legend(fontsize=8)

# plt.tight_layout()
# plt.savefig(f"{OUT_DIR}/btree_height.png", dpi=150, bbox_inches="tight")
# plt.close()
# print("Saved: btree_height.png")

# # ── GRAPH 5: Execution Time — All operations ──────────────────────────────────
# fig, axes = plt.subplots(3, 3, figsize=(16, 13))
# fig.suptitle("B-Tree vs B+ Tree — Execution Time (s)", fontsize=15, fontweight="bold")

# for row, pattern in enumerate(PATTERNS):
#     for col, op in enumerate(ops):
#         ax = axes[row][col]
#         plot_trees(ax, pattern, op, "time")
#         ax.set_title(f"{op.capitalize()} — {pattern}", fontweight="bold")
#         ax.set_xlabel("Input Size")
#         ax.set_ylabel("Time (s)")
#         ax.legend(fontsize=8)

# plt.tight_layout()
# plt.savefig(f"{OUT_DIR}/btree_time.png", dpi=150, bbox_inches="tight")
# plt.close()
# print("Saved: btree_time.png")

# # ── GRAPH 6: B+ Range Query — Leaf Accesses ───────────────────────────────────
# fig, axes = plt.subplots(1, 3, figsize=(16, 5))
# fig.suptitle("B+ Tree — Range Query Leaf Accesses (B+ exclusive)", fontsize=15, fontweight="bold")

# for col, pattern in enumerate(PATTERNS):
#     ax = axes[col]
#     vals = get(pattern, "range_query", "BPlusTree", "node_accesses")
#     ax.bar(SIZE_LABELS, vals,
#            color=COLORS["BPlusTree"], edgecolor="white", linewidth=1.5, width=0.5)
#     for j, v in enumerate(vals):
#         ax.text(j, v * 1.02, f"{v:,}", ha="center", va="bottom",
#                 fontsize=9, fontweight="bold")
#     ax.set_title(f"Range Query — {pattern}", fontweight="bold")
#     ax.set_xlabel("Input Size")
#     ax.set_ylabel("Leaf Node Accesses")
#     ax.spines["top"].set_visible(False)
#     ax.spines["right"].set_visible(False)

# plt.tight_layout()
# plt.savefig(f"{OUT_DIR}/btree_range_query.png", dpi=150, bbox_inches="tight")
# plt.close()
# print("Saved: btree_range_query.png")

# # ── GRAPH 7: Fill Factor ──────────────────────────────────────────────────────
# fig, axes = plt.subplots(1, 3, figsize=(16, 5))
# fig.suptitle("B-Tree vs B+ Tree — Fill Factor % After Insert", fontsize=15, fontweight="bold")

# for col, pattern in enumerate(PATTERNS):
#     ax = axes[col]
#     for tree in TREES:
#         vals = get(pattern, "insert", tree, "fill_factor")
#         ax.plot(SIZE_LABELS, vals,
#                 color=COLORS[tree], marker=MARKERS[tree],
#                 linewidth=LW, markersize=MS, label=LABELS[tree])
#     ax.axhline(y=50, color="#999", linestyle="--", linewidth=1,
#                alpha=0.6, label="50% (min)")
#     ax.set_title(f"Fill Factor — {pattern}", fontweight="bold")
#     ax.set_xlabel("Input Size")
#     ax.set_ylabel("Fill Factor (%)")
#     ax.set_ylim(0, 105)
#     ax.legend(fontsize=8)

# plt.tight_layout()
# plt.savefig(f"{OUT_DIR}/btree_fill_factor.png", dpi=150, bbox_inches="tight")
# plt.close()
# print("Saved: btree_fill_factor.png")

# # ── GRAPH 8: Summary bar — n=100,000 random ──────────────────────────────────
# fig, axes = plt.subplots(1, 4, figsize=(18, 5))
# fig.suptitle("Summary at n=100,000 — Random Input", fontsize=15, fontweight="bold")

# metrics_summary = [
#     ("splits",        "insert", "Insert Splits"),
#     ("comparisons",   "search", "Search Comparisons"),
#     ("node_accesses", "search", "Search Node Accesses"),
#     ("time",          "insert", "Insert Time (s)"),
# ]
# for i, (metric, op, title) in enumerate(metrics_summary):
#     ax = axes[i]
#     vals   = [data["random"][op][t].get(100000, {}).get(metric, 0) for t in TREES]
#     colors = [COLORS[t] for t in TREES]
#     bars   = ax.bar(TREES, vals, color=colors, width=0.5,
#                     edgecolor="white", linewidth=1.5)
#     for bar, val in zip(bars, vals):
#         label = f"{val:.4f}" if metric == "time" else f"{val:,}"
#         ax.text(bar.get_x() + bar.get_width() / 2, bar.get_height() * 1.02,
#                 label, ha="center", va="bottom", fontsize=9, fontweight="bold")
#     ax.set_title(title, fontweight="bold")
#     ax.set_ylabel(title)
#     ax.set_xticklabels([LABELS[t] for t in TREES])
#     ax.spines["top"].set_visible(False)
#     ax.spines["right"].set_visible(False)

# plt.tight_layout()
# plt.savefig(f"{OUT_DIR}/btree_summary.png", dpi=150, bbox_inches="tight")
# # plt.close()
# # print("Saved: btree_summary.png")

# # print(f"\nAll 8 graphs saved to {OUT_DIR}/")
import csv
import os
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np
from collections import defaultdict

CSV_PATH  = "../result/btree_results.csv"
OUT_DIR   = "graphs"
os.makedirs(OUT_DIR, exist_ok=True)

SIZES       = [1000, 5000, 10000, 50000, 100000]
SIZE_LABELS = ["1K", "5K", "10K", "50K", "100K"]
PATTERNS    = ["random", "sorted", "reverse"]
TREES       = ["BTree", "BPlusTree"]

COLORS  = {"BTree": "#7C3AED", "BPlusTree": "#D97706"}
MARKERS = {"BTree": "o",       "BPlusTree": "s"}
LABELS  = {"BTree": "B-Tree",  "BPlusTree": "B+ Tree"}
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
# data[pattern][operation][tree][size] = {time, splits, merges, comparisons,
#                                          node_accesses, height, fill_factor}
data = defaultdict(lambda: defaultdict(lambda: defaultdict(dict)))

with open(CSV_PATH, newline='', encoding='utf-8-sig') as f:
    for row in csv.DictReader(f):
        row = {k.strip(): v.strip() if v else "0" for k, v in row.items()}
        try:
            data[row["pattern"]][row["operation"]][row["tree"]][int(row["n"])] = {
                "time":         float(row["time"]         or 0),
                "splits":       int(row["splits"]         or 0),
                "merges":       int(row["merges"]         or 0),
                "comparisons":  int(row["comparisons"]    or 0),
                "node_accesses":int(row["node_accesses"]  or 0),
                "height":       int(row["height"]         or 0),
                "fill_factor":  float(row["fill_factor"]  or 0),
            }
        except (ValueError, KeyError):
            continue

def get(pattern, op, tree, metric):
    d = data[pattern][op][tree]
    return [d.get(s, {}).get(metric, 0) for s in SIZES]

def plot_trees(ax, pattern, op, metric):
    for tree in TREES:
        vals = get(pattern, op, tree, metric)
        ax.plot(SIZE_LABELS, vals,
                color=COLORS[tree], marker=MARKERS[tree],
                linewidth=LW, markersize=MS, label=LABELS[tree])

# ── GRAPH 1: Node Splits — Insert ─────────────────────────────────────────────
fig, axes = plt.subplots(1, 3, figsize=(16, 5))
fig.suptitle("B-Tree vs B+ Tree — Node Splits (Insert)", fontsize=15, fontweight="bold")

for col, pattern in enumerate(PATTERNS):
    ax = axes[col]
    plot_trees(ax, pattern, "insert", "splits")
    ax.set_title(f"Insert — {pattern}", fontweight="bold")
    ax.set_xlabel("Input Size")
    ax.set_ylabel("Splits")
    ax.legend(fontsize=9)

plt.tight_layout()
plt.savefig(f"{OUT_DIR}/btree_splits.png", dpi=150, bbox_inches="tight")
plt.close()
print("Saved: btree_splits.png")

# ── GRAPH 2: Node Accesses — Insert & Search ─────────────────────────────────
fig, axes = plt.subplots(3, 2, figsize=(12, 13))
fig.suptitle("B-Tree vs B+ Tree — Node Accesses", fontsize=15, fontweight="bold")

ops = ["insert", "search"]       # delete excluded: B-Tree mirrors insert, B+ is zero
for row, pattern in enumerate(PATTERNS):
    for col, op in enumerate(ops):
        ax = axes[row][col]
        plot_trees(ax, pattern, op, "node_accesses")
        ax.set_title(f"{op.capitalize()} — {pattern}", fontweight="bold")
        ax.set_xlabel("Input Size")
        ax.set_ylabel("Node Accesses")
        ax.legend(fontsize=8)

plt.tight_layout()
plt.savefig(f"{OUT_DIR}/btree_node_accesses.png", dpi=150, bbox_inches="tight")
plt.close()
print("Saved: btree_node_accesses.png")

# ── GRAPH 3: Comparisons — Insert & Search ───────────────────────────────────
fig, axes = plt.subplots(3, 2, figsize=(12, 13))
fig.suptitle("B-Tree vs B+ Tree — Key Comparisons", fontsize=15, fontweight="bold")

for row, pattern in enumerate(PATTERNS):
    for col, op in enumerate(ops):
        ax = axes[row][col]
        plot_trees(ax, pattern, op, "comparisons")
        ax.set_title(f"{op.capitalize()} — {pattern}", fontweight="bold")
        ax.set_xlabel("Input Size")
        ax.set_ylabel("Comparisons")
        ax.legend(fontsize=8)

plt.tight_layout()
plt.savefig(f"{OUT_DIR}/btree_comparisons.png", dpi=150, bbox_inches="tight")
plt.close()
print("Saved: btree_comparisons.png")

# ── GRAPH 4: Tree Height After Insert ────────────────────────────────────────
fig, axes = plt.subplots(1, 3, figsize=(16, 5))
fig.suptitle("B-Tree vs B+ Tree — Tree Height After Insert", fontsize=15, fontweight="bold")

for col, pattern in enumerate(PATTERNS):
    ax = axes[col]
    plot_trees(ax, pattern, "insert", "height")
    # theoretical log_t(n) reference lines
    for t, ls in [(5, "--"), (4, ":")]:
        log_vals = [round(np.log(s) / np.log(t)) for s in SIZES]
        ax.plot(SIZE_LABELS, log_vals, color="#999", linestyle=ls,
                linewidth=1.2, alpha=0.6, label=f"log_{t}(n)")
    ax.set_title(f"Height — {pattern}", fontweight="bold")
    ax.set_xlabel("Input Size")
    ax.set_ylabel("Height")
    ax.legend(fontsize=8)

plt.tight_layout()
plt.savefig(f"{OUT_DIR}/btree_height.png", dpi=150, bbox_inches="tight")
plt.close()
print("Saved: btree_height.png")

# ── GRAPH 5: Execution Time — Insert & Search ────────────────────────────────
fig, axes = plt.subplots(3, 2, figsize=(12, 13))
fig.suptitle("B-Tree vs B+ Tree — Execution Time (s)", fontsize=15, fontweight="bold")

for row, pattern in enumerate(PATTERNS):
    for col, op in enumerate(ops):
        ax = axes[row][col]
        plot_trees(ax, pattern, op, "time")
        ax.set_title(f"{op.capitalize()} — {pattern}", fontweight="bold")
        ax.set_xlabel("Input Size")
        ax.set_ylabel("Time (s)")
        ax.legend(fontsize=8)

plt.tight_layout()
plt.savefig(f"{OUT_DIR}/btree_time.png", dpi=150, bbox_inches="tight")
plt.close()
print("Saved: btree_time.png")

# ── GRAPH 6: B+ Range Query — Leaf Accesses ───────────────────────────────────
fig, axes = plt.subplots(1, 3, figsize=(16, 5))
fig.suptitle("B+ Tree — Range Query Leaf Accesses (B+ exclusive)", fontsize=15, fontweight="bold")

for col, pattern in enumerate(PATTERNS):
    ax = axes[col]
    vals = get(pattern, "range_query", "BPlusTree", "node_accesses")
    ax.bar(SIZE_LABELS, vals,
           color=COLORS["BPlusTree"], edgecolor="white", linewidth=1.5, width=0.5)
    for j, v in enumerate(vals):
        ax.text(j, v * 1.02, f"{v:,}", ha="center", va="bottom",
                fontsize=9, fontweight="bold")
    ax.set_title(f"Range Query — {pattern}", fontweight="bold")
    ax.set_xlabel("Input Size")
    ax.set_ylabel("Leaf Node Accesses")
    ax.spines["top"].set_visible(False)
    ax.spines["right"].set_visible(False)

plt.tight_layout()
plt.savefig(f"{OUT_DIR}/btree_range_query.png", dpi=150, bbox_inches="tight")
plt.close()
print("Saved: btree_range_query.png")

# ── GRAPH 7: Fill Factor ──────────────────────────────────────────────────────
fig, axes = plt.subplots(1, 3, figsize=(16, 5))
fig.suptitle("B-Tree vs B+ Tree — Fill Factor % After Insert", fontsize=15, fontweight="bold")

for col, pattern in enumerate(PATTERNS):
    ax = axes[col]
    for tree in TREES:
        vals = get(pattern, "insert", tree, "fill_factor")
        ax.plot(SIZE_LABELS, vals,
                color=COLORS[tree], marker=MARKERS[tree],
                linewidth=LW, markersize=MS, label=LABELS[tree])
    ax.axhline(y=50, color="#999", linestyle="--", linewidth=1,
               alpha=0.6, label="50% (min)")
    ax.set_title(f"Fill Factor — {pattern}", fontweight="bold")
    ax.set_xlabel("Input Size")
    ax.set_ylabel("Fill Factor (%)")
    ax.set_ylim(0, 105)
    ax.legend(fontsize=8)

plt.tight_layout()
plt.savefig(f"{OUT_DIR}/btree_fill_factor.png", dpi=150, bbox_inches="tight")
plt.close()
print("Saved: btree_fill_factor.png")

# ── GRAPH 8: Summary bar — n=100,000 random ──────────────────────────────────
fig, axes = plt.subplots(1, 4, figsize=(18, 5))
fig.suptitle("Summary at n=100,000 — Random Input", fontsize=15, fontweight="bold")

metrics_summary = [
    ("splits",        "insert", "Insert Splits"),
    ("comparisons",   "search", "Search Comparisons"),
    ("node_accesses", "search", "Search Node Accesses"),
    ("time",          "insert", "Insert Time (s)"),
]
for i, (metric, op, title) in enumerate(metrics_summary):
    ax = axes[i]
    vals   = [data["random"][op][t].get(100000, {}).get(metric, 0) for t in TREES]
    colors = [COLORS[t] for t in TREES]
    bars   = ax.bar(TREES, vals, color=colors, width=0.5,
                    edgecolor="white", linewidth=1.5)
    for bar, val in zip(bars, vals):
        label = f"{val:.4f}" if metric == "time" else f"{val:,}"
        ax.text(bar.get_x() + bar.get_width() / 2, bar.get_height() * 1.02,
                label, ha="center", va="bottom", fontsize=9, fontweight="bold")
    ax.set_title(title, fontweight="bold")
    ax.set_ylabel(title)
    ax.set_xticklabels([LABELS[t] for t in TREES])
    ax.spines["top"].set_visible(False)
    ax.spines["right"].set_visible(False)

plt.tight_layout()
plt.savefig(f"{OUT_DIR}/btree_summary.png", dpi=150, bbox_inches="tight")
plt.close()
print("Saved: btree_summary.png")

print(f"\nAll 8 graphs saved to {OUT_DIR}/")
