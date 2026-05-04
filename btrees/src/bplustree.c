#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "bplustree.h"

long long bp_comparisons = 0;
long long bp_splits = 0;
long long bp_node_accesses = 0;
long long bp_range_accesses = 0;
long long bp_merges = 0;
int bp_height = 0;

static BPlusNode *new_node(bool leaf) {
    BPlusNode *node = (BPlusNode *)malloc(sizeof(BPlusNode));
    if (!node) { 
        perror("malloc"); 
        exit(1);
    }
    node->leaf = leaf;
    node->n = 0;
    node->parent = NULL;
    node->next = NULL;
    for (int i = 0; i < BP_ORDER; i++) node->children[i] = NULL;
    return node;
}

BPlusTree *bp_create(void) {
    BPlusTree *t = (BPlusTree *)malloc(sizeof(BPlusTree));
    if (!t) { 
        perror("malloc"); 
        exit(1); 
    }
    t->root = new_node(true);   
    bp_height = 1;
    return t;
}

void bp_free(BPlusNode *node) {
    if (!node) return;
    if (!node->leaf)
        for (int i = 0; i <= node->n; i++) bp_free(node->children[i]);
    free(node);
}

bool bp_search(BPlusNode *root, int key) {
    if (!root) return false;
    bp_node_accesses++;

    int i = 0;
    while (i < root->n) {
        bp_comparisons++;
        if (key == root->keys[i] && root->leaf) return true;
        if (key < root->keys[i]) break;
        i++;
    }
    if (root->leaf) return false;
    return bp_search(root->children[i], key);
}

static void split_child(BPlusNode *parent, int i, BPlusNode *child) {
    bp_splits++;

    BPlusNode *newNode = new_node(child->leaf);
    newNode->parent = parent;
    int orig_n = child->n;                   
    int mid = BP_MAX_KEYS / 2;            

    if (child->leaf) {
        newNode->n = orig_n - mid;
        for (int j = 0; j < newNode->n; j++)
            newNode->keys[j] = child->keys[j + mid];
        child->n = mid;
  
        newNode->next = child->next;
        child->next   = newNode;
    } else {
        newNode->n = orig_n - mid - 1;

        for (int j = 0; j < newNode->n; j++)
            newNode->keys[j] = child->keys[j + mid + 1];

        // for (int j = 0; j <= newNode->n; j++)
        //     newNode->children[j] = child->children[j + mid + 1];

        for (int j = 0; j <= newNode->n; j++) {
            newNode->children[j] = child->children[j + mid + 1];
            if (newNode->children[j])
                newNode->children[j]->parent = newNode;
        }

        child->n = mid;
    }

    for (int j = parent->n; j >= i + 1; j--)
        parent->children[j + 1] = parent->children[j];
    parent->children[i + 1] = newNode;

    for (int j = parent->n - 1; j >= i; j--)
        parent->keys[j + 1] = parent->keys[j];

    parent->keys[i] = child->leaf ? newNode->keys[0] : child->keys[mid];
    parent->n++;
}

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

void bp_insert(BPlusTree *tree, int key) {
    BPlusNode *root = tree->root;

    if (root->n == BP_MAX_KEYS) {
        BPlusNode *newRoot = new_node(false);
        newRoot->children[0] = root;
        root->parent = newRoot;

        split_child(newRoot, 0, root);

        int i = (newRoot->keys[0] < key) ? 1 : 0;
        insert_non_full(newRoot->children[i], key);

        tree->root = newRoot;
        bp_height++;
    } else {
        insert_non_full(root, key);
    }
}

int bp_range_query(BPlusTree *tree, int low, int high) {
    if (!tree->root) return 0;

    BPlusNode *node = tree->root;
    while (!node->leaf) {
        bp_node_accesses++;
        int i = 0;
        while (i < node->n) {
            bp_comparisons++;
            if (low < node->keys[i]) break;
            i++;
        }
        node = node->children[i];
    }

    int count = 0;
    while (node) {
        bp_range_accesses++;
        for (int i = 0; i < node->n; i++) {
            bp_comparisons++;
            if (node->keys[i] > high)  return count;
            if (node->keys[i] >= low) count++;
        }
        node = node->next;
    }
    return count;
}

int bp_get_height(BPlusNode *root) {
    if (!root) return 0;
    if (root->leaf) return 1;
    return 1 + bp_get_height(root->children[0]);
}

static void fill_factor_helper(BPlusNode *node, double *sum, int *count) {
    if (!node) return;
    (*sum) += (double)node->n / BP_MAX_KEYS;
    (*count) += 1;
    if (!node->leaf)
        for (int i = 0; i <= node->n; i++)
            fill_factor_helper(node->children[i], sum, count);
}

double bp_fill_factor(BPlusNode *root) {
    double sum = 0.0;
    int cnt = 0;
    fill_factor_helper(root, &sum, &cnt);
    return cnt ? (sum / cnt) * 100.0 : 0.0;
}

/* ── find leaf containing key ─────────────────────────────── */
static BPlusNode *find_leaf(BPlusNode *root, int key) {
    if (!root) return NULL;

    BPlusNode *node = root;
    while (!node->leaf) {
        int i = 0;
        while (i < node->n && key >= node->keys[i]) i++;
        node = node->children[i];
    }
    return node;
}

/* ── child index in parent ────────────────────────────────── */
static int child_index(BPlusNode *parent, BPlusNode *child) {
    for (int i = 0; i <= parent->n; i++)
        if (parent->children[i] == child) return i;
    return -1;
}

/* ── remove key from leaf ─────────────────────────────────── */
static bool remove_from_leaf(BPlusNode *leaf, int key) {
    int i = 0;
    while (i < leaf->n && leaf->keys[i] != key) i++;
    if (i == leaf->n) return false;

    for (; i < leaf->n - 1; i++)
        leaf->keys[i] = leaf->keys[i + 1];

    leaf->n--;
    return true;
}

/* ── update parent separator after leaf first key changes ─── */
static void update_parent_key(BPlusNode *node) {
    if (!node || !node->parent) return;

    BPlusNode *parent = node->parent;
    int idx = child_index(parent, node);

    if (idx > 0 && node->n > 0)
        parent->keys[idx - 1] = node->keys[0];
}

/* ── borrow from left sibling ─────────────────────────────── */
static bool borrow_left(BPlusNode *node, int idx) {
    if (idx == 0) return false;

    BPlusNode *parent = node->parent;
    BPlusNode *left   = parent->children[idx - 1];

    if (left->n <= BP_MIN_KEYS) return false;

    for (int i = node->n; i > 0; i--)
        node->keys[i] = node->keys[i - 1];

    node->keys[0] = left->keys[left->n - 1];
    node->n++;
    left->n--;

    parent->keys[idx - 1] = node->keys[0];
    return true;
}

/* ── borrow from right sibling ────────────────────────────── */
static bool borrow_right(BPlusNode *node, int idx) {
    BPlusNode *parent = node->parent;
    if (idx >= parent->n) return false;

    BPlusNode *right = parent->children[idx + 1];

    if (right->n <= BP_MIN_KEYS) return false;

    node->keys[node->n] = right->keys[0];
    node->n++;

    for (int i = 0; i < right->n - 1; i++)
        right->keys[i] = right->keys[i + 1];

    right->n--;

    parent->keys[idx] = right->keys[0];
    return true;
}

/* ── forward declaration ──────────────────────────────────── */
static void rebalance_internal(BPlusTree *tree, BPlusNode *node);

/* ── merge leaf with sibling ──────────────────────────────── */
static void merge_leaf(BPlusTree *tree, BPlusNode *node, int idx) {
    BPlusNode *parent = node->parent;

    BPlusNode *left;
    BPlusNode *right;
    int sep;

    if (idx > 0) {
        left  = parent->children[idx - 1];
        right = node;
        sep   = idx - 1;
    } else {
        left  = node;
        right = parent->children[idx + 1];
        sep   = idx;
    }

    for (int i = 0; i < right->n; i++)
        left->keys[left->n + i] = right->keys[i];

    left->n += right->n;
    bp_merges++;

    left->next = right->next;

    for (int i = sep; i < parent->n - 1; i++) {
        parent->keys[i] = parent->keys[i + 1];
        parent->children[i + 1] = parent->children[i + 2];
    }

    parent->n--;
    free(right);

    if (parent == tree->root && parent->n == 0) {
        tree->root = left;
        left->parent = NULL;
        free(parent);
        return;
    }

    if (parent != tree->root && parent->n < BP_MIN_KEYS)
        rebalance_internal(tree, parent);
}

/* ── internal rebalance ───────────────────────────────────── */
static void rebalance_internal(BPlusTree *tree, BPlusNode *node) {
    if (node == tree->root) return;

    BPlusNode *parent = node->parent;
    int idx = child_index(parent, node);

    /* try left borrow */
    if (idx > 0) {
        BPlusNode *left = parent->children[idx - 1];

        if (left->n > BP_MIN_KEYS) {
            for (int i = node->n; i > 0; i--)
                node->keys[i] = node->keys[i - 1];

            for (int i = node->n + 1; i > 0; i--)
                node->children[i] = node->children[i - 1];

            node->keys[0] = parent->keys[idx - 1];
            node->children[0] = left->children[left->n];
            if (node->children[0]) node->children[0]->parent = node;

            parent->keys[idx - 1] = left->keys[left->n - 1];

            node->n++;
            left->n--;
            return;
        }
    }

    /* try right borrow */
    if (idx < parent->n) {
        BPlusNode *right = parent->children[idx + 1];

        if (right->n > BP_MIN_KEYS) {
            node->keys[node->n] = parent->keys[idx];
            node->children[node->n + 1] = right->children[0];
            if (node->children[node->n + 1])
                node->children[node->n + 1]->parent = node;

            parent->keys[idx] = right->keys[0];

            for (int i = 0; i < right->n - 1; i++)
                right->keys[i] = right->keys[i + 1];

            for (int i = 0; i < right->n; i++)
                right->children[i] = right->children[i + 1];

            node->n++;
            right->n--;
            return;
        }
    }

    /* merge */
    BPlusNode *left;
    BPlusNode *right;
    int sep;

    if (idx > 0) {
        left = parent->children[idx - 1];
        right = node;
        sep = idx - 1;
    } else {
        left = node;
        right = parent->children[idx + 1];
        sep = idx;
    }

    left->keys[left->n] = parent->keys[sep];
    left->n++;

    for (int i = 0; i < right->n; i++)
        left->keys[left->n + i] = right->keys[i];

    for (int i = 0; i <= right->n; i++) {
        left->children[left->n + i] = right->children[i];
        if (right->children[i])
            right->children[i]->parent = left;
    }

    left->n += right->n;
    bp_merges++;

    for (int i = sep; i < parent->n - 1; i++) {
        parent->keys[i] = parent->keys[i + 1];
        parent->children[i + 1] = parent->children[i + 2];
    }

    parent->n--;
    free(right);

    if (parent == tree->root && parent->n == 0) {
        tree->root = left;
        left->parent = NULL;
        free(parent);
        return;
    }

    if (parent != tree->root && parent->n < BP_MIN_KEYS)
        rebalance_internal(tree, parent);
}

/* ── public delete ────────────────────────────────────────── */
void bp_delete(BPlusTree *tree, int key) {
    if (!tree || !tree->root) return;

    BPlusNode *leaf = find_leaf(tree->root, key);
    if (!leaf) return;

    if (!remove_from_leaf(leaf, key)) return;

    if (leaf == tree->root) {
        if (leaf->n == 0) {
            free(tree->root);
            tree->root = new_node(true);
        }
        return;
    }

    update_parent_key(leaf);

    if (leaf->n >= BP_MIN_KEYS)
        return;

    int idx = child_index(leaf->parent, leaf);

    if (borrow_left(leaf, idx)) return;
    if (borrow_right(leaf, idx)) return;

    merge_leaf(tree, leaf, idx);
}