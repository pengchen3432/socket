#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

void new_rb_node()
{
    Node *node = (Node *)malloc(sizeof(Node));
    memset(node, 0x00, sizeof(Node));
}

void rbtree_left_rotate(RBRoot *root, Node *x)
{
    Node* y = x->right;
    x->right = y->left;

    if (y->left != NULL)
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

void insert_fixup(RBRoot *root, Node *node)
{
    Node *parent, *gparent;
    while ((parent = node->parent) && parent->color == RED)
    {
        gparent = parent->parent;
        if (parent == gparent->left)
        {
            Node *uncle = gparent->right;
            if (uncle->color == RED)
            {
                uncle->color = BLOCK;
                parent->color = BLOCK;
                gparent->color = RED;
                node = gparent;
                continue;
            }
            if(uncle->color == BLOCK && parent->right == node)
            {
                rbtree_left_rotate(root, node);
                Node *temp;
                temp = node;
                node = parent;
                parent = temp;
            }
            parent->color = BLOCK;
            gparent->color = RED;
            rbtree_right_rotate(root, gparent);
        }
        else 
        {
            Node *uncle = gparent->left;
            if (uncle->color == RED)
            {
                parent->color = BLOCK;
                uncle->color = BLOCK;
                gparent->color = RED;
                node = gparent;
                continue;
            }
            if (uncle->color == BLOCK && node == parent->left)
            {
                rbtree_right_rotate(root, parent);
                Node *temp;
                temp = parent;
                parent = node;
                node = temp;
            }
            parent->color = BLOCK;
            gparent->color = RED;
            rbtree_left_rotate(root, gparent);
        }
    }
    root->node->color = BLOCK;
}

void insert_node(RBRoot *root, Node *e)
{
    Node *y = NULL;
    Node *x = root->node;
    while (x != NULL)
    {
        y = x;
        if (e->key > x->key)
        {
            x = x->right;
        }
        else
        {
            x = x->left;
        }
    }

    e->parent = y;
    if (y == NULL)
    {
        root->node = e;
    }
    else 
    {
        if (e->key > y->key)
        {
            y->right = e;
        }
        else 
        {
            y->left = e;
        }
    }
    e->color = RED;
    insert_fixup(root, e);
}

void del_fixup(RBRoot *root, Node *chile, Node *parent)
{
    
}

void del_node(RBRoot *root, Node *node)
{
    Node *child, *parent;
    int color;
    if (node->left && node->right)
    {
        Node *replace;
        replace = node->right;
        while (replace->left)
        {
            replace = replace->left;
        }
        if (node->parent != NULL)
        {
            if (node == node->parent->left)
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
                child->parent = parent;
            parent->left = child;

            replace->right = node->right;
            replace->left = node->left;
            node->right->parent = replace;
        }

        replace->left = node->left;
        replace->parent = node->parent;
        replace->color = node->color;
        node->left->parent = replace;

        if (color == BLOCK)
        {
            del_fixup(root, child, parent);
        }
        free(node);
        return;
    }
    if (node->left)
    {
        child = node->left;
    }
    else 
    {
        child = node->right;
    }

    parent = node->parent;
    color = node->color;
    
    if (child)
    {
        child->parent = parent;
    }

    if (parent)
    {
        if (parent->left == node)
        {
            parent->left = child;
        }
        else 
        {
            parent->right = child;
        }
    }
    else 
    {
        root->node = child;
    }
    if (color == BLOCK)
    {
        del_fixup(root, child, parent);
    }
    free(node);
}



