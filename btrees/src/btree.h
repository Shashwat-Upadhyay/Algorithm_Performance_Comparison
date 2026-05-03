#ifndef BTREE_H
#define BTREE_H

#include <stdbool.h>

#define BT_ORDER 5
#define BT_MIN_CHILD ((BT_ORDER + 1) / 2)
#define BT_MIN_KEYS (BT_MIN_CHILD - 1)
#define BT_MAX_KEYS (BT_ORDER - 1)

extern long long bt_comparisons;
extern long long bt_splits;
extern long long bt_merges;
extern long long bt_node_accesses;
extern int bt_height;

typedef struct BtNode {
    int keys[BT_ORDER];    
    struct BtNode *C[BT_ORDER + 1];  
    int n;
    bool leaf;
} BtNode;

typedef struct BTree {
    BtNode *root;
} BTree;

BTree *bt_create(void);
void bt_free(BtNode *node);

void bt_insert(BTree *tree, int k);
bool bt_search(BtNode *root, int k);
void bt_delete(BTree *tree, int k);

int bt_get_height(BtNode *root);
double bt_fill_factor(BtNode *root, int height);

#endif
