#ifndef avl_H
#define avl_H  

#include    "type.h"



typedef int bfactor;

struct avl_node;

struct avl_node
{
    struct avl_node* left;
    struct avl_node* right;
    struct avl_node* parent;    /* Parent is required for iterators. */
    
    /* Keep track of the balance of this subtree */
    bfactor balance;

    /* Pointer to the actual data object */
    void* data;
};

/* The comparator function for the red-black tree.
 * When we perform insertions or lookups on the index, the function that
 * defines the operation for comparisons must comply to this function typedef.
 * */
typedef int avl_tree_compare_func (const void*, const void*, 
        const void* AUX);
typedef void avl_tree_dump_func (const void*);


struct avl_tree
{
    /* the root node */
    struct avl_node* root;
    
    /* the comparator function */
    avl_tree_compare_func* comparator;
};





/* Initializes a new red black tree.
 * TREE must point to a valid block of memory
 * FUNC must be a comparator function used for comparing elements in the index
 * */
void avl_init (struct avl_tree* tree, avl_tree_compare_func* func);

/* Inserts the data pointed to by DATA into TREE
 * TREE must be a valid initialized tree
 * DATA must be a valid pointer
 * */
bool avl_insert (struct avl_tree* tree, void* data);

/* Deletes the data pointed to by DATA from TREE
 * TREE must be a valid initialized tree
 * DATA must be a valid pointer
 * */
void* avl_delete (struct avl_tree* tree, void* data);

/* Looks up a value from the tree.  If it finds a match, returns it, 
 * otherwise, returns NULL.
 * */
void* avl_find (struct avl_tree* tree, const void* elem);

/* Prints out the contents of TREE using FUNC to print the data 
 * (if supplied)
 * TREE must be a valid initialized tree
 * FUNC (optional) must be of type rb_tree_dump_func
 * */
void avl_dump (struct avl_tree* tree, avl_tree_dump_func* func);


/* avl iterator functions.  */
struct avl_iterator
{
    struct avl_node* node;
    struct avl_tree* tree;
};

struct avl_iterator* avl_get_iterator (struct avl_tree*);
struct avl_iterator* avl_get_reverse_iterator (struct avl_tree*);
void* avl_get (struct avl_iterator*);
void avl_itr_remove (struct avl_iterator*);
void* avl_next (struct avl_iterator*);
void* avl_prev (struct avl_iterator*);



#endif

