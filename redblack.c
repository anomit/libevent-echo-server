/*
 * Implementation according to tutorial on Red Black trees by Julienne Walker
 * at http://www.eternallyconfuzzled.com/tuts/datastructures/jsw_tut_rbtree.aspx
 */
#include <stdio.h>
#include <stdlib.h>
#include <event.h>
#include "redblack.h"

node_t *insert_r(node_t *root, int data, read_callback rcb);
node_t *delete_r(node_t *root, int data, int *done);
node_t *remove_balance(node_t *root, int direction, int *done);
node_t *getnode(int data);
node_t *single_rotate(node_t *root, int direction);
node_t *double_rotate(node_t *root, int direction);

node_t *getnode(int data)
{
    node_t *temp = malloc(sizeof(node_t));
    if (temp)
    {
        temp->data = data;
        temp->red = 1;
        temp->link[0] = NULL;
        temp->link[1] = NULL;
    }
    return temp;
}

int is_red(node_t *n)
{
    return n != NULL && n->red == 1;
}

node_t *insert(node_t *root, int data, read_callback rcb)
{
    root = insert_r(root, data, rcb);
    root->red = 0;
    return root;
}

node_t *insert_r(node_t *root, int data, read_callback rcb)
{
    if (!root)
    {
        root = getnode(data);
        event_set(&root->ev, data, EV_READ|EV_PERSIST, rcb, root);
        event_add(&root->ev, NULL);
    }
    else if (data != root->data)
    {
        int direction = root->data < data;
        root->link[direction] = insert_r(root->link[direction], data, rcb);

        /* Check child and sibling */
        if (is_red(root->link[direction]))
        {
            if (is_red(root->link[!direction]))
            {
                /* Case 1: flip colors */
                root->red = 1;
                root->link[0]->red = 0;
                root->link[1]->red = 0;
            }
            else
            {
                /* Cases 2 and 3 */
                if (is_red(root->link[direction]->link[direction]))
                    root = single_rotate(root, !direction);
                else if (is_red(root->link[direction]->link[!direction]))
                    root = double_rotate(root, !direction);
            }
        }
    }
    return root;
}

node_t *delete(node_t *root, int data)
{
    int done = 0;

    root = delete_r(root, data, &done);
    if (root)
        root->red = 0;
    return root;
}

node_t *delete_r(node_t *root, int data, int *done)
{
    if (!root)
        *done = 1;
    else
    {
        int direction;

        if (root->data == data)
        {
            /* Has only one child */
            if (root->link[0] == NULL || root->link[1] == NULL)
            {
                // saves the link which is non-empty or non-null
                node_t *saved = root->link[root->link[0] == NULL];

                /* Case 0: If node to be removed is red */
                if (is_red(root))
                    *done = 1;
                else if (is_red(saved))
                {
                    /* Make child black if it's red */
                    saved->red = 0;
                    *done = 1;
                }
                free(root);
                return saved;
            }
            else
            {
                /* Find inorder predecessor */
                node_t *pred = root->link[0];
                while (pred->link[1])
                    pred = pred->link[1];
                root->data = pred->data;
                data = pred->data;
            }
        }

        direction = root->data < data;
        root->link[direction] = delete_r(root->link[direction], data, done);

        if (!*done)
            root = remove_balance(root, direction, done);
    }
    
    return root;
}

node_t *remove_balance(node_t *root, int direction, int *done)
{
    node_t *p = root;
    node_t *sibling = root->link[!direction];

    /* Case reduction, remove red sibling */
    if (is_red(sibling))
    {
        root = single_rotate(root, direction);
        sibling = p->link[!direction];
    }
    if(sibling)
    {
        /* If both children are black */
        if (!is_red(sibling->link[0]) && !(is_red(sibling->link[1])))
        {
            if (is_red(p))
                *done = 1;
            p->red = 0;
            sibling->red = 1;
        }
        else
        {
            int saved_color = root->red;
            int new_root = (root == p);

            if (is_red(sibling->link[!direction]))
                p = single_rotate(p, direction);
            else
                p = double_rotate(p, direction);
            p->red = saved_color;
            p->link[0]->red = 0;
            p->link[1]->red = 0;

            if (new_root)
                root = p;
            *done = 1;
        }
    }

    return root;
}

node_t *single_rotate(node_t *root, int direction)
{
    node_t *temp = root->link[!direction];

    root->link[!direction] = temp->link[direction];
    temp->link[direction] = root;

    root->red = 1;
    temp->red = 0;

    return temp;
}

node_t *double_rotate(node_t *root, int direction)
{
    root->link[!direction] = single_rotate(root->link[!direction], !direction);
    return single_rotate(root, direction);
}

void inorder(node_t *root)
{
    if (root->link[0])
        inorder(root->link[0]);
    printf ("\t%d,%s", root->data, root->red?"Red":"Black");
    if (root->link[1])
        inorder(root->link[1]);
}

/*
int main()
{
    node_t *root = NULL;
    while(1)
    {
        int d;
        if (scanf("%d", &d) == EOF)
            break;
        else
            root = insert(root, d);
    }
    int r;
    scanf("%d", &r);
    root = delete(root, r);
    inorder(root);
    printf("\n");
    return 0;
}
*/
