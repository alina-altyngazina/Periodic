#include "libproject.h"
#include "message.h"

int main(int argc, char **argv){

    char*plus;
    char* endptr;
    int fd;
    int pid;
    char* recv;
    plus=argv[1];
   
    if (argc==1) goto k;
/*
Verficiation de syntax des 3 cas(Epoch,now,+x) de start dans argv[1];
*/
    

    if(strcmp(argv[1],"remove")==0){
        pid=test_pid(0);
        kill(pid,SIGUSR1);
        fd=open("/tmp/period.fifo", O_WRONLY);
        send_argv(fd,argv);
        close(fd);
    }
    else{
        long m=strtol(argv[1],&endptr,10);
        if((strcmp(argv[1],"now")!=0 && plus[0]!='+' && strlen(argv[1])!=10) ||
            ((strlen(argv[1])!=10 || m<now()) && plus[0]!='+' && strcmp(argv[1],"now")!=0) || 
            (endptr[0]!='\0' && strcmp(argv[1],"now")!=0)){
            printf("invalid start\n");
        }else {
        
    /*
    Test of inputs;
    Si le nombre des argv est supérieur à 3, envoyer le signal SIGUSR1 au programme Period;
        Ensuite, envoyer les argv a travers le tube  nommée "/tmp/period.fifo"

    Si le nombre des argv inférieur à 3, envoyer le signal SIGUSR2 au programme Period;
        Attendre de recevoir la liste des commandes depuis Period;
        
    */   
        
            k:pid=test_pid(0);
            if(argc>=3){
                if (strcmp(argv[1],"now")==0) printf("On souhaite executer %s toutes les %s secondes a partir de maintenant\n",argv[3],argv[2]);
                if (plus[0]=='+') printf("On souhaite executer %s %s fois toutes les %s secondes\n",argv[3],argv[2],argv[1]);
                if (strlen(argv[1])==10) printf("On souhaite executer %s à %s\n",argv[3],argv[1]);
                kill(pid, SIGUSR1);
                fd= open("/tmp/period.fifo", O_WRONLY);
                send_argv(fd,argv);
                close(fd);
                
            }
            if(argc==1){
                kill(pid,SIGUSR2);
                fd= open("/tmp/period.fifo", O_RDONLY);
                for(;;){
                    recv=recv_string(fd);
                    if(recv[0]=='\0') break;
                    printf(" %s",recv);
                }
                close(fd);
            }
        }
    }
return 0;
}
