#include "rb_tree.h"
#include <stdio.h>
#include <stdlib.h>

long long rb_rotations = 0; 
long long rb_comparison = 0;

RBNode* createRB(int data) {
    RBNode* n = (RBNode*)malloc(sizeof(RBNode));
    n->data = data;
    n->color = RED;
    n->left = n->right = n->parent = NULL;
    return n;
}

void rb_leftRotate(RBNode* x, RBNode** root) {
    rb_rotations++;
    RBNode* y = x->right;
    x->right = y->left;

    if (y->left)
        y->left->parent = x;

    y->parent = x->parent;

    if (!x->parent)
        *root = y;
    else if (x == x->parent->left)
        x->parent->left = y;
    else
        x->parent->right = y;

    y->left = x;
    x->parent = y;
}

void rb_rightRotate(RBNode* y, RBNode** root) {
    rb_rotations++;
    RBNode* x = y->left;
    y->left = x->right;

    if (x->right)
        x->right->parent = y;

    x->parent = y->parent;

    if (!y->parent)
        *root = x;
    else if (y == y->parent->left)
        y->parent->left = x;
    else
        y->parent->right = x;

    x->right = y;
    y->parent = x;
}

void fixInsert(RBNode** root, RBNode* z) {
    while (z != *root && z->parent && z->parent->color == RED) {
        RBNode* parent = z->parent;
        RBNode* grand = parent ? parent->parent : NULL;
        if (!grand) break; 

        if (parent == grand->left) {
            RBNode* y = grand->right;
            if (y && y->color == RED) {
                parent->color = BLACK;
                y->color = BLACK;
                grand->color = RED;
                z = grand;
            } else {
                if (z == parent->right) {
                    z = parent;
                    rb_leftRotate(z, root);
                    parent = z->parent;
                    grand = parent ? parent->parent : NULL;
                    if (!parent || !grand) continue;
                }
                parent->color = BLACK;
                if (parent->parent)
                    parent->parent->color = RED;
                rb_rightRotate(parent->parent, root);
            }
        } else {
            RBNode* y = grand->left;
            if (y && y->color == RED) {
                parent->color = BLACK;
                y->color = BLACK;
                grand->color = RED;
                z = grand;
            } else {
                if (z == parent->left) {
                    z = parent;
                    rb_rightRotate(z, root);
                    parent = z->parent; 
                    grand = parent ? parent->parent : NULL;
                    if (!parent || !grand) continue;
                }
                parent->color = BLACK;
                if (parent->parent)
                    parent->parent->color = RED;
                rb_leftRotate(parent->parent, root);
            }
        }
    }
    if (root && *root)
        (*root)->color = BLACK;
}

RBNode* rb_insert(RBNode* root, int data) {
    RBNode* z = createRB(data);
    RBNode *y = NULL, *x = root;

    while (x) {
        y = x;
        rb_comparison++;
        if (z->data < x->data)
            x = x->left;
        else
            x = x->right;
    }

    z->parent = y;

    if (!y)
        root = z;
    else if (z->data < y->data)
        y->left = z;
    else
        y->right = z;

    fixInsert(&root, z);
    return root;
}

RBNode* rb_search(RBNode* root, int key) {
    if (!root || root->data == key)
        return root;
    
    rb_comparison++;
    if (key < root->data)
        return rb_search(root->left, key);
    return rb_search(root->right, key);
}

RBNode* minimum(RBNode* node) {
    while (node->left)
        node = node->left;
    return node;
}

void fixDelete(RBNode** root, RBNode* x) {
    while (x != *root && x && x->color == BLACK) {
        if (!x || !x->parent) break;  
        if (x == x->parent->left) {
            RBNode* w = x->parent->right;
            if (!w) break;  
            if (w->color == RED) {
                w->color = BLACK;
                x->parent->color = RED;
                rb_leftRotate(x->parent, root);
                w = x->parent->right;
                if (!w) break;  
            }
            if ((!w->left || w->left->color == BLACK) && (!w->right || w->right->color == BLACK)) {
                w->color = RED;
                x = x->parent;
            } else {
                if (!w->right || w->right->color == BLACK) {
                    if (w->left)
                        w->left->color = BLACK;
                    w->color = RED;
                    rb_rightRotate(w, root);
                    w = x->parent->right;
                    if (!w) break;  
                }
                w->color = x->parent->color;
                x->parent->color = BLACK;
                if (w->right)
                    w->right->color = BLACK;
                rb_leftRotate(x->parent, root);
                x = *root;
            }
        } else {
            RBNode* w = x->parent->left;
            if (!w) break;  
            if (w->color == RED) {
                w->color = BLACK;
                x->parent->color = RED;
                rb_rightRotate(x->parent, root);
                w = x->parent->left;
                if (!w) break;  
            }
            if ((!w->left || w->left->color == BLACK) && (!w->right || w->right->color == BLACK)) {
                w->color = RED;
                x = x->parent;
            } else {
                if (!w->left || w->left->color == BLACK) {
                    if (w->right)
                        w->right->color = BLACK;
                    w->color = RED;
                    rb_leftRotate(w, root);
                    w = x->parent->left;
                    if (!w) break;  
                }
                w->color = x->parent->color;
                x->parent->color = BLACK;
                if (w->left)
                    w->left->color = BLACK;
                rb_rightRotate(x->parent, root);
                x = *root;
            }
        }
    }
    if (x) {
        x->color = BLACK;
    }
}

RBNode* rb_delete(RBNode* root, int key) {
    RBNode* z = rb_search(root, key);
    if (!z) return root;

    RBNode *y = z, *x;
    int yOrigColor = y->color;

    if (!z->left) {
        x = z->right;
        if (z->parent == NULL)
            root = z->right;
        else if (z == z->parent->left)
            z->parent->left = z->right;
        else
            z->parent->right = z->right;
        if (z->right)
            z->right->parent = z->parent;
        free(z);
    } else if (!z->right) {
        x = z->left;
        if (z->parent == NULL)
            root = z->left;
        else if (z == z->parent->left)
            z->parent->left = z->left;
        else
            z->parent->right = z->left;
        z->left->parent = z->parent;
        free(z);
    } else {
        y = minimum(z->right);
        yOrigColor = y->color;
        x = y->right;
        z->data = y->data;
        if (y->right)
            y->right->parent = y->parent;
        if (y->parent->left == y)
            y->parent->left = y->right;
        else
            y->parent->right = y->right;
        free(y);
    }

    if (yOrigColor == BLACK) {
        fixDelete(&root, x);
    }
    return root;
}

int rb_height(RBNode* node) {
    if (!node) return 0;
    int l = rb_height(node->left);
    int r = rb_height(node->right);
    return (l > r ? l : r) + 1;
}

void rb_free(RBNode* root) {
    if (!root) return;
    rb_free(root->left);
    rb_free(root->right);
    free(root);
}


