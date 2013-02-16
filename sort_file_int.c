#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

struct node_t {
    int val;
    struct node_t *next;
};

struct node_t *alloc_node(void)
{
    struct node_t *p;
    p = malloc(sizeof(struct node_t));
    if (p == NULL) {
        fprintf(stderr, "Memory allocation error\n");
        exit(EXIT_FAILURE);
    }
    return p;
}

void free_node(struct node_t *d)
{
    free(d);
}

void insert_after_node(struct node_t *new, struct node_t **pnext)
{
    new->next = *pnext;
    *pnext = new;
}

void remove_after_node(struct node_t **ppos)
{
    struct node_t *d = *ppos;
    *ppos = d->next;
}

void print_list(struct node_t *h)
{
    struct node_t *p;
    for (p = h; p != NULL; p = p->next)
        printf("%d ", p->val);
    printf("\n");
}

void insert_sorted_list(struct node_t *new, struct node_t **pp)
{
    struct node_t *p;
    for (p = *pp; p != NULL; pp = &p->next, p = p->next)
        if (p->val > new->val) {
            insert_after_node(new, pp);
            return;
        }
    insert_after_node(new, pp);
}

void insert_value(int v, struct node_t **phead)
{
    struct node_t *new;

    new = alloc_node();
    new->val = v;
    insert_sorted_list(new, phead);
}

int main(int argc, char *argv[])
{
    struct node_t *list_head = NULL;
    int v;

    if (argc != 1) {
        fprintf(stderr, "Usage: %s [< file] [> file]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    while (feof(stdin) == 0)
        if (fscanf(stdin, "%d", &v) == 1)
            insert_value(v, &list_head);
    print_list(list_head);
    return EXIT_SUCCESS;
}
