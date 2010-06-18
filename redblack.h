#ifndef REDBLACK_H
#define REDBLACK_H
#include "common.h"

node_t *insert(node_t *root, int fd, read_callback rcb);
node_t *delete(node_t *root, int data);

#endif /* REDBLACK_H */
