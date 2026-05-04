#ifndef BPLUSTREE_H
#define BPLUSTREE_H

#include <stdbool.h>

#define BP_ORDER 5
#define BP_MAX_KEYS (BP_ORDER - 1)
#define BP_MIN_KEYS ((BP_ORDER - 1) / 2)

extern long long bp_comparisons;
extern long long bp_splits;
extern long long bp_node_accesses;
extern long long bp_range_accesses;
extern long long bp_merges;  
extern int bp_height;

typedef struct BPlusNode {
    int keys[BP_MAX_KEYS];
    struct BPlusNode *children[BP_ORDER];
    struct BPlusNode *next;     
    struct BPlusNode *parent;        
    int n;
    bool leaf;
} BPlusNode;

typedef struct BPlusTree {
    BPlusNode *root;
} BPlusTree;

BPlusTree *bp_create(void);
void bp_free(BPlusNode *node);

void bp_insert(BPlusTree *tree, int key);
bool bp_search(BPlusNode *root, int key);
int bp_range_query(BPlusTree *tree, int low, int high);  
void bp_delete(BPlusTree *tree, int key);
int bp_get_height(BPlusNode *root);
double bp_fill_factor(BPlusNode *root);

#endif
