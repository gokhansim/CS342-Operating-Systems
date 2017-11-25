#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

void static process(const char* letter) {
    for (int i = 0; i < 100; i++) {
        printf("Current process:%s with pid:%d and parent id:%d\n",letter,getpid(),getppid());
    }
    sleep(5);
}
int main() {
    //Currently in process a
    pid_t pidb,pidc,pidf;
    process("a");
    pidb = fork();
    if (pidb < 0) {
        perror("fork failure");
        exit(EXIT_FAILURE);
    }
    else if (pidb == 0) {
        //in process b
        process("b");
        exit(0);
    }
    else {
        int returnStatus;
        waitpid(pidb, &returnStatus, 0);
    }
    pidc = fork();
    if (pidc < 0) {
        perror("fork failure");
        exit(EXIT_FAILURE);
    }
    else if (pidc == 0) {
        process("c");
        pid_t pidd,pide;
        pidd = fork();
        if (pidd < 0) {
            perror("fork failure");
            exit(EXIT_FAILURE);
        }
        else if (pidd == 0) {
            process("d");
            exit(0);
        }
        else {
            int returnStatus;
            waitpid(pidd, &returnStatus, 0);
        }
        pide = fork();
        if (pide < 0) {
            perror("fork failure");
            exit(EXIT_FAILURE);
        }
        else if (pide == 0) {
            process("e");
            exit(0);
        }
        else {
            int returnStatus;
            waitpid(pide, &returnStatus, 0);
        }
        exit(0);
    }
    else {
        int returnStatus;
        waitpid(pidc, &returnStatus, 0);
    }
    pidf = fork();
    if (pidf < 0) {
        perror("fork failure");
        exit(EXIT_FAILURE);
    }
    else if (pidf == 0) {
        process("f");
        pid_t pidg;
        pidg = fork();
        if (pidg < 0) {
            perror("fork failure");
            exit(EXIT_FAILURE);
        }
        else if (pidg == 0) {
            process("g");
            exit(0);
        }
        else {
            int returnStatus;
            waitpid(pidg, &returnStatus, 0);
        }
        exit(0);
    }
    else {
        int returnStatus;
        waitpid(pidf, &returnStatus, 0);
    }

    return 1;
}
