#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>


struct my_msgbuf
{
    long    mtype;
    int     messageContent;
};

int main() {
    int msqid;
    int msgflg = IPC_CREAT | 0666;
    key_t key;
    struct my_msgbuf sbuf;
    size_t buf_length;

    key = 1112;

    if ((msqid = msgget(key, msgflg )) < 0) {
        perror("msgget");
        exit(1);
    }

    sbuf.mtype = 1;
    int a;
    pid_t pid;
    for (a = 0; a < 3; a++) {
        pid = fork();
        if (pid < 0) {
            perror("fork failure");
            exit(EXIT_FAILURE);
        }
        else if (pid == 0) {
            printf("Process is created.\n");
            int i;
            for ( i = 1; i <= 1000; i++ ){
                sbuf.messageContent = i;
                buf_length = sizeof(i);
                if (msgsnd(msqid, &sbuf, buf_length, IPC_NOWAIT) < 0) {
                    perror("msgsnd");
                    exit(1);
                }
            }
            exit(0);
        }
        else {
            int returnStatus;
            waitpid(pid, &returnStatus, 0);
        }
    }
}
