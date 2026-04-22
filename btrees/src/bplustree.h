#ifndef BPLUSTREE_H
#define BPLUSTREE_H

#include <stdbool.h>

#define BP_ORDER    4
#define BP_MAX_KEYS (BP_ORDER - 1)

/* ── Metrics (defined in bplustree.c, read by benchmark) ── */
extern long long bp_comparisons;
extern long long bp_splits;
extern long long bp_node_accesses;
extern long long bp_range_accesses;  /* leaf-link traversals during range query */
extern int       bp_height;

typedef struct BPlusNode {
    int              keys[BP_MAX_KEYS];
    struct BPlusNode *children[BP_ORDER];
    struct BPlusNode *next;            /* leaf linked-list pointer */
    int              n;
    bool             leaf;
} BPlusNode;

typedef struct {
    BPlusNode *root;
} BPlusTree;

/* lifecycle */
BPlusTree *bp_create(void);
void       bp_free(BPlusNode *node);

/* operations */
void  bp_insert(BPlusTree *tree, int key);
bool  bp_search(BPlusNode *root, int key);
int   bp_range_query(BPlusTree *tree, int lo, int hi);  /* returns count of keys in [lo,hi] */

/* stats */
int    bp_get_height(BPlusNode *root);
double bp_fill_factor(BPlusNode *root);

#endif /* BPLUSTREE_H */
