#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "btree.h"
#include "bplustree.h"

/* ── Input generators (same as BST benchmark) ─────────────────────────────── */
static void gen_random (int arr[], int n) { for (int i = 0; i < n; i++) arr[i] = rand(); }
static void gen_sorted (int arr[], int n) { for (int i = 0; i < n; i++) arr[i] = i; }
static void gen_reverse(int arr[], int n) { for (int i = 0; i < n; i++) arr[i] = n - i; }

static double get_time(void) { return (double)clock() / CLOCKS_PER_SEC; }

/* ── Range query window: 10% of n centred at median ──────────────────────── */
static void range_bounds(int arr[], int n, int *lo, int *hi) {
    int window = n / 10;
    *lo = arr[n / 2] - window / 2;
    *hi = arr[n / 2] + window / 2;
    if (*lo < 0) *lo = 0;
}

int main(void) {
    srand((unsigned)time(NULL));

    int   sizes[]       = {1000, 5000, 10000, 50000, 100000};
    int   num_sizes     = 5;
    int   num_patterns  = 3;
    char *pattern_name[]= {"random", "sorted", "reverse"};

    /* ── Open CSV ────────────────────────────────────────────────────────── */
    FILE *fp = fopen("../result/btree_results.csv", "w");
    if (!fp) fp = fopen("btree_results.csv", "w");
    if (!fp) { perror("Cannot open btree_results.csv"); return 1; }

    /* CSV columns mirror BST benchmark + B-tree extras */
    fprintf(fp, "tree,operation,pattern,n,time,splits,merges,"
                "comparisons,node_accesses,height,fill_factor\n");

    for (int s = 0; s < num_sizes; s++) {
        int  n   = sizes[s];
        int *arr = (int *)malloc(sizeof(int) * n);
        if (!arr) { perror("malloc"); return 1; }

        for (int p = 0; p < num_patterns; p++) {
            if (p == 0) gen_random (arr, n);
            if (p == 1) gen_sorted (arr, n);
            if (p == 2) gen_reverse(arr, n);

            printf("Running n=%-7d  pattern=%s ...\n", n, pattern_name[p]);

            /* ════════════════════════════════════════════════════════════
               B-TREE
               ════════════════════════════════════════════════════════════ */
            BTree *bt = bt_create();
            bt_comparisons = bt_splits = bt_merges = bt_node_accesses = 0;
            bt_height = 0;

            /* insert */
            double start = get_time();
            for (int i = 0; i < n; i++) bt_insert(bt, arr[i]);
            double end = get_time();

            int    bth = bt_get_height(bt->root);
            double btf = bt_fill_factor(bt->root, bth);
            fprintf(fp, "BTree,insert,%s,%d,%.6f,%lld,%lld,%lld,%lld,%d,%.2f\n",
                    pattern_name[p], n, end-start,
                    bt_splits, bt_merges, bt_comparisons, bt_node_accesses, bth, btf);

            /* search */
            bt_comparisons = bt_node_accesses = 0;
            start = get_time();
            for (int i = 0; i < n; i++) bt_search(bt->root, arr[i]);
            end = get_time();

            fprintf(fp, "BTree,search,%s,%d,%.6f,0,0,%lld,%lld,%d,%.2f\n",
                    pattern_name[p], n, end-start,
                    bt_comparisons, bt_node_accesses, bth, btf);

            /* delete */
        //     bt_comparisons = bt_splits = bt_merges = bt_node_accesses = 0;
        //     start = get_time();
        //     for (int i = 0; i < n; i++) bt_delete(bt, arr[i]);
        //     end = get_time();

        //     fprintf(fp, "BTree,delete,%s,%d,%.6f,%lld,%lld,%lld,%lld,%d,%.2f\n",
        //             pattern_name[p], n, end-start,
        //             bt_splits, bt_merges, bt_comparisons, bt_node_accesses, bth, btf);

        //     bt_free(bt->root);
        //     free(bt);

            /* ════════════════════════════════════════════════════════════
               B+ TREE
               ════════════════════════════════════════════════════════════ */
            BPlusTree *bpt = bp_create();
            bp_comparisons = bp_splits = bp_node_accesses = bp_range_accesses = 0;
            bp_height = 0;

            /* insert */
            start = get_time();
            for (int i = 0; i < n; i++) bp_insert(bpt, arr[i]);
            end = get_time();

            int    bph = bp_get_height(bpt->root);
            double bpf = bp_fill_factor(bpt->root);
            fprintf(fp, "BPlusTree,insert,%s,%d,%.6f,%lld,0,%lld,%lld,%d,%.2f\n",
                    pattern_name[p], n, end-start,
                    bp_splits, bp_comparisons, bp_node_accesses, bph, bpf);

            /* search */
            bp_comparisons = bp_node_accesses = 0;
            start = get_time();
            for (int i = 0; i < n; i++) bp_search(bpt->root, arr[i]);
            end = get_time();

            fprintf(fp, "BPlusTree,search,%s,%d,%.6f,0,0,%lld,%lld,%d,%.2f\n",
                    pattern_name[p], n, end-start,
                    bp_comparisons, bp_node_accesses, bph, bpf);

            /* range query — B+ tree's killer feature */
            bp_comparisons = bp_node_accesses = bp_range_accesses = 0;
            int lo, hi;
            range_bounds(arr, n, &lo, &hi);

            start = get_time();
            int found = bp_range_query(bpt, lo, hi);
            end = get_time();

            fprintf(fp, "BPlusTree,range_query,%s,%d,%.6f,0,0,%lld,%lld,%d,%.2f\n",
                    pattern_name[p], n, end-start,
                    bp_comparisons, bp_range_accesses, bph, bpf);
            printf("  B+ range [%d,%d]: %d keys found  (leaf-accesses=%lld)\n",
                   lo, hi, found, bp_range_accesses);

            /* delete */
        //     bp_comparisons = bp_splits = bp_node_accesses = 0;
        //     start = get_time();
        //     /* B+ delete not yet implemented — record zero row as placeholder */
        //     end = get_time();
        //     fprintf(fp, "BPlusTree,delete,%s,%d,%.6f,0,0,0,0,%d,%.2f\n",
        //             pattern_name[p], n, end-start, bph, bpf);

        //     bp_free(bpt->root);
        //     free(bpt);
        }

        free(arr);
    }

    fclose(fp);
    printf("\nBenchmark finished.\n");
    printf("Results saved to btree/result/btree_results.csv\n");
    return 0;
}
