#include <stdio.h>
#define RED 0
#define BLOCK 1
typedef int type;

typedef struct RBTreeNode {
    unsigned char color;
    type key;
    struct RBTreeNode* left;
    struct RBTreeNode* right;
    struct RBTreeNode* parent;
}Node, *RBTree;

typedef struct rb_root {
    Node *node;
}RBRoot;

void rbtree_left_rotate(RBRoot *root, Node *x)
{
    Node* y = x->right;
    x->right = y->left;

    if (y->left != NULL )
    {
        y->left->parent = x;
    }

    y->parent = x->parent;
    if (x->parent == NULL)
    {
        root->node = y;
    }
    else 
    {
        if (x->parent->left == x)
        {
            x->parent->left = y;
        }
        else 
        {
            x->parent->right = y;
        }
    }

    y->left = x;
    x->parent = y;
}

void rbtree_right_rotate(RBRoot *root, Node *y)
{
    Node* x = y->left;
    y->left = x->right;

    if (x->right != NULL)
    {
        x->right->parent = y;
    }

    x->parent = y->parent;
    if (y->parent == NULL)
    {   
        root->node = x;
    }
    else 
    {
        if (y->parent->left == y)
        {
            y->parent->left = x;
        }
        else 
        {
            y->parent->right = x;
        }
    }
    x->right = y;
    y->parent = x;
}

int insert(RBRoot *root, Node* node)
{
    if (root->node == NULL)
    {
        node->color = BLOCK;
        root->node = node;
        return 1;
    }

    Node *parent = NULL;
    Node *cur = root->node;

    while (cur)
    {
        if (node->key > cur->key)
        {
            parent = cur;
            cur = cur->right;
        }
        else if (node->key < cur->key)
        {
            parent =cur;
            cur = cur->left;
        }
        else 
        {
            return -1;
        }
    }

    node->color = RED;
    if (parent->key > node->key)
    {
        parent->left = node;
        node->parent = parent;
    }
    else 
    {
        parent->right = node;
        node->parent = parent;
    }

    while (parent && parent->color == RED)
    {
        Node* grandfather = parent->parent;
        if (parent == grandfather->left)
        {
            Node *uncle = grandfather->right;
            
            if (uncle && uncle->color == RED)
            {
                parent->color = uncle->color = BLOCK;
                grandfather->color = RED;
                cur = grandfather;
                parent = cur->parent;
            }
            else 
            {
                if (cur == parent->right)
                {
                    rbtree_left_rotate(root, parent);
                    Node* temp = parent;
                    parent = cur;
                    cur = temp;
                }
                rbtree_right_rotate(root, grandfather);
                grandfather->color = RED;
                parent->color = BLOCK;
            }
        }
        else 
        {
             Node *uncle = grandfather->left;
             if (uncle && uncle->color == RED)
             {
                 parent->color = uncle->color = BLOCK;
                 grandfather->color = RED;
                 cur = grandfather;
                 parent = cur->parent;
             }
             else 
             {
                 if (cur == parent->left)
                 {
                     rbtree_right_rotate(root, parent);
                     Node* temp = parent;
                     parent = cur;
                     cur = temp;
                 }
                 rbtree_left_rotate(root, grandfather);
                 cur->color = BLOCK;
                 grandfather->color = RED;
             }
        }
    }
    root->node->color = BLOCK;
    return 1;
}

void rbtree_del(RBRoot *root, Node *node)
{
    Node *child, *parent;
    int color;

    if (node->left && node->right)
    {
        Node* replace = node->right;

        while (replace->left != NULL)
        {
            replace = replace->left;
        }

        if (node->parent)
        {
            if (node->parent->left == node)
            {
                node->parent->left = replace;
            }
            else 
            {
                node->parent->right = replace;
            }
        }
        else 
        {
            root->node = replace;
        }
        child = replace->right;
        parent = replace->parent;
        color = replace->color;

        if (parent == node)
        {
            parent = replace;
        }
        else 
        {
            if (child)
            {
                child->parent = parent;
            }
            parent->left = child;

            replace->right = parent->right;
            node->parent->right = replace;
        }

        replace->parent = node->parent;
        replace->left = node->left;
        replace->right = node->right;
        replace->color = node->color;
    }
}

