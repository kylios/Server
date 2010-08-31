
#include    <stdio.h>
#include    <stdlib.h>
#include    "debug.h"
#include    "avl.h"

/* helper functions for tree operations */
static bool insert_helper (struct avl_node** node, struct avl_node* parent, 
        void* data, bool* taller, avl_tree_compare_func* func);

static bfactor adjust_balance (struct avl_node* node, bfactor balance);
static void rotate_left (struct avl_node** node);
static void rotate_right (struct avl_node** node);
static void balance_left (struct avl_node** node);
static void balance_right (struct avl_node** node);

static struct avl_node* find_helper (struct avl_node*, const void*, 
        avl_tree_compare_func*);

static void dump_helper (struct avl_node* node, unsigned, avl_tree_dump_func*);

static void dump_node (struct avl_node* node, unsigned level);

void 
avl_init (struct avl_tree* tree, avl_tree_compare_func* func)
{
    if (!tree || !func)   return;
    
    tree->root = NULL;
    tree->comparator = func;
};

bool 
avl_insert (struct avl_tree* tree, void* data)
{
    if (!data)  return false;
    bool taller = false;
    return insert_helper (&(tree->root), NULL, data, &taller, tree->comparator);
};

void* 
avl_delete (struct avl_tree* tree, void* data)
{
    if (!data || !tree)
        return NULL;

    struct avl_node* del = avl_find (tree, data);

    /* Case 1: Node is a leaf */
    if (del->left == NULL && del->right == NULL)
    {
        if (!del->parent)   tree->root = NULL;
        else if (del->parent->right == del) del->parent->right = NULL;
        else                                del->parent->left = NULL;
    }

    /* Case 2: Node has one child */
    else if (del->left)
    {
        if (del->parent == NULL)    tree->root = del->left;
        else if (del->parent->right == del) del->parent->right = del->left;
        else                                del->parent->left = del->left;
    }
    else if (del->right)
    {
        if (del->parent == NULL)    tree->root = del->right;
        else if (del->parent->right == del) del->parent->right = del->right;
        else                                del->parent->left = del->left;
    }

    /* Case 3: Node has both children */
    else
    {
        ASSERT (del->left);
        ASSERT (del->right);

        struct avl_node* right = del->right;
        /* Find the leftmost child */
        struct avl_node* n = right;
        while (n->left)
            n = n->left;
        ASSERT (n->left == NULL);
        if (n->right)
        {
            n->parent->left = n->right;
            n->right->parent = n->parent;
        }
        n->right = del->right;
        n->left = del->left;
        n->parent = del->parent;
        if (del->parent == NULL)    tree->root = n;
        else if (del->parent->right == del) del->parent->right = n;
        else                                del->parent->left = n;
    }

    void* ret = del->data;
    free (del);
    return ret;
};

void*
avl_find (struct avl_tree* tree, const void* elem)
{
    ASSERT (tree != NULL);

    if (elem == NULL)   return NULL;

    struct avl_node* node = find_helper (tree->root, elem, tree->comparator);
    if (node == NULL)
        return NULL;
    return node->data;
};

void 
avl_dump (struct avl_tree* tree, avl_tree_dump_func* func)
{
    dump_helper (tree->root, 0, func);   
};


/*******
  Iterator Functions
  *********/
struct avl_iterator* 
avl_get_iterator (struct avl_tree* tree)
{
    if (tree == NULL || tree->root == NULL) return NULL;
    struct avl_iterator* it = malloc (sizeof (struct avl_iterator));
    if (it == NULL)
        return NULL;
    it->node = tree->root;
    it->tree = tree;
    /* Go to the smallest element in the tree... The left-most. */
    while (it->node->left)
        it->node = it->node->left;
    return it;
};

struct avl_iterator* 
avl_get_reverse_iterator (struct avl_tree* tree)
{
    if (tree == NULL)   return NULL;
    struct avl_iterator* it = malloc (sizeof (struct avl_iterator));
    if (it == NULL) return NULL;
    it->node = tree->root;
    it->tree = tree;
    /* Go to the largest element in the tree... The right-most. */
    while (it->node->right)
        it->node = it->node->right;
    return it;
};

void* 
avl_get (struct avl_iterator* it)
{
    if (it == NULL) return NULL;
    return it->node->data;
};

void 
avl_itr_remove (struct avl_iterator* it)
{
    if (it == NULL) return;
    struct avl_node* node = it->node;
    avl_next (it);
    avl_delete (it->tree, node->data);
};

void* 
avl_next (struct avl_iterator* it)
{
    if (it == NULL) return NULL;
    if (it->node->right)
    {
        it->node = it->node->right;
        while (it->node->left)
            it->node = it->node->left;
    }
    else if (it->node->parent)
    {
        /* We need to go up the tree. */
        if (it->node == it->node->parent->left)
            it->node = it->node->parent;
        else
        {
            while (it->node == it->node->parent->right)
            {
                it->node = it->node->parent;
                if (!it->node->parent)
                    return NULL;
            }
            it->node = it->node->parent;
        }
    }
    else
    {
        return NULL;
    }
    return it->node->data;
};

void* 
avl_prev (struct avl_iterator* it)
{
    if (it == NULL) return NULL;
    if (it->node->left)
    {
        it->node = it->node->left;
        while (it->node->right)
            it->node = it->node->right;
    }
    else if (it->node->parent)
    {
        if (it->node == it->node->parent->right)
            it->node = it->node->parent;
        else
        {
            while (it->node == it->node->parent->left)
            {
                it->node = it->node->parent;
                if (!it->node->parent)
                    return NULL;
            }
            it->node = it->node->parent;
        }
    }
    else
    {
        return NULL;
    }
    return it->node->data;
};



/* ===  HELPER FUNCTIONS === */
static bfactor 
adjust_balance (struct avl_node* node, bfactor balance)
{
    ASSERT (node != NULL);
    ASSERT (balance >= -1);
    ASSERT (balance <= 1);

    node->balance += balance;
    return node->balance;
};

static void 
rotate_left (struct avl_node** node)
{
    ASSERT (node != NULL);
    ASSERT ((*node)->right != NULL);

    struct avl_node* temp = *node;
    *node = (*node)->right;
    temp->right = (*node)->left;
    (*node)->left = temp;
};

static void
rotate_right (struct avl_node** node)
{
    ASSERT (node != NULL);
    ASSERT ((*node)->left != NULL);

    struct avl_node* temp = *node;
    *node = (*node)->left;
    temp->left = (*node)->right;
    (*node)->right = temp;
};

static void
balance_right (struct avl_node** node)
{
    if (node == NULL || *node == NULL || (*node)->right == NULL)    return;
    struct avl_node* right_sub_node = (*node)->right;
    struct avl_node* left_sub_right_sub_node;
    switch (right_sub_node->balance)
    {
    case 1:
        (*node)->balance = 0;
        right_sub_node->balance = 0;
        rotate_left (node);
        break;
    case -1:
        left_sub_right_sub_node = right_sub_node->left;
        switch (left_sub_right_sub_node->balance)
        {
            case -1:
                (*node)->balance = 0;
                break;
            case 1:
                (*node)->balance = -1;
                break;
            default:
                (*node)->balance = 0;
                break;
        }
        left_sub_right_sub_node->balance = 0;
        right_sub_node->balance = 0;
        rotate_right (&(*node)->right);
        rotate_left (node);
        break;
    }
};

static void
balance_left (struct avl_node** node)
{
    if (node == NULL || *node == NULL || (*node)->left == NULL)     return;
    struct avl_node* left_sub_node = (*node)->left;
    struct avl_node* right_sub_left_sub_node;
    switch (left_sub_node->balance)
    {
    case -1:
        (*node)->balance = 0;
        left_sub_node->balance = 0;
        rotate_right (node);
        break;
    case 1:
        right_sub_left_sub_node = left_sub_node->right;
        switch (right_sub_left_sub_node->balance)
        {
            case 1: 
                (*node)->balance = 0;
                break;
            case -1:
                (*node)->balance = 1;
                break;
            default:
                (*node)->balance = 0;
                break;
        }
        right_sub_left_sub_node->balance = 0;
        left_sub_node->balance = 0;
        rotate_left (&(*node)->left);
        rotate_right (node);
        break;
    }
};

bool 
insert_helper (struct avl_node** node, struct avl_node* parent, void* data, 
        bool* taller, avl_tree_compare_func* func)
{
    ASSERT (node != NULL);
    ASSERT (data != NULL);

    struct avl_node* new_node = NULL;
    if (*node == NULL)  
    {
        new_node = malloc (sizeof (struct avl_node));
        if (new_node == NULL)   return false;
        new_node->left      = NULL;
        new_node->right     = NULL;
        new_node->parent    = parent;
        new_node->data      = data;
        new_node->balance   = 0;
    
        (*node) = new_node;

        return true;
    }   
    else    
    {
        new_node = (*node);
        ASSERT (new_node->data != NULL);

        int compare_result = func (data, (*node)->data, NULL);
            
        if (compare_result < 0) 
        {
            if (!insert_helper (&(*node)->left, *node, data, taller, func))
            {
                return false;
            }
            if (taller)
            {
                adjust_balance (new_node, -1);
                switch ((*node)->balance)
                {
                case 0:
                    *taller = false;
                    break;
                case -1:
                    *taller = true;
                    break;
                case -2:
                    balance_left (&new_node);
                    *taller = false;
                    break;
                }
            }
        }   
        else if (compare_result < 0) 
        {
            if (!insert_helper (&(*node)->right, *node, data, taller, func))
            {
                return false;
            }
            if (taller)
            {
                adjust_balance (new_node, 1);
                switch ((*node)->balance)
                {
                case 0:
                    *taller = false;
                    break;
                case 1:
                    *taller = true;
                    break;
                case 2:
                    balance_right (&new_node);
                    *taller = false;
                    break;
                }
            }
        }
    }
    return true;
};

struct avl_node*
find_helper (struct avl_node* node, const void* elem, 
        avl_tree_compare_func* func)
{
    ASSERT (elem != NULL);

    if (node == NULL)   return NULL;

    ASSERT (node->data != NULL);
    int cmp = func (elem, node->data, NULL);

    if (cmp > 0)
    {
        return find_helper (node->right, elem, func);
    }
    else if (cmp < 0) 
    {
        return find_helper (node->left, elem, func);
    }
    else
    {
        return node;
    }
};

void 
dump_helper (struct avl_node* node, unsigned level, 
        avl_tree_dump_func* func)
{
    if (node)   
    {
        dump_helper (node->left, level + 1, func);

        unsigned i = 0;
        for (i = 0; i < level; i++)    
        {
            printf ("   ");
        } 
        if (node->data) 
        {
            func (node->data);
        }   
        else    
        {
            printf ("NULL");
        }
        printf ("\n");
        dump_helper (node->right, level + 1, func);
    }
};

void
dump_node (struct avl_node* node, unsigned level)
{
    
};
