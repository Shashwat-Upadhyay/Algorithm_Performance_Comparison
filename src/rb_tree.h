#ifndef RBT_TREE_H
#define RBT_TREE_H

#define RED 1
#define BLACK 0

typedef struct RBNode {
    int data;
    int color;
    struct RBNode *left, *right, *parent;
} RBNode;

RBNode* createRB(int data);
RBNode* rb_insert(RBNode* root, int data);
RBNode* rb_search(RBNode* root, int key);
RBNode* rb_delete(RBNode* root, int key);
int rb_height(RBNode* node);
void rb_leftRotate(RBNode* x, RBNode** root);
void rb_rightRotate(RBNode* y, RBNode** root);
void rb_free(RBNode* root);

extern long long rb_rotations;
extern long long rb_comparison;

#endif
