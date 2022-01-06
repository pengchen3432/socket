#include <iostream>
#include <queue>
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
    return max(get_heigh(l), get_heigh(r));
}

int BSTNode::get_heigh(BSTNode *node)
{
    if (!node) 
    {
        return 0;
    }
    return node->heigh;
}

BSTNode*right_rotate(BSTNode *root)
{
    BSTNode *lc = root->left;
    root->left = lc->right;
    lc->right = root;

    root->heigh = BSTNode::max_heigh(root->left, root->right) + 1;
    lc->heigh = BSTNode::max_heigh(lc->left, lc->right) + 1;
    return lc;
}

BSTNode *left_rotate(BSTNode *root) 
{
    BSTNode *rc = root->right;
    root->right = rc->left;
    rc->left = root;

    root->heigh = BSTNode::max_heigh(root->left, root->right) + 1;
    rc->heigh = BSTNode::max_heigh(rc->left, rc->right) + 1;
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

    root->heigh = BSTNode::max_heigh(root->left, root->right) + 1;
    root->bf = BSTNode::get_heigh(root->left) - BSTNode::get_heigh(root->right);

    if (root->bf == 2 && root->left->bf == 1) 
    {
        root = right_rotate(root);
    }

    if (root->bf == 2 && root->left->bf == -1)
    {
        root->left = left_rotate(root->left);
        root = left_rotate(root);
    }

    if (root->bf == -2 && root->right->bf == -1)
    {
        root = left_rotate(root);
    }

    if (root->bf == -2 && root->right->bf == 1)
    {
        root->right = right_rotate(root->right);
        root = left_rotate(root);
    }
    return root;
}

BSTNode *del_node(BSTNode *root, int val)
{
    if (!root)
    {
        return NULL;
    }
    
    if (val > root->val)
    {
        root->right = del_node(root->right, val);
    }
    else if (val < root->val)
    {
        root->left = del_node(root->left, val);
    }
    else 
    {
        if (root->left == NULL && root->right == NULL)
        {
            delete root;
            root = NULL;
            return root;
        }
        else if (root->left)
        {
            BSTNode *temp = root->left;
            while (temp->right)
            {
                temp = temp->right;
            }
            root->val = temp->val;
            root->left = del_node(root->left, root->val);
        }
        else 
        {
            BSTNode *temp = root->right;
            while (temp->left)
            {
                temp = temp->left;
            }
            root->val = temp->val;
            root->right = del_node(root->right, root->val);
        }
    }
    
    root->heigh = BSTNode::max_heigh(root->left, root->right) + 1;
    root->bf = BSTNode::get_heigh(root->left) - BSTNode::get_heigh(root->right);


    if (root->bf == 2 && root->left->bf == 1) 
    {
        root = right_rotate(root);
    }

    if (root->bf == 2 && root->left->bf == -1)
    {
        root->left = left_rotate(root->left);
        root = left_rotate(root);
    }

    if (root->bf == -2 && root->right->bf == -1)
    {
        root = left_rotate(root);
    }

    if (root->bf == -2 && root->right->bf == 1)
    {
        root->right = right_rotate(root->right);
        root = left_rotate(root);
    }

    return root;
}

void sequence_traversal(BSTNode *root)
{
    queue<BSTNode *>q;
    q.push(root);
    while (!q.empty())
    {
        int len = q.size();
        for (int i = 0; i < len; i++)
        {
            BSTNode* temp = q.front();
            q.pop();
            cout << temp->val << " ";
            if (temp->left)
            {
                q.push(temp->left);
            }
            if (temp->right)
            {
                q.push(temp->right);
            }
        }
    }
    cout << endl;
}
int get_tree_heigh(BSTNode *root)
{
    if (!root)
    {
        return 0;
    }
    return max(get_tree_heigh(root->left), get_tree_heigh(root->right)) + 1;
}
int main()
{
    BSTNode* root = NULL;
    root = insert_node(root, new BSTNode(5));
    root = insert_node(root, new BSTNode(4));
    root = insert_node(root, new BSTNode(3));
    root = insert_node(root, new BSTNode(6));
    root = insert_node(root, new BSTNode(7));
    root = del_node(root, 6);
    root = del_node(root, 4);
    root = insert_node(root, new BSTNode(8));
    sequence_traversal(root);
    cout <<get_tree_heigh(root) <<endl;
}