#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct my_msgbuf
{
    long    mtype;
    int     messageContent;
};

int main() {
    int msqid;
    key_t key;
    struct my_msgbuf rcvbuf;
    size_t buf_length;

    key = 1112;

    if ((msqid = msgget(key, 0666 )) < 0) {
        perror("msgget");
        exit(1);
    }
    buf_length = sizeof(long);
    while(true) {
        if (msgrcv(msqid, &rcvbuf, buf_length, 1, 0) < 0) {
            perror("msgrcv");
            exit(1);
        }
        printf("%d\n",rcvbuf.messageContent);
    }

}
