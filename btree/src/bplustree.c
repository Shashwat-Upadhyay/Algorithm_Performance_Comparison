#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "bplustree.h"

/* ── Global metrics ────────────────────────────────────────────────────────── */
long long bp_comparisons    = 0;
long long bp_splits         = 0;
long long bp_node_accesses  = 0;
long long bp_range_accesses = 0;
int       bp_height         = 0;

/* ── Internal helpers ──────────────────────────────────────────────────────── */
static BPlusNode *new_node(bool leaf) {
    BPlusNode *node = (BPlusNode *)malloc(sizeof(BPlusNode));
    if (!node) { perror("malloc"); exit(1); }
    node->leaf = leaf;
    node->n    = 0;
    node->next = NULL;
    for (int i = 0; i < BP_ORDER; i++) node->children[i] = NULL;
    return node;
}

/* ── Public lifecycle ──────────────────────────────────────────────────────── */
BPlusTree *bp_create(void) {
    BPlusTree *t = (BPlusTree *)malloc(sizeof(BPlusTree));
    if (!t) { perror("malloc"); exit(1); }
    t->root = new_node(true);   /* B+ tree always starts with one empty leaf */
    bp_height = 1;
    return t;
}

void bp_free(BPlusNode *node) {
    if (!node) return;
    if (!node->leaf)
        for (int i = 0; i <= node->n; i++) bp_free(node->children[i]);
    free(node);
}

/* ── Search ────────────────────────────────────────────────────────────────── */
bool bp_search(BPlusNode *root, int key) {
    if (!root) return false;
    bp_node_accesses++;

    int i = 0;
    while (i < root->n) {
        bp_comparisons++;
        if (key == root->keys[i] && root->leaf) return true;
        if (key <  root->keys[i]) break;
        i++;
    }
    if (root->leaf) return false;
    return bp_search(root->children[i], key);
}

/* ── Split child ───────────────────────────────────────────────────────────── */
static void split_child(BPlusNode *parent, int i, BPlusNode *child) {
    bp_splits++;

    BPlusNode *newNode = new_node(child->leaf);
    int orig_n = child->n;                    /* save BEFORE any truncation */
    int mid    = BP_MAX_KEYS / 2;             /* split point index           */

    if (child->leaf) {
        /*
         * B+ tree leaf split: the key at 'mid' is COPIED up (not moved).
         * Left leaf keeps keys[0..mid-1].
         * Right leaf gets  keys[mid..orig_n-1]  (includes the copied-up key).
         * Parent receives  keys[mid] as separator.
         */
        newNode->n = orig_n - mid;
        for (int j = 0; j < newNode->n; j++)
            newNode->keys[j] = child->keys[j + mid];
        child->n = mid;
        /* maintain leaf linked list */
        newNode->next = child->next;
        child->next   = newNode;
    } else {
        /*
         * Internal node split: key at 'mid' is MOVED up (not copied).
         * Left  keeps keys[0..mid-1]  and children[0..mid].
         * Right gets  keys[mid+1..orig_n-1] and children[mid+1..orig_n].
         * Parent receives keys[mid] as separator.
         */
        newNode->n = orig_n - mid - 1;
        for (int j = 0; j < newNode->n; j++)
            newNode->keys[j] = child->keys[j + mid + 1];
        for (int j = 0; j <= newNode->n; j++)
            newNode->children[j] = child->children[j + mid + 1];
        child->n = mid;
    }

    /* shift parent's children and keys */
    for (int j = parent->n; j >= i + 1; j--)
        parent->children[j + 1] = parent->children[j];
    parent->children[i + 1] = newNode;

    for (int j = parent->n - 1; j >= i; j--)
        parent->keys[j + 1] = parent->keys[j];

    parent->keys[i] = child->leaf ? newNode->keys[0] : child->keys[mid];
    parent->n++;
}

/* ── Insert into non-full node ─────────────────────────────────────────────── */
static void insert_non_full(BPlusNode *node, int key) {
    bp_node_accesses++;
    int i = node->n - 1;

    if (node->leaf) {
        while (i >= 0) {
            bp_comparisons++;
            if (node->keys[i] <= key) break;
            node->keys[i + 1] = node->keys[i];
            i--;
        }
        node->keys[i + 1] = key;
        node->n++;
    } else {
        while (i >= 0) {
            bp_comparisons++;
            if (node->keys[i] <= key) break;
            i--;
        }
        i++;
        if (node->children[i]->n == BP_MAX_KEYS) {
            split_child(node, i, node->children[i]);
            bp_comparisons++;
            if (node->keys[i] < key) i++;
        }
        insert_non_full(node->children[i], key);
    }
}

/* ── Public insert ─────────────────────────────────────────────────────────── */
void bp_insert(BPlusTree *tree, int key) {
    BPlusNode *root = tree->root;

    if (root->n == BP_MAX_KEYS) {
        BPlusNode *newRoot = new_node(false);
        newRoot->children[0] = root;
        split_child(newRoot, 0, root);
        int i = (newRoot->keys[0] < key) ? 1 : 0;
        insert_non_full(newRoot->children[i], key);
        tree->root = newRoot;
        bp_height++;
    } else {
        insert_non_full(root, key);
    }
}

/* ── Range query (B+ tree advantage: leaf linked-list traversal) ─────────── */
int bp_range_query(BPlusTree *tree, int lo, int hi) {
    if (!tree->root) return 0;

    /* Step 1: descend to the first leaf >= lo */
    BPlusNode *node = tree->root;
    while (!node->leaf) {
        bp_node_accesses++;
        int i = 0;
        while (i < node->n) {
            bp_comparisons++;
            if (lo < node->keys[i]) break;
            i++;
        }
        node = node->children[i];
    }

    /* Step 2: scan across leaf linked-list until key > hi */
    int count = 0;
    while (node) {
        bp_range_accesses++;
        for (int i = 0; i < node->n; i++) {
            bp_comparisons++;
            if (node->keys[i] > hi)  return count;
            if (node->keys[i] >= lo) count++;
        }
        node = node->next;
    }
    return count;
}

/* ── Stats ─────────────────────────────────────────────────────────────────── */
int bp_get_height(BPlusNode *root) {
    if (!root) return 0;
    if (root->leaf) return 1;
    return 1 + bp_get_height(root->children[0]);
}

static void fill_factor_helper(BPlusNode *node, double *sum, int *count) {
    if (!node) return;
    (*sum)   += (double)node->n / BP_MAX_KEYS;
    (*count) += 1;
    if (!node->leaf)
        for (int i = 0; i <= node->n; i++)
            fill_factor_helper(node->children[i], sum, count);
}

double bp_fill_factor(BPlusNode *root) {
    double sum = 0.0;
    int    cnt = 0;
    fill_factor_helper(root, &sum, &cnt);
    return cnt ? (sum / cnt) * 100.0 : 0.0;
}
