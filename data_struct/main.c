#include <stdio.h>
#include "queue.h"
#include "avl_tree.h"
void seqenc_traversal(BSTNode *root)
{
    if ( !root )
    {
        return;
    }
    int i;
    queue *q = (queue *)malloc(sizeof(queue));
    queue_init(q);

    queue_push(q, root);
    while ( !queue_isempty(q) )
    {
        int len = q->size;
        for (i = 0; i < len; i++)
        {
            BSTNode *temp = q->front->val;
            printf("%d ", temp->val);
            if (temp->lchild)
            {
                queue_push(q, temp->lchild);
            }
            if (temp->rchild)
            {
                queue_push(q, temp->rchild);
            }
            queue_pop(q);
        }
    }
    
}
int main()
{
    BSTNode *root;
    root = insert_node(root, new_node(100));

    root = insert_node(root, new_node(50));

    root = insert_node(root, new_node(150));

    root = insert_node(root, new_node(75));

    root = insert_node(root, new_node(25));

    root = insert_node(root, new_node(0));
    root = insert_node(root, new_node(30));
    root = insert_node(root, new_node(35));
    root = insert_node(root, new_node(33));
    root = insert_node(root, new_node(34));
    root = del_node(root, 50);
    seqenc_traversal(root);
    printf("\n");
}