#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

int main(int argc, char **argv){

    char s[20];
    strcpy(s,"/bin/");
    strcat(s,argv[1]);
    char com[20];
    strcpy(com,argv[1]);
    int i;
    char *args[argc];
    args[0] = com;
    args[argc-1]=NULL;
    if (argc >= 2) {
        for (i = 2; i < argc; i++) {
            args[i-1] = argv[i];
        }
    }
    printf("%s",s);
    if ( execv(s,args) )
    {
        printf("execv failed with error %d %s\n",errno,strerror(errno));
        return 254;
    }
    return 0;
}
