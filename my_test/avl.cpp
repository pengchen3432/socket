#include <iostream>
using namespace std;
class BSTNode{
public:
    BSTNode(int v = 0, int b = 0, int h = 1, BSTNode *l = nullptr, BSTNode *r = nullptr) : val(v), bf(b), heigh(h), left(l), right(r) {};
    int val;
    int bf;
    int heigh;
    BSTNode *left;
    BSTNode *right;
    static int max_heigh(BSTNode* l, BSTNode* r);
    static int get_heigh(BSTNode * node);
};
int BSTNode::max_heigh(BSTNode* l, BSTNode* r) {
    return max(l->heigh, r->heigh);
}

int BSTNode::get_heigh(BSTNode *node)
{
    if (!node) 
    {
        return 0;
    }
    return node->heigh;
}

BSTNode*left_rotate(BSTNode *root)
{
    BSTNode *lc = root->left;
    root->left = lc->right;
    lc->right = root;

    root->heigh = BSTNode::max_heigh(root->left, root->right);
    lc->heigh = BSTNode::max_heigh(lc->left, lc->right);
    return lc;
}

BSTNode *right_rotate(BSTNode *root) 
{
    BSTNode *rc = root->right;
    root->right = rc->left;
    rc->left = root;

    root->heigh = BSTNode::max_heigh(root->left, root->right);
    rc->heigh = BSTNode::max_heigh(rc->left, rc->right);
    return rc;
}

BSTNode *insert_node(BSTNode *root, BSTNode *node) 
{
    if ( !root ) 
    {
        root = node;
        return root;
    }
    if (node->val > root->val) 
    {
        root->right = insert_node(root->right, node);
    }
    else if (node->val < root->val) 
    {
        root->left = insert_node(root->left, node);
    }
    int left = BSTNode::get_heigh(root->left);
    int right = BSTNode::get_heigh(root->right);

    

}
int main()
{
    
}