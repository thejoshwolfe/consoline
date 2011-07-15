#ifndef _RB_TREE_H_
#define _RB_TREE_H_

enum RbTree_Color { RbTree_Color_RED, RbTree_Color_BLACK };

typedef struct RbTree_Node_ {
    void* key;
    void* value;
    struct RbTree_Node_* left;
    struct RbTree_Node_* right;
    struct RbTree_Node_* parent;
    enum RbTree_Color color;
} RbTree_Node;

typedef int (*compare_func)(void* left, void* right);

typedef struct {
    RbTree_Node* root;
    compare_func comparator;
} RbTree;

typedef char (*visitor_func)(RbTree_Node * node, void * data);

RbTree* RbTree_create(compare_func comparator);
// the visitor, if non NULL, needs to free the members of the node, but not the node itself.
// the visitor will get NULL for the data parameter and the return value is ignored.
void RbTree_delete(RbTree* t, visitor_func delete_visitor);

void* RbTree_get(RbTree* t, void* key);
void RbTree_put(RbTree* t, void* key, void* value);
void RbTree_remove(RbTree* t, void* key);

// Calls the visitor function for each node starting at or after the key specified (inorder traversal).
// Visitor is called with the data provided, whatever it is.
// Visitor should return non-zero to continue and 0 to terminate traversal.
void RbTree_traverse_starting_at(RbTree * t, void * min_key, visitor_func visitor, void * data);

#endif

