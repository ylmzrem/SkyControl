#include "flight_records.h"

/*BINARY SEARCH TREE  –  flight archive
  Key = Flight ID (lexicographic) */

// Helper: check if two flight events are identical
static int flight_is_equal(const Flight *f1, const Flight *f2) {
  // Compare relevant fields to uniquely identify a flight event.
  return (strcmp(f1->id, f2->id) == 0 && f1->fuel == f2->fuel &&
          f1->is_landing == f2->is_landing && f1->emergency == f2->emergency);
}

static BSTNode *make_bst_node(Flight *f) {
  BSTNode *n = (BSTNode *)malloc(sizeof(BSTNode));
  if (!n) {
    fprintf(stderr, "malloc failed\n");
    exit(1);
  }
  n->data = *f;
  n->left = n->right = NULL;
  return n;
}

BSTNode *bst_insert(BSTNode *root, Flight *f) {
  if (!root)
    return make_bst_node(f);
  int cmp = strcmp(f->id, root->data.id);
  if (cmp < 0) {
    root->left = bst_insert(root->left, f);
  } else {
    root->right = bst_insert(root->right, f);
  }
  return root;
}

BSTNode *bst_search(BSTNode *root, const char *id) {
  if (!root)
    return NULL;
  int cmp = strcmp(id, root->data.id);
  if (cmp == 0)
    return root;
  if (cmp < 0)
    return bst_search(root->left, id);
  return bst_search(root->right, id);
}

// Search for all occurrences of a flight ID and print them
void bst_search_all(BSTNode *root, const char *id, int *found_count) {
  if (!root)
    return;
  // We need to traverse both sides of the tree because the IDs are sorted according to our sorting rule (smaller IDs on the left, equal/greater IDs on the right).
  // First, check the bottom left tree
  bst_search_all(root->left, id, found_count);

  // Check the current node
  if (strcmp(id, root->data.id) == 0) {
    (*found_count)++;
    printf("  -> Found [%d]: %-12s | fuel=%-3d | %-7s | %s\n", *found_count,
           root->data.id, root->data.fuel,
           root->data.is_landing ? "LANDING" : "TAKEOFF",
           root->data.emergency ? "EMERGENCY" : "normal");
  }

  // Now check the right subtree
  bst_search_all(root->right, id, found_count);
}

// In-order traversal → sorted by ID
void bst_inorder(BSTNode *root) {
  if (!root)
    return;
  bst_inorder(root->left);
  printf("    %-12s | fuel=%-3d | %-7s | %s\n", root->data.id, root->data.fuel,
         root->data.is_landing ? "LANDING" : "TAKEOFF",
         root->data.emergency ? "EMERGENCY" : "normal");
  bst_inorder(root->right);
}

// Find in-order predecessor (rightmost in left subtree)
static BSTNode *max_node(BSTNode *n) {
  while (n->right)
    n = n->right;
  return n;
}

BSTNode *bst_delete(BSTNode *root, const Flight *target) {
  if (!root)
    return NULL;

  int cmp = strcmp(target->id, root->data.id);

  if (cmp < 0) {
    root->left = bst_delete(root->left, target);
  } else if (cmp > 0) {
    root->right = bst_delete(root->right, target);
  } else {                                            // The IDs matched, now check if this is the exact correct flight.
    if (flight_is_equal(&root->data, target)) {
      // This is the node to be deleted. Apply standard deletion logic.
      if (!root->left && !root->right) {
        free(root);
        return NULL;
      } else if (!root->left) {
        BSTNode *tmp = root->right;
        free(root);
        return tmp;
      } else if (!root->right) {
        BSTNode *tmp = root->left;
        free(root);
        return tmp;
      } else {
        // There are two children: in order replaced by predecessor.
        BSTNode *pred = max_node(root->left);
        root->data = pred->data;
        // Now delete the copied predecessor from the subtree on the left.
        root->left = bst_delete(root->left, &pred->data);
      }
    } else {
      // The IDs are the same but the flights are different. According to our insertion rule, we should go to the right.
      root->right = bst_delete(root->right, target);
    }
  }
  return root;
}

void bst_free(BSTNode *root) {
  if (!root)
    return;
  bst_free(root->left);
  bst_free(root->right);
  free(root);
}