#include "splay_tree.h"
#include <stdio.h>
#include <stdlib.h>

long long splay_rotations = 0;
long long splay_comparison = 0;
SplayNode* splay_root = NULL;

SplayNode* createSPT(int key) {
    SplayNode* t = malloc(sizeof(SplayNode));
    t->key = key;
    t->left = t->right = t->parent = NULL; 
    return t;
}

SplayNode* rr_rotate(SplayNode* x) {
    splay_rotations++;
    SplayNode* y = x->left;

    x->left = y->right; 
    if(y->right) 
        y->right->parent = x; 
    y->parent = x->parent;

    if(x->parent == NULL) 
        splay_root = y; 
    else if(x == x->parent->left) 
        x->parent->left = y; 
    else 
        x->parent->right = y; 
    y->right = x; 
    x->parent = y; 
    return y; 
}

SplayNode* ll_rotate(SplayNode* x) { 
    splay_rotations++;
    SplayNode* y = x->right; 

    x->right = y->left; 
    if(y->left) 
        y->left->parent = x; 
    y->parent = x->parent;

    if(x->parent == NULL) 
        splay_root = y; 
    else if(x == x->parent->left) 
        x->parent->left = y; 
    else 
        x->parent->right = y; 
    y->left = x; 
    x->parent = y; 
    return y; 
}

SplayNode* splay(SplayNode* x) { 
    while(x->parent != NULL) { 
        if(x->parent->parent == NULL) { 
            if(x == x->parent->left) 
                rr_rotate(x->parent); 
            else 
                ll_rotate(x->parent);

        } else if(x == x->parent->left && x->parent == x->parent->parent->left) { 
            rr_rotate(x->parent->parent); 
            rr_rotate(x->parent);

        } else if(x == x->parent->right && x->parent == x->parent->parent->right) { 
            ll_rotate(x->parent->parent); 
            ll_rotate(x->parent);

        } else if(x == x->parent->right && x->parent == x->parent->parent->left) { 
            ll_rotate(x->parent); 
            rr_rotate(x->parent); 

        } else { 
            rr_rotate(x->parent); 
            ll_rotate(x->parent); 
        } 
    } 
    return x; 
}

SplayNode* spt_search(int key) { 
    SplayNode* temp = splay_root; 

    while(temp) { 
        splay_comparison++;
        if(key == temp->key) { 
            splay_root = splay(temp); 
            return splay_root; 
        } else if (key < temp->key) 
            temp = temp->left; 
        else 
            temp = temp->right; 
        } 
    return NULL; 
}

SplayNode* spt_insert(int key) { 
    SplayNode* temp = splay_root; 
    SplayNode* parent = NULL;

    while(temp) { 
        parent = temp;
        splay_comparison++;

        if(key < temp->key) 
            temp = temp->left; 
        else if (key > temp->key) 
            temp = temp->right; 
        else { 
            splay_root = splay(temp); 
            return splay_root;
        } 
    }

    SplayNode* n = createSPT(key);
    n->parent = parent;

    if(parent == NULL)
        splay_root = n;
    else if(key < parent->key)
        parent->left = n;
    else
        parent->right = n;

    splay_root = splay(n);
    return splay_root;
}

SplayNode* spt_delete(int key) { 
    SplayNode* node = spt_search(key); 
    if(node == NULL) return splay_root;

    if(node->left == NULL) { 
        splay_root = node->right; 
        if(splay_root) 
            splay_root->parent = NULL;
    } else { 
        SplayNode* temp = node->left; 
        temp->parent = NULL; 
        while(temp->right) 
            temp = temp->right;
             
        splay_root = splay(temp); 
        splay_root->right = node->right; 

        if(node->right) 
            node->right->parent = splay_root;
    } 
    free(node); 
    return splay_root;
}

int splay_height(SplayNode* root) {
    if (root == NULL) return 0;
    int l = splay_height(root->left);
    int r = splay_height(root->right);
    return (l > r ? l : r) + 1;
}

void splay_free(SplayNode* root) {
    if (root == NULL) return;
    splay_free(root->left);
    splay_free(root->right);
    free(root);
}

