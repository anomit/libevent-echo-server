/*
 * Libevent echo server
 * Established connections are maintained in a red black tree
 * Run as ./server <bind IP address> <port>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <event.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include "common.h"
#include "redblack.h"

void sock_callback(int fd, short event, void *arg);
void read_cb(int fd, short event, void *arg);


int main(int argc, char *argv[])
{
    int yes = 1;
    int sockfd = socket(AF_INET, SOCK_STREAM|SOCK_NONBLOCK, 0);
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[2]));
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    memset(serv_addr.sin_zero, '\0', sizeof serv_addr.sin_zero);
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof serv_addr) == -1)
    {
        printf("Problem with bind\n");
        exit(1);
    }
    listen(sockfd, 10000);
    node_t *root = NULL;

    struct event_base *ev_base = event_init();
    event_base_loop(ev_base, EVLOOP_NONBLOCK);
    struct event ev;
    event_set(&ev, sockfd, EV_READ|EV_PERSIST, sock_callback, root);
    event_add(&ev, NULL);
    event_dispatch();

    return 0;
}

void sock_callback(int fd, short event, void *arg)
{
    printf ("Got connection fd:%d\n", fd);
    node_t *root = (node_t *)arg;
    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);
    int afd;
    afd = accept(fd, (struct sockaddr *)&client_addr, &addrlen);
    fcntl(afd, F_SETFL, fcntl(afd, F_GETFL) | O_NONBLOCK);
    root = insert(root, afd, read_cb);
}

void read_cb(int fd, short event, void *arg)
{
    char buf[1024];
    memset(buf, '\0', sizeof(buf));
    node_t *root = (node_t *)arg;
    ssize_t recvd_bytes = recv(fd, buf, sizeof(buf), 0);
    /*
     * Client closed connection
     */
    printf("Received bytes on fd %d: %u\n", fd, recvd_bytes);
    if ( recvd_bytes == 0)
    {
        root = delete(root, fd);
        close(fd);
    }
    else
    {
        size_t to_send = strlen(buf);
        ssize_t sent = 0;
        while (sent < to_send)
            sent += send(fd, buf+sent, strlen(buf+sent), 0);
    }
}
