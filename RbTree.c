// adapted from the location described below.

// original copyright:

/* Copyright (c) 2011 the authors listed at the following URL, and/or
the authors of referenced articles or incorporated external code:
http://en.literateprograms.org/Red-black_tree_(C)?action=history&offset=20090121005050

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Retrieved from: http://en.literateprograms.org/Red-black_tree_(C)?oldid=16016
*/

#include "RbTree.h"
#include <assert.h>

#include <stdlib.h>


typedef RbTree_Node* node;
typedef enum RbTree_Color color;


static node grandparent(node n);
static node sibling(node n);
static node uncle(node n);
static color node_color(node n);
static void verify_properties(RbTree* t);
#ifdef VERIFY_RBTREE
static void verify_property_1(node root);
static void verify_property_2(node root);
static void verify_property_4(node root);
static void verify_property_5(node root);
static void verify_property_5_helper(node n, int black_count, int* black_count_path);
#endif

static node new_node(void* key, void* value, color node_color, node left, node right);
static node lookup_node(RbTree* t, void* key);
static void rotate_left(RbTree* t, node n);
static void rotate_right(RbTree* t, node n);

static void replace_node(RbTree* t, node oldn, node newn);
static void insert_case1(RbTree* t, node n);
static void insert_case2(RbTree* t, node n);
static void insert_case3(RbTree* t, node n);
static void insert_case4(RbTree* t, node n);
static void insert_case5(RbTree* t, node n);
static node maximum_node(node root);
static void delete_case1(RbTree* t, node n);
static void delete_case2(RbTree* t, node n);
static void delete_case3(RbTree* t, node n);
static void delete_case4(RbTree* t, node n);
static void delete_case5(RbTree* t, node n);
static void delete_case6(RbTree* t, node n);

node grandparent(node n) {
    assert (n != NULL);
    assert (n->parent != NULL); /* Not the root node */
    assert (n->parent->parent != NULL); /* Not child of root */
    return n->parent->parent;
}

node sibling(node n) {
    assert (n != NULL);
    assert (n->parent != NULL); /* Root node has no sibling */
    if (n == n->parent->left)
        return n->parent->right;
    else
        return n->parent->left;
}

node uncle(node n) {
    assert (n != NULL);
    assert (n->parent != NULL); /* Root node has no uncle */
    assert (n->parent->parent != NULL); /* Children of root have no uncle */
    return sibling(n->parent);
}

color node_color(node n) {
    return n == NULL ? RbTree_Color_BLACK : n->color;
}

void verify_properties(RbTree* t) {
#ifdef VERIFY_RBTREE
    verify_property_1(t->root);
    verify_property_2(t->root);
    /* Property 3 is implicit */
    verify_property_4(t->root);
    verify_property_5(t->root);
#endif
}

#ifdef VERIFY_RBTREE
void verify_property_1(node n) {
    assert(node_color(n) == RbTree_Color_RED || node_color(n) == RbTree_Color_BLACK);
    if (n == NULL) return;
    verify_property_1(n->left);
    verify_property_1(n->right);
}

void verify_property_2(node root) {
    assert(node_color(root) == RbTree_Color_BLACK);
}

void verify_property_4(node n) {
    if (node_color(n) == RbTree_Color_RED) {
        assert (node_color(n->left)   == RbTree_Color_BLACK);
        assert (node_color(n->right)  == RbTree_Color_BLACK);
        assert (node_color(n->parent) == RbTree_Color_BLACK);
    }
    if (n == NULL) return;
    verify_property_4(n->left);
    verify_property_4(n->right);
}

void verify_property_5(node root) {
    int black_count_path = -1;
    verify_property_5_helper(root, 0, &black_count_path);
}

void verify_property_5_helper(node n, int black_count, int* path_black_count) {
    if (node_color(n) == RbTree_Color_BLACK) {
        black_count++;
    }
    if (n == NULL) {
        if (*path_black_count == -1) {
            *path_black_count = black_count;
        } else {
            assert (black_count == *path_black_count);
        }
        return;
    }
    verify_property_5_helper(n->left,  black_count, path_black_count);
    verify_property_5_helper(n->right, black_count, path_black_count);
}
#endif

RbTree* RbTree_create(compare_func comparator) {
    RbTree* t = malloc(sizeof(RbTree));
    t->root = NULL;
    t->comparator = comparator;
    verify_properties(t);
    return t;
}

node new_node(void* key, void* value, color node_color, node left, node right) {
    node result = malloc(sizeof(RbTree_Node));
    result->key = key;
    result->value = value;
    result->color = node_color;
    result->left = left;
    result->right = right;
    if (left  != NULL)  left->parent = result;
    if (right != NULL) right->parent = result;
    result->parent = NULL;
    return result;
}

node lookup_node(RbTree* t, void* key) {
    node n = t->root;
    while (n != NULL) {
        int comp_result = t->comparator(key, n->key);
        if (comp_result == 0) {
            return n;
        } else if (comp_result < 0) {
            n = n->left;
        } else {
            assert(comp_result > 0);
            n = n->right;
        }
    }
    return n;
}

void* RbTree_get(RbTree* t, void* key) {
    node n = lookup_node(t, key);
    return n == NULL ? NULL : n->value;
}

void rotate_left(RbTree* t, node n) {
    node r = n->right;
    replace_node(t, n, r);
    n->right = r->left;
    if (r->left != NULL) {
        r->left->parent = n;
    }
    r->left = n;
    n->parent = r;
}

void rotate_right(RbTree* t, node n) {
    node L = n->left;
    replace_node(t, n, L);
    n->left = L->right;
    if (L->right != NULL) {
        L->right->parent = n;
    }
    L->right = n;
    n->parent = L;
}

void replace_node(RbTree* t, node oldn, node newn) {
    if (oldn->parent == NULL) {
        t->root = newn;
    } else {
        if (oldn == oldn->parent->left)
            oldn->parent->left = newn;
        else
            oldn->parent->right = newn;
    }
    if (newn != NULL) {
        newn->parent = oldn->parent;
    }
}

void RbTree_put(RbTree* t, void* key, void* value) {
    node inserted_node = new_node(key, value, RbTree_Color_RED, NULL, NULL);
    if (t->root == NULL) {
        t->root = inserted_node;
    } else {
        node n = t->root;
        while (1) {
            int comp_result = t->comparator(key, n->key);
            if (comp_result == 0) {
                n->value = value;
                /* inserted_node isn't going to be used, don't leak it */
                free (inserted_node);
                return;
            } else if (comp_result < 0) {
                if (n->left == NULL) {
                    n->left = inserted_node;
                    break;
                } else {
                    n = n->left;
                }
            } else {
                assert (comp_result > 0);
                if (n->right == NULL) {
                    n->right = inserted_node;
                    break;
                } else {
                    n = n->right;
                }
            }
        }
        inserted_node->parent = n;
    }
    insert_case1(t, inserted_node);
    verify_properties(t);
}

void insert_case1(RbTree* t, node n) {
    if (n->parent == NULL)
        n->color = RbTree_Color_BLACK;
    else
        insert_case2(t, n);
}

void insert_case2(RbTree* t, node n) {
    if (node_color(n->parent) == RbTree_Color_BLACK)
        return; /* Tree is still valid */
    else
        insert_case3(t, n);
}

void insert_case3(RbTree* t, node n) {
    if (node_color(uncle(n)) == RbTree_Color_RED) {
        n->parent->color = RbTree_Color_BLACK;
        uncle(n)->color = RbTree_Color_BLACK;
        grandparent(n)->color = RbTree_Color_RED;
        insert_case1(t, grandparent(n));
    } else {
        insert_case4(t, n);
    }
}

void insert_case4(RbTree* t, node n) {
    if (n == n->parent->right && n->parent == grandparent(n)->left) {
        rotate_left(t, n->parent);
        n = n->left;
    } else if (n == n->parent->left && n->parent == grandparent(n)->right) {
        rotate_right(t, n->parent);
        n = n->right;
    }
    insert_case5(t, n);
}

void insert_case5(RbTree* t, node n) {
    n->parent->color = RbTree_Color_BLACK;
    grandparent(n)->color = RbTree_Color_RED;
    if (n == n->parent->left && n->parent == grandparent(n)->left) {
        rotate_right(t, grandparent(n));
    } else {
        assert (n == n->parent->right && n->parent == grandparent(n)->right);
        rotate_left(t, grandparent(n));
    }
}

void RbTree_remove(RbTree* t, void* key) {
    node child;
    node n = lookup_node(t, key);
    if (n == NULL) return;  /* Key not found, do nothing */
    if (n->left != NULL && n->right != NULL) {
        /* Copy key/value from predecessor and then delete it instead */
        node pred = maximum_node(n->left);
        n->key   = pred->key;
        n->value = pred->value;
        n = pred;
    }

    assert(n->left == NULL || n->right == NULL);
    child = n->right == NULL ? n->left  : n->right;
    if (node_color(n) == RbTree_Color_BLACK) {
        n->color = node_color(child);
        delete_case1(t, n);
    }
    replace_node(t, n, child);
    if (n->parent == NULL && child != NULL)
        child->color = RbTree_Color_BLACK;
    free(n);

    verify_properties(t);
}

static node maximum_node(node n) {
    assert (n != NULL);
    while (n->right != NULL) {
        n = n->right;
    }
    return n;
}

void delete_case1(RbTree* t, node n) {
    if (n->parent == NULL)
        return;
    else
        delete_case2(t, n);
}

void delete_case2(RbTree* t, node n) {
    if (node_color(sibling(n)) == RbTree_Color_RED) {
        n->parent->color = RbTree_Color_RED;
        sibling(n)->color = RbTree_Color_BLACK;
        if (n == n->parent->left)
            rotate_left(t, n->parent);
        else
            rotate_right(t, n->parent);
    }
    delete_case3(t, n);
}

void delete_case3(RbTree* t, node n) {
    if (node_color(n->parent) == RbTree_Color_BLACK &&
        node_color(sibling(n)) == RbTree_Color_BLACK &&
        node_color(sibling(n)->left) == RbTree_Color_BLACK &&
        node_color(sibling(n)->right) == RbTree_Color_BLACK)
    {
        sibling(n)->color = RbTree_Color_RED;
        delete_case1(t, n->parent);
    }
    else
        delete_case4(t, n);
}

void delete_case4(RbTree* t, node n) {
    if (node_color(n->parent) == RbTree_Color_RED &&
        node_color(sibling(n)) == RbTree_Color_BLACK &&
        node_color(sibling(n)->left) == RbTree_Color_BLACK &&
        node_color(sibling(n)->right) == RbTree_Color_BLACK)
    {
        sibling(n)->color = RbTree_Color_RED;
        n->parent->color = RbTree_Color_BLACK;
    }
    else
        delete_case5(t, n);
}

void delete_case5(RbTree* t, node n) {
    if (n == n->parent->left &&
        node_color(sibling(n)) == RbTree_Color_BLACK &&
        node_color(sibling(n)->left) == RbTree_Color_RED &&
        node_color(sibling(n)->right) == RbTree_Color_BLACK)
    {
        sibling(n)->color = RbTree_Color_RED;
        sibling(n)->left->color = RbTree_Color_BLACK;
        rotate_right(t, sibling(n));
    }
    else if (n == n->parent->right &&
             node_color(sibling(n)) == RbTree_Color_BLACK &&
             node_color(sibling(n)->right) == RbTree_Color_RED &&
             node_color(sibling(n)->left) == RbTree_Color_BLACK)
    {
        sibling(n)->color = RbTree_Color_RED;
        sibling(n)->right->color = RbTree_Color_BLACK;
        rotate_left(t, sibling(n));
    }
    delete_case6(t, n);
}

void delete_case6(RbTree* t, node n) {
    sibling(n)->color = node_color(n->parent);
    n->parent->color = RbTree_Color_BLACK;
    if (n == n->parent->left) {
        assert (node_color(sibling(n)->right) == RbTree_Color_RED);
        sibling(n)->right->color = RbTree_Color_BLACK;
        rotate_left(t, n->parent);
    }
    else
    {
        assert (node_color(sibling(n)->left) == RbTree_Color_RED);
        sibling(n)->left->color = RbTree_Color_BLACK;
        rotate_right(t, n->parent);
    }
}

static char postorder_traversal(RbTree_Node* node, visitor_func visitor, void * data)
{
    if (node == NULL)
        return 1;
    if (!postorder_traversal(node->left, visitor, data))
        return 0;
    if (!postorder_traversal(node->right, visitor, data))
        return 0;
    if (!visitor(node, data))
        return 0;
    return 1;
}

static char internal_delete_visitor(RbTree_Node * node, void * data)
{
    visitor_func delete_visitor = (visitor_func)data;
    if (delete_visitor != NULL)
        delete_visitor(node, NULL);
    free(node);
    return 1;
}

void RbTree_delete(RbTree* t, visitor_func delete_visitor)
{
    postorder_traversal(t->root, internal_delete_visitor, delete_visitor);
    free(t);
}

static RbTree_Node * next_node(RbTree_Node * node)
{
    if (node->right != NULL) {
        node = node->right;
        while (node->left != NULL)
            node = node->left;
        return node;
    } else {
        while (1) {
            RbTree_Node * previous_node = node;
            node = node->parent;
            if (node == NULL)
                return NULL;
            if (previous_node == node->left)
                return node;
        }
    }
}

void RbTree_traverse_starting_at(RbTree * t, void * min_key, visitor_func visitor, void * data)
{
    RbTree_Node * node = t->root;
    if (node == NULL)
        return;
    while (1) {
        int comp_result = t->comparator(min_key, node->key);
        if (comp_result == 0) {
            break;
        } else if (comp_result < 0) {
            if (node->left != NULL) {
                node = node->left;
                continue;
            }
            break;
        } else {
            if (node->right != NULL) {
                node = node->right;
                continue;
            }
            node = next_node(node);
            break;
        }
    }
    while (node != NULL) {
        if (!visitor(node, data))
            return;
        node = next_node(node);
    }
}

