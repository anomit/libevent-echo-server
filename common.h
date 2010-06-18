#ifndef COMMON_H
#define COMMON_H
/*
 * 0 is for left direction
 * 1 is for right direction
 */
typedef struct connection_info
{
    int data;
    int red;
    struct connection_info *link[2];
    struct event ev;
} node_t;

typedef void (*read_callback) (int, short, void *);
#endif /* COMMON_H */
