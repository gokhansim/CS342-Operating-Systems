#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
int main() {
        struct timeval start,end;
        int numiter = 10000;
        int a,b;

        gettimeofday(&start,NULL);
        for(a=0; a<numiter; a++){
                getpid();
        }
        gettimeofday(&end,NULL);

        printf("System call average time in nanoseconds: %f\n",(end.tv_sec*1000000+end.tv_usec - (start.tv_sec * 1000000 + start.tv_usec))*1000/(numiter*1.0));

        gettimeofday(&start,NULL);
        for(b = 0; b < numiter; b++) {
                1+1;
        }
        gettimeofday(&end,NULL);
	printf("Procedure call average time in nanoseconds: %f\n",(end.tv_sec*1000000+end.tv_usec - (start.tv_sec * 1000000 + start.tv_usec))*1000/(numiter*1.0));

}

