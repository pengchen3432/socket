#ifndef _AVL_TREE
#define _AVL_TREE
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include "queue.h"
#define LH 1
#define RH -1
#define EH 0

typedef struct BSTNode{
    int val;            
    int bf;          // 平衡银子
    int hight;       // 高度
    struct BSTNode *lchild, *rchild;
}BSTNode;

int max(int a, int b)
{
    if (a > b)
    {
        return a;
    }
    return b;
}

int get_hight(BSTNode * node)
{
    if (!node)
    {
        return 0;
    }
    return node->hight;
}

BSTNode *new_node(int e)
{
    BSTNode *node = (BSTNode *)malloc(sizeof(BSTNode));
    memset(node, 0x00, sizeof(BSTNode));
    node->hight = 1;
    node->val = e;
    node->bf = 0;
    return node;
}

BSTNode *right_rotate(BSTNode *node)
{
    BSTNode *lc = node->lchild;
    node->lchild = lc->rchild;
    lc->rchild = node;

    node->hight = max(get_hight(node->lchild), get_hight(node->rchild)) + 1;
    lc->hight = max(get_hight(lc->lchild), get_hight(lc->rchild)) + 1;
    return lc;
}

BSTNode *left_rotate(BSTNode *node)
{
    BSTNode *rc = node->rchild;
    node->rchild = rc->lchild;
    rc->lchild = node;

    node->hight = max(get_hight(node->lchild), get_hight(node->rchild)) + 1;
    rc->hight = max(get_hight(rc->lchild), get_hight(rc->rchild)) + 1;
    return rc;
}

BSTNode *insert_node(BSTNode *root, BSTNode *node) 
{
    if (root == NULL) 
    {
        root = node;
        return root;
    }

    if (node->val > root->val)
    {
        root->rchild = insert_node(root->rchild, node);
    }
    else if (node->val < root->val)
    {
        root->lchild = insert_node(root->lchild, node);
    }
    
    root->hight = max(get_hight(root->lchild), get_hight(root->rchild)) + 1;
    root->bf = get_hight(root->lchild) - get_hight(root->rchild);

    if (root->bf == 2 && root->lchild->bf == 1) // ll型
    {
        return right_rotate(root);
    }
    if (root->bf == 2 && root->lchild->bf == -1)  // lr型
    {
        root->lchild = left_rotate(root->lchild);
        return right_rotate(root);
    }
    if (root->bf == -2 && root->rchild->bf == -1) // rr型 
    {
        return left_rotate(root);
    }
    if (root->bf == -2 && root->rchild->bf == 1) // rl型
    {
        root->rchild = right_rotate(root->rchild);
        return left_rotate(root);
    }
    return root;

}

BSTNode *del_node(BSTNode *root, int e)
{
    if ( !root ) 
    {
        return NULL;
    }
    if (e > root->val) 
    {
        root->rchild = del_node(root->rchild, e);
    }
    else if (e < root->val)
    {
        root->lchild = del_node(root->lchild, e);
    }
    else 
    {
        // 如果没有左孩子和右孩子,直接删除元素
        if (root->lchild == NULL && root->rchild == NULL)
        {
            free(root);
            root= NULL;
        }
        else if (root->lchild) // 如果有左孩子，让左孩子的最右节点来替补这个位置（因为这个节点肯定是比他小的最大元素）
        {
            BSTNode *temp = root->lchild;
            while (temp->rchild)
            {
                temp = temp->rchild;
            }
            root->val = temp->val;
            root->lchild = del_node(root->lchild, temp->val);
        }
        else // 如果有右孩子，让右孩子的最左节点来替补这个位置（这个节点肯定是比他大的最小节点），这个节点都来替补了，肯定要把下面的他干掉
        {
            BSTNode *temp = root->rchild;
            while (temp->lchild)
            {
                temp = temp->lchild;
            }
            root->val = temp->val;
            root->rchild = del_node(root->rchild, temp->val);
        }
    }
    if ( !root )
    {
        return NULL;
    }

    root->hight = max(get_hight(root->lchild), get_hight(root->rchild)) + 1;
    root->bf = get_hight(root->lchild) - get_hight(root->rchild);

    if (root->bf == 2 && root->lchild->bf == 1) // ll型
    {
        return right_rotate(root);
    }
    if (root->bf == 2 && root->lchild->bf == -1)  // lr型
    {
        root->lchild = left_rotate(root->lchild);
        return right_rotate(root);
    }
    if (root->bf == -2 && root->rchild->bf == -1) // rr型 
    {
        return left_rotate(root);
    }
    if (root->bf == -2 && root->rchild->bf == 1) // rl型
    {
        root->rchild = right_rotate(root->rchild);
        return left_rotate(root);
    }
    
    return root;
}

#endif