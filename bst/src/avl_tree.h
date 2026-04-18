#ifndef AVL_TREE_H
#define AVL_TREE_H

typedef struct AVLNode {
    int data;
    struct AVLNode *left;
    struct AVLNode *right;
    int height;
} AVLNode;

AVLNode* createAVL(int data);
AVLNode* avl_insert(AVLNode* node, int key);
AVLNode* avl_delete(AVLNode* root, int data);
AVLNode* avl_rightRotate(AVLNode* y);
AVLNode* avl_leftRotate(AVLNode* x);
int avl_search(AVLNode* root, int key);
int avl_height(AVLNode* n);
int getBalancefac(AVLNode* n);
void avl_free(AVLNode* root);

extern long long avl_rotations;
extern long long avl_comparison;

#endif