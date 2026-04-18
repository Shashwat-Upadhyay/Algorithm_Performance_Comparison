#ifndef SPLAY_TREE_H
#define SPLAY_TREE_H

typedef struct SplayNode {
    int key;
    struct SplayNode *left, *right, *parent;
} SplayNode;

SplayNode* createSPT(int key);
SplayNode* rr_rotate(SplayNode* x);
SplayNode* ll_rotate(SplayNode* x);
SplayNode* splay(SplayNode* x);
SplayNode* spt_search(int key);
SplayNode* spt_insert(int key);
SplayNode* spt_delete(int key);
int splay_height(SplayNode* root);
void splay_free(SplayNode* root);

extern long long splay_rotations;
extern long long splay_comparison;
extern SplayNode* splay_root;

#endif