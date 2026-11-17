#ifndef FLIGHT_RECORDS_H
#define FLIGHT_RECORDS_H

#include "flight_queue.h"

typedef struct BSTNode {
    Flight        data;
    struct BSTNode *left;
    struct BSTNode *right;
} BSTNode;

BSTNode *bst_insert(BSTNode *root, Flight *f);
BSTNode *bst_search(BSTNode *root, const char *id);
void     bst_search_all(BSTNode *root, const char *id, int *found_count);
void     bst_inorder(BSTNode *root);
BSTNode *bst_delete(BSTNode *root, const Flight *target);
void     bst_free(BSTNode *root);

#endif