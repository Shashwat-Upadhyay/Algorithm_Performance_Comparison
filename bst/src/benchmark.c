#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "avl_tree.h"
#include "rb_tree.h"
#include "splay_tree.h"

void generate_random(int arr[], int n) {
    for (int i = 0; i < n; i++) arr[i] = rand();
}

void generate_sorted(int arr[], int n) {
    for (int i = 0; i < n; i++) arr[i] = i;
}

void generate_reverse(int arr[], int n) {
    for (int i = 0; i < n; i++) arr[i] = n - i;
}

double get_time() {
    return (double)clock() / CLOCKS_PER_SEC;
}

int main() {
    srand(time(NULL));

    int   sizes[]        = {1000, 5000, 10000, 50000, 100000};
    int   num_sizes      = 5;
    int   num_patterns   = 3;
    char *pattern_name[] = {"random", "sorted", "reverse"};

    system("if not exist ..\\result mkdir ..\\result");
    FILE *fp = fopen("D:/Sem_4/ADS_CP/result/results.csv", "w");
    if (!fp) fp = fopen("../result/results.csv", "w");
    if (!fp) fp = fopen("results.csv", "w");
    if (!fp) { perror("Cannot open results.csv"); return 1; }

    fprintf(fp, "tree,operation,pattern,n,time,rotations,comparisons,height\n");

    for (int s = 0; s < num_sizes; s++) {
        int  n   = sizes[s];
        int *arr = malloc(sizeof(int) * n);

        for (int p = 0; p < num_patterns; p++) {
            if (p == 0) generate_random (arr, n);
            if (p == 1) generate_sorted (arr, n);
            if (p == 2) generate_reverse(arr, n);

            printf("Running n=%-7d  pattern=%s ...\n", n, pattern_name[p]);

            //AVL insert
            AVLNode *avl_root = NULL;
            avl_rotations = 0;
            avl_comparison = 0;

            double start = get_time();
            for (int i = 0; i < n; i++)
                avl_root = avl_insert(avl_root, arr[i]);
            double end = get_time();

            int avl_h = avl_height(avl_root);
            fprintf(fp, "AVL,insert,%s,%d,%lf,%lld,%lld,%d\n",
                    pattern_name[p], n, end-start,
                    avl_rotations, avl_comparison, avl_h);
            
            //AVL search
            avl_comparison = 0; 

            start = get_time();
            for (int i = 0; i < n; i++)
                avl_search(avl_root, arr[i]);
            end = get_time();

            fprintf(fp, "AVL,search,%s,%d,%lf,0,%lld,%d\n",
                    pattern_name[p], n, end-start,
                    avl_comparison, avl_h);
            
            //AVL delete
            avl_rotations = 0;
            avl_comparison = 0;

            start = get_time();
            for (int i = 0; i < n; i++)
                avl_root = avl_delete(avl_root, arr[i]);
            end = get_time();

            fprintf(fp, "AVL,delete,%s,%d,%lf,%lld,%lld,%d\n",
                    pattern_name[p], n, end-start,
                    avl_rotations, avl_comparison, avl_h);

            avl_free(avl_root);
            
            //RB insert
            RBNode *rb_rt = NULL;
            rb_rotations = 0;
            rb_comparison = 0;

            start = get_time();
            for (int i = 0; i < n; i++)
                rb_rt = rb_insert(rb_rt, arr[i]);
            end = get_time();

            int rb_h = rb_height(rb_rt);
            fprintf(fp, "RB,insert,%s,%d,%lf,%lld,%lld,%d\n",
                    pattern_name[p], n, end-start,
                    rb_rotations, rb_comparison, rb_h);
            
            //RB search
            rb_comparison = 0;

            start = get_time();
            for (int i = 0; i < n; i++)
                rb_search(rb_rt, arr[i]);
            end = get_time();

            fprintf(fp, "RB,search,%s,%d,%lf,0,%lld,%d\n",
                    pattern_name[p], n, end-start,
                    rb_comparison, rb_h);
            
            //RB delete
            rb_rotations = 0;
            rb_comparison = 0;

            start = get_time();
            for (int i = 0; i < n; i++)
                rb_rt = rb_delete(rb_rt, arr[i]);
            end = get_time();

            fprintf(fp, "RB,delete,%s,%d,%lf,%lld,%lld,%d\n",
                    pattern_name[p], n, end-start,
                    rb_rotations, rb_comparison, rb_h);

            rb_free(rb_rt);
            
            //Splay insert
            splay_root = NULL;
            splay_rotations = 0;
            splay_comparison = 0;
 
            start = get_time();
            for (int i = 0; i < n; i++)
                spt_insert(arr[i]);
            end = get_time();
 
            int sp_h = splay_height(splay_root);
            fprintf(fp, "Splay,insert,%s,%d,%lf,%lld,%lld,%d\n",
                    pattern_name[p], n, end-start,
                    splay_rotations, splay_comparison, sp_h);
            
            //Splay search    
            splay_comparison = 0;
 
            start = get_time();
            for (int i = 0; i < n; i++)
                spt_search(arr[i]);
            end = get_time();
 
            sp_h = splay_height(splay_root);
            fprintf(fp, "Splay,search,%s,%d,%lf,0,%lld,%d\n",
                    pattern_name[p], n, end-start,
                    splay_comparison, sp_h);

            //Splay delete
            splay_rotations = 0;
            splay_comparison = 0;
 
            start = get_time();
            for (int i = 0; i < n; i++)
                spt_delete(arr[i]);
            end = get_time();
 
            fprintf(fp, "Splay,delete,%s,%d,%lf,%lld,%lld,%d\n",
                    pattern_name[p], n, end-start,
                    splay_rotations, splay_comparison, 0);
 
            splay_free(splay_root);
            splay_root = NULL;
                
        }

        free(arr);
    }

    fclose(fp);

    printf("\nBenchmark finished.\n");
    printf("Results saved to results/results.csv\n");

    return 0;
}

