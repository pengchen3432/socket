#ifndef _QUEUE_H
#define _QUEUE_H
#include <stdio.h>
#include <stdlib.h>
#include "avl_tree.h"
// 因为层序遍历更好观察结果,写个队列
typedef BSTNode* type;
typedef struct Node{
    type val;
    struct Node *next;
}Node;

typedef struct queue{
    int size;
    Node *front;
    Node *rear;
}queue;

Node *new_queue_node(type e)
{
    Node *node = (Node *)malloc(sizeof(Node));
    node->val = e;
    node->next = NULL;
}

void queue_init(queue *q)
{
    q->size = 0;
    q->front = q->rear = NULL;
}

int queue_isempty(queue *q)
{
    return q->size == 0;
}

void queue_push(queue *q, type e)
{
    Node *node = new_queue_node(e);
    if (q->front == NULL)
    {
        q->front = q->rear = node;
    }
    else 
    {
        q->rear->next = node;
        q->rear = node;
    }
    q->size++;
}

void queue_pop(queue *q)
{
    if (queue_isempty(q))
    {
        return ;
    }
    Node *temp = q->front;
    q->front = q->front->next;
    free(temp);
    temp = NULL;
    q->size--;
}
#endif