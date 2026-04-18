#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "btree.h"

/*
 * B-Tree of order BT_ORDER (default 5).
 * Each node holds at most BT_MAX_KEYS = BT_ORDER-1 = 4 keys.
 * Each internal node has at most BT_ORDER = 5 children.
 * Minimum keys per non-root node: BT_MIN_KEYS = ceil(BT_ORDER/2)-1 = 2.
 *
 * Split arithmetic (full node has BT_MAX_KEYS keys, indices 0..BT_MAX_KEYS-1):
 *   median_idx = BT_MIN_KEYS          (= 2 for order-5)
 *   left  keeps keys[0 .. median_idx-1],      n = BT_MIN_KEYS
 *   right gets  keys[median_idx+1 .. end],    n = BT_MAX_KEYS - BT_MIN_KEYS - 1
 *   parent receives keys[median_idx] (promoted)
 */

/* ── Metrics ───────────────────────────────────────────────────────────────── */
long long bt_comparisons   = 0;
long long bt_splits        = 0;
long long bt_merges        = 0;
long long bt_node_accesses = 0;
int       bt_height        = 0;

/* ── Constants derived from BT_ORDER ──────────────────────────────────────── */
#define MEDIAN     BT_MIN_KEYS                              /* 2 */
#define RIGHT_N    (BT_MAX_KEYS - BT_MIN_KEYS - 1)         /* 1 */
#define RIGHT_START (BT_MIN_KEYS + 1)                       /* 3 */

/* ── Internal helpers ──────────────────────────────────────────────────────── */
static BtNode *new_node(bool leaf) {
    BtNode *node = calloc(1, sizeof(BtNode));
    if (!node) { perror("calloc"); exit(1); }
    node->leaf = leaf;
    return node;
}

/* ── Public lifecycle ──────────────────────────────────────────────────────── */
BTree *bt_create(void) {
    BTree *t = malloc(sizeof(BTree));
    if (!t) { perror("malloc"); exit(1); }
    t->root = NULL;
    return t;
}

void bt_free(BtNode *node) {
    if (!node) return;
    if (!node->leaf)
        for (int i = 0; i <= node->n; i++) bt_free(node->C[i]);
    free(node);
}

/* ── Search ────────────────────────────────────────────────────────────────── */
bool bt_search(BtNode *root, int k) {
    if (!root) return false;
    bt_node_accesses++;
    int i = 0;
    while (i < root->n) {
        bt_comparisons++;
        if (k == root->keys[i]) return true;
        if (k <  root->keys[i]) break;
        i++;
    }
    if (root->leaf) return false;
    return bt_search(root->C[i], k);
}

/* ── Split full child y (child i of parent) ───────────────────────────────── */
static void split_child(BtNode *parent, int i, BtNode *y) {
    bt_splits++;

    /* right sibling inherits the upper half of y's keys/children */
    BtNode *z = new_node(y->leaf);
    z->n = RIGHT_N;

    for (int j = 0; j < RIGHT_N; j++)
        z->keys[j] = y->keys[j + RIGHT_START];

    if (!y->leaf)
        for (int j = 0; j <= RIGHT_N; j++)
            z->C[j] = y->C[j + RIGHT_START];

    /* truncate y to its left half */
    y->n = MEDIAN;

    /* clear the now-dead upper pointers in y so they can't be followed */
    if (!y->leaf)
        for (int j = RIGHT_START; j <= BT_MAX_KEYS; j++)
            y->C[j] = NULL;

    /* make room in parent for the new child and promoted key */
    for (int j = parent->n; j >= i + 1; j--)
        parent->C[j + 1] = parent->C[j];
    parent->C[i + 1] = z;

    for (int j = parent->n - 1; j >= i; j--)
        parent->keys[j + 1] = parent->keys[j];
    parent->keys[i] = y->keys[MEDIAN];   /* promoted key */
    parent->n++;
}

/* ── Insert into a guaranteed-non-full node ───────────────────────────────── */
static void insert_non_full(BtNode *node, int k) {
    bt_node_accesses++;
    int i = node->n - 1;

    if (node->leaf) {
        /* shift keys right to make room */
        while (i >= 0) {
            bt_comparisons++;
            if (node->keys[i] <= k) break;
            node->keys[i + 1] = node->keys[i];
            i--;
        }
        node->keys[i + 1] = k;
        node->n++;
    } else {
        /* find the child subtree to descend into */
        while (i >= 0) {
            bt_comparisons++;
            if (node->keys[i] <= k) break;
            i--;
        }
        i++;   /* child index */
        if (node->C[i]->n == BT_MAX_KEYS) {
            split_child(node, i, node->C[i]);
            bt_comparisons++;
            if (node->keys[i] < k) i++;
        }
        insert_non_full(node->C[i], k);
    }
}

/* ── Public insert ─────────────────────────────────────────────────────────── */
void bt_insert(BTree *tree, int k) {
    if (!tree->root) {
        tree->root = new_node(true);
        tree->root->keys[0] = k;
        tree->root->n = 1;
        bt_height = 1;
        return;
    }

    if (tree->root->n == BT_MAX_KEYS) {
        /* root is full — grow the tree upward */
        BtNode *s = new_node(false);
        s->C[0] = tree->root;
        split_child(s, 0, tree->root);
        int i = (s->keys[0] < k) ? 1 : 0;
        insert_non_full(s->C[i], k);
        tree->root = s;
        bt_height++;
    } else {
        insert_non_full(tree->root, k);
    }
}

/* ── Delete helpers ────────────────────────────────────────────────────────── */
static int find_key(BtNode *node, int k) {
    int idx = 0;
    while (idx < node->n) {
        bt_comparisons++;
        if (node->keys[idx] >= k) break;
        idx++;
    }
    return idx;
}

static int get_pred(BtNode *node, int idx) {
    BtNode *cur = node->C[idx];
    while (!cur->leaf) cur = cur->C[cur->n];
    return cur->keys[cur->n - 1];
}

static int get_succ(BtNode *node, int idx) {
    BtNode *cur = node->C[idx + 1];
    while (!cur->leaf) cur = cur->C[0];
    return cur->keys[0];
}

static void remove_node(BtNode *node, int k);

static void merge_nodes(BtNode *parent, int idx) {
    bt_merges++;
    BtNode *left  = parent->C[idx];
    BtNode *right = parent->C[idx + 1];

    /* pull parent's separator key down into left */
    left->keys[left->n] = parent->keys[idx];

    /* copy right's keys and children into left */
    for (int i = 0; i < right->n; i++)
        left->keys[left->n + 1 + i] = right->keys[i];

    if (!left->leaf)
        for (int i = 0; i <= right->n; i++)
            left->C[left->n + 1 + i] = right->C[i];

    left->n += 1 + right->n;

    /* remove separator from parent */
    for (int i = idx + 1; i < parent->n; i++)
        parent->keys[i - 1] = parent->keys[i];
    for (int i = idx + 2; i <= parent->n; i++)
        parent->C[i - 1] = parent->C[i];
    parent->C[parent->n] = NULL;
    parent->n--;

    free(right);
}

static void borrow_from_prev(BtNode *parent, int idx) {
    BtNode *child = parent->C[idx];
    BtNode *sib   = parent->C[idx - 1];

    /* shift child's keys/children right by one */
    for (int i = child->n - 1; i >= 0; i--)
        child->keys[i + 1] = child->keys[i];
    if (!child->leaf)
        for (int i = child->n; i >= 0; i--)
            child->C[i + 1] = child->C[i];

    child->keys[0]          = parent->keys[idx - 1];
    parent->keys[idx - 1]   = sib->keys[sib->n - 1];
    if (!child->leaf)
        child->C[0] = sib->C[sib->n];

    sib->C[sib->n] = NULL;
    child->n++;
    sib->n--;
}

static void borrow_from_next(BtNode *parent, int idx) {
    BtNode *child = parent->C[idx];
    BtNode *sib   = parent->C[idx + 1];

    child->keys[child->n] = parent->keys[idx];
    parent->keys[idx]     = sib->keys[0];
    if (!child->leaf)
        child->C[child->n + 1] = sib->C[0];

    for (int i = 1; i < sib->n; i++)   sib->keys[i - 1] = sib->keys[i];
    if (!child->leaf)
        for (int i = 1; i <= sib->n; i++) sib->C[i - 1] = sib->C[i];
    sib->C[sib->n] = NULL;

    child->n++;
    sib->n--;
}

static void fill(BtNode *parent, int idx) {
    if (idx > 0 && parent->C[idx - 1]->n > BT_MIN_KEYS)
        borrow_from_prev(parent, idx);
    else if (idx < parent->n && parent->C[idx + 1]->n > BT_MIN_KEYS)
        borrow_from_next(parent, idx);
    else {
        if (idx < parent->n) merge_nodes(parent, idx);
        else                  merge_nodes(parent, idx - 1);
    }
}

static void remove_from_leaf(BtNode *node, int idx) {
    for (int i = idx + 1; i < node->n; i++)
        node->keys[i - 1] = node->keys[i];
    node->n--;
}

static void remove_from_non_leaf(BtNode *node, int idx) {
    int k = node->keys[idx];
    if (node->C[idx]->n > BT_MIN_KEYS) {
        int pred = get_pred(node, idx);
        node->keys[idx] = pred;
        remove_node(node->C[idx], pred);
    } else if (node->C[idx + 1]->n > BT_MIN_KEYS) {
        int succ = get_succ(node, idx);
        node->keys[idx] = succ;
        remove_node(node->C[idx + 1], succ);
    } else {
        merge_nodes(node, idx);
        remove_node(node->C[idx], k);
    }
}

static void remove_node(BtNode *node, int k) {
    bt_node_accesses++;
    int idx = find_key(node, k);

    if (idx < node->n && node->keys[idx] == k) {
        if (node->leaf) remove_from_leaf(node, idx);
        else            remove_from_non_leaf(node, idx);
    } else {
        if (node->leaf) return;   /* key not present */
        bool last = (idx == node->n);
        if (node->C[idx]->n <= BT_MIN_KEYS) fill(node, idx);
        /* after fill, the child at idx may have been merged into idx-1 */
        if (last && idx > node->n) remove_node(node->C[idx - 1], k);
        else                        remove_node(node->C[idx], k);
    }
}

/* ── Public delete ─────────────────────────────────────────────────────────── */
void bt_delete(BTree *tree, int k) {
    if (!tree->root) return;
    remove_node(tree->root, k);
    if (tree->root->n == 0) {
        BtNode *tmp = tree->root;
        tree->root  = tree->root->leaf ? NULL : tree->root->C[0];
        if (bt_height > 0) bt_height--;
        free(tmp);
    }
}

/* ── Stats ─────────────────────────────────────────────────────────────────── */
int bt_get_height(BtNode *root) {
    if (!root) return 0;
    if (root->leaf) return 1;
    return 1 + bt_get_height(root->C[0]);
}

static void fill_factor_helper(BtNode *node, double *sum, int *count) {
    if (!node) return;
    (*sum)   += (double)node->n / BT_MAX_KEYS;
    (*count) += 1;
    if (!node->leaf)
        for (int i = 0; i <= node->n; i++)
            fill_factor_helper(node->C[i], sum, count);
}

double bt_fill_factor(BtNode *root, int height) {
    (void)height;
    double sum = 0.0; int cnt = 0;
    fill_factor_helper(root, &sum, &cnt);
    return cnt ? (sum / cnt) * 100.0 : 0.0;
}
