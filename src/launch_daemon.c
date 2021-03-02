#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

/*

Launch_daemon lance la programme Period (ou un autre programme avec un chemin absolue)
    comme un daemon, en utilisant double fork;
*/
int main(int argc, char **argv){
    pid_t pid;
    char* x[]={argv[1], 0};
    pid=fork(); //Le processus principal forke
    if(pid==0){
        close(0);
        close(1);
        close(2);
        pid=fork();//Le fils forke
        setsid(); //Le fils devient leader de session via setsid(2)
        if(pid>0){
            exit(0);
        }
        if(pid==0){
            
            execvp(x[0],x);
            exit(0); //Le fils se termine, rendant le petit-fils orphelin
        }

        exit(0); //Le processus principal termine, laissant le petit-fils être le daemon
    }
    waitpid(pid,0,0);//Le processus principal attends son fils
    return 0;

}