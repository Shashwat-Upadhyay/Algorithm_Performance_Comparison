#include "avl_tree.h"
#include <stdio.h>
#include <stdlib.h>

long long avl_rotations = 0;
long long avl_comparison = 0;

AVLNode* createAVL(int data) {
    AVLNode* n = (AVLNode*)malloc(sizeof(AVLNode));
    n->data = data;
    n->left = NULL;
    n->right = NULL;
    n->height = 1;
    return n;
}

int max(int a, int b) {
    if(a > b) {
        return a;
    } else {
        return b;
    }
}

int avl_height(AVLNode* n) {
    if(n == NULL) {
        return 0;
    }
    return n->height;
}

int getBalancefac(AVLNode* n) {
    if(n == NULL) {
        return 0;
    }
    return avl_height(n->left) - avl_height(n->right);
}

AVLNode* avl_rightRotate(AVLNode* y) {
    avl_rotations++;
    AVLNode* x = y->left;
    AVLNode* T2 = x->right;

    x->right = y;
    y->left = T2;

    y->height = max(avl_height(y->left),avl_height(y->right)) + 1;
    x->height = max(avl_height(x->left),avl_height(x->right)) + 1;

    return x;
}

AVLNode* avl_leftRotate(AVLNode* x) { 
    avl_rotations++;    
    AVLNode* y = x->right;
    AVLNode* T2 = y->left;

    y->left = x;
    x->right = T2;

    x->height = max(avl_height(x->left),avl_height(x->right)) + 1;
    y->height = max(avl_height(y->left),avl_height(y->right)) + 1;

    return y;
}

AVLNode* avl_insert(AVLNode* node, int key) {
    if(node == NULL) {
        return createAVL(key);
    }

    avl_comparison++;
    if(key < node->data) {
        node->left = avl_insert(node->left, key);
    } else if (key > node->data) {
        node->right = avl_insert(node->right, key);
    } else {
        return node;
    }

    node->height = 1 + max(avl_height(node->left), avl_height(node->right));

    int balance = getBalancefac(node);

    if(balance > 1 && node->left && key < node->left->data) {
        return avl_rightRotate(node);
    }

    if (balance < -1 && node->right && key > node->right->data) {
        return avl_leftRotate(node);
    }

    if(balance > 1 && node->left && key > node->left->data) {
        node->left = avl_leftRotate(node->left);
        return avl_rightRotate(node);
    }

    if(balance < -1 && node->right && key < node->right->data) {
        node->right = avl_rightRotate(node->right);
        return avl_leftRotate(node);
    }
    return node;
}

AVLNode* minValueNode(AVLNode* node) {
    AVLNode* current = node;
    while (current->left != NULL){
        current = current->left;
    }
    return current;
}

AVLNode* avl_delete(AVLNode* root, int data) {
    if(root == NULL) {
        return root;
    }

    avl_comparison++;
    if(data < root->data) {
        root->left = avl_delete(root->left, data);
    }

    else if(data > root->data) {
        root->right = avl_delete(root->right, data);
    }

    else {
        //node with one and no child
        if((root->left == NULL)||(root->right == NULL)) {
            AVLNode* temp = root->left ? root->left : root->right;

            if(temp == NULL) {
                temp = root;
                root = NULL;
            } else {
                *root = *temp; //child data copied by parent 
            }
            free(temp);
        }
        else {
            AVLNode* temp = minValueNode(root->right); //smallest inorder successor
            root->data = temp->data;
            root->right = avl_delete(root->right, temp->data);
        }
    }

    if(root == NULL) {
        return root;
    }

    root->height = 1 + max(avl_height(root->left), avl_height(root->right));
    int balance = getBalancefac(root);

    if(balance > 1 && getBalancefac(root->left) >= 0) {
        return avl_rightRotate(root);
    }

    if(balance > 1 && getBalancefac(root->left) < 0) {
        root->left = avl_leftRotate(root->left);
        return avl_rightRotate(root);
    }

    if(balance < -1 && getBalancefac(root->right) <=0) {
        return avl_leftRotate(root);
    }

    if(balance < -1 && getBalancefac(root->right) > 0) {
        root->right = avl_rightRotate(root->right);
        return avl_leftRotate(root);
    }

    return root;
}

int avl_search(AVLNode* root, int key) {
    if(root == NULL) {
        return 0;
    }

    avl_comparison++;
    if(key == root->data) {
        return 1;
    }
    if(key < root->data) {
        return avl_search(root->left, key);
    }
    return avl_search(root->right, key);
}

void avl_free(AVLNode* root) {
    if (root == NULL) return;
    avl_free(root->left);
    avl_free(root->right);
    free(root);
}





