#include "libproject.h"
#include "message.h"

/*
Tester si le programme Period est en cours d'execution;
Si Period est existe, lire le pid dans le fichier "/tmp/period.pid";
Si ce n'est pas le cas creer le fichier "/tmp/period.pid";
*/
int test_pid(int z){

    FILE* f;
    int x;
    f=fopen("/tmp/period.pid", "r");
    if(f==NULL){
        if(z==1){
            f=fopen("/tmp/period.pid", "w+");
            x= getpid();
            fprintf(f,"%d\n", x );
        }else{
            printf("No period program\n");
        }
    }else{
        fscanf(f, "%d ", &x);
    }
    
    fclose(f);
    return x;
}

/*
Création du tube nomée "/tmp/period.fifo";
*/
void creat_fifo(){
    if(mkfifo("/tmp/period.fifo",0644)){
        perror("mkfifo");
        exit(EXIT_FAILURE);
    }
}   

/*
Récupèrer le temps actuel dans le format Epoch;
*/
long now(){
    time_t seconds;
    seconds = time(NULL);
    return seconds;
}

/*
Ajouter les donnees reçues du programme Periodic dans la structure dynamique;
La fonction recoit la structure "com" et les donnees reçues;

    -Allocation de la memoire pour chaque element de la structure;
    -Affectation des donnees recues dans leur propre element du structure; 
*/
void add(commande *com, char** recv){
    int nb=0;
    while(1){
       if(recv[nb]==NULL) break;
       nb++;
    }
    nb=nb-2;//On cherche le nombre des options dans la commande pour allouer apres la memoire
    com[count].start=(char*)malloc(sizeof(char));
    com[count].period=(char*)malloc(sizeof(char));
    com[count].cmd=(char*)malloc(sizeof(char));
    com[count].decount=(char*)malloc(sizeof(char));
    com[count].args=calloc(nb,sizeof(char));
    for(int i=0;i<nb;i++){
        com[count].args[i]= malloc(strlen(recv[i+2]));
    }
    com[count].start=recv[0];
    com[count].period=recv[1];
    com[count].cmd=recv[2];
    sprintf(com[count].decount,"%d",atoi(recv[1])+2);
    com[count].start2=atoi(recv[0]);
    if (recv[2]!=NULL){// Verification s'il y a des option pour la coomande;
        for(int i=0;i<nb;i++){
            com[count].args[i]=recv[2+i];
        }
    }
    com[count].args[nb]=NULL; //La derniere case de l'args (option) toujours egale a NULL pour l'utiliser apres dans fork et execvp;
    count++;
    com[count-1].num=count;
}

/*
Supression d'une commande de la liste des commandes (structure dynamique);
La fonction recoit la structure "com" et le numero de commande a supprimer;
*/
void remove_c(commande *com,int n){
    if(count==1 || n==count) {
        free(com[n-1].start);
        free(com[n-1].period);
        free(com[n-1].args);
        free(com[n-1].cmd);
        free(com[n-1].decount);
    }
    else {
        for(int i=n;i<count;i++){
            sprintf(com[i-1].start,"+%d",com[i].start2);
            sprintf(com[i-1].period,"%s",com[i].period);
            sprintf(com[i-1].args[0],"%s",com[i].args[0]);
            if(com[i].args[1]!=NULL) com[i-1].args[1]=realloc(com[i].args[1],2);
            sprintf(com[i-1].cmd,"%s",com[i].cmd);
            sprintf(com[i-1].decount,"%s",com[i].decount);
        }
        free(com[count-1].start);
        free(com[count-1].period);
        free(com[count-1].args);
        free(com[count-1].cmd);
    }
    count--;
}

/*
Handler de signal();
*/
void sig_handler(int a){
    if(a==SIGALRM){
        alrm_receive=1;
    }
    if(a==SIGINT || a==SIGQUIT || a==SIGTERM){
        unlink("/tmp/period.pid");//Suppression des fichiers "/tmp/period.pid", ("/tmp/period.fifo");
        unlink("/tmp/period.fifo");
        for(int i=0;;i++){
            if(recv_a[i]==NULL) break;
            free(recv_a[i]);
        }
        free(recv_a);
        free(recv_s);
        if(com[count-1].args[0]!=NULL){
            for(int i=0;i<count;i++){
                free(com[i].start);
                free(com[i].period);
                free(com[i].args);
                free(com[i].cmd);
                free(com[i].decount);
            }
        }
        free(com);
        exit(0);
    }
    if(a==SIGUSR1){
        usr1_receive=1;
    }
    if(a==SIGUSR2){
        usr2_receive=1;

    }
    if(a==SIGCHLD){
        chld_receive=1;
    }
}

/*
Calcul du temps a attendre avant l'exécution des commandes de la structure;
Fonction recoit la structure "com";
    1- Verification si start est egale a "now"; 
        si c'est le cas => execution du comamnde directement sans attendre; 
    2-Recherche de plus petit element de la structure pour calculer le temps d'attente;
    3-Creation d'alarm avec le temps a attendre;
    4-Attendre le signal SIGALRM;
    5-Creer un fils et executer la commande;
    6-Redirection stdin,stdout,stderr dans leur propres fichiers;
    7-Calcul du nouveau temps a attendre pour chaque commande apres l'execution de la commande precedente;
*/
void commande_a_executer(commande *com){
    pid_t pid;
    int status,fd;
    int exi=0;
    int x=0,k=1;
    int j[25]={0};

    if(strcmp(com[count-1].start,"now")==0){
        j[0]=count-1;
        sprintf(com[count-1].start,"%s",com[count-1].period);
        com[count-1].start2=atoi(com[count-1].start);
        goto m;
    }

    /*
    Dans le cas de "now" on a le argv de period qui represente les secondes a attendre.
    Donc on stocke la variable period dans variable start(secondes);  
    */
    int g=0,w=0,y=0;
    if(count==1 && strcmp(com[0].decount,"end")==0) pause(); 
    else if(count>1){
        for (int i=0;i<count;i++){
            if(strcmp(com[i].decount,"end")!=0){
                y=i;
                g++;
                break;
            }else w++;
        }
        /*
        Dormir jusqu’à ce qu’une commande doive être exécutée.
        Si toutes les commandes sont terminees
        */
        if(count==1 && strcmp(com[0].decount,"end")==0){
            pause();
        }
        if(w==count || y==count) pause();
    
        if(count-g==1 && strcmp(com[count-1].decount,"end")==0) j[0]=y+1;
        else{
            x = atoi(com[y].start);
            j[0]=y;
            for (int i = y+1; i < count; i++){
                if(x>atoi(com[i].start) && strcmp(com[i].decount,"end")!=0 && strcmp(com[i].start,"end")!=0){
                    x=atoi(com[i].start);
                    j[0]=i;// j[0]= le numero de la commande qui doit s'executer;
                }else if(x==atoi(com[i].start) && strcmp(com[i].decount,"end")!=0 && strcmp(com[i].start,"end")!=0){
                    j[k]=i;// j[k] represente les numeros des commandes qui doivent s'executer en meme temps; 
                    k++;
                }
            }
        }
    }
    temps_attendre = atoi(com[j[0]].start);
    if(atoi(com[j[0]].decount)>0 && strcmp(com[j[0]].start,"end")!=0 && atoi(com[j[0]].start)!=0){
        alarm(temps_attendre);
        signal(SIGALRM,sig_handler);
        pause();
        if(alrm_receive){
        m:  sprintf(s,"/tmp/period/%d.out",com[j[0]].num);
            sprintf(e,"/tmp/period/%d.err",com[j[0]].num);
            sortie=open(s,O_WRONLY| O_APPEND| O_CREAT, 0666);
            err=open(e,O_WRONLY| O_APPEND| O_CREAT, 0666);
            pid= fork();
            if(pid==-1) {
                perror("fork");
                exit(1);
            }
            if(pid==0){
                dup2(entree,0);
                dup2(sortie,1);
                dup2(err,2);
                exi=execvp(com[j[0]].args[0],com[j[0]].args);
                close(entree);
                close(sortie);
                close(err);
                exit(exi);
            }
            waitpid(pid,&status,0);
            if(status!=0){
                kill(pid,SIGTERM);
            }
            if(chld_receive==1 && WIFEXITED(status)){
                printf("pid %d  status %d\n",pid,status);
            }
            
            if(com[j[0]].start[0]!='+') {
                sprintf(com[j[0]].decount,"%d",atoi(com[j[0]].decount)+2);
            }
            else{
                sprintf(com[j[0]].decount,"%d",atoi(com[j[0]].decount)-1);
                if(atoi(com[j[0]].decount)==0) memcpy(com[j[0]].decount,"end",3*sizeof(char));
            }
            alrm_receive = 0;
            }
    
    //Actualisation de nouveau temps a attendre apres le calcul du precedent temps a attendre;
        for(int z=0;z<count;z++ ){
            int n = atoi(com[z].start)-temps_attendre;
            if(com[z].start[0]=='+' && n<=0 && atoi(com[z].decount)>0 && strcmp(com[z].start,"end")!=0) {
                if(n==0 && atoi(com[z].decount)==1) strcpy(com[z].decount,"end");
                else sprintf(com[z].start,"+%d",com[z].start2);
            }
            else if(com[z].start[0]=='+' && strcmp(com[z].start,"end")!=0) sprintf(com[z].start,"+%d",n);
            else if(com[z].start[0]!='+'&& atoi(com[z].decount)>0 && strcmp(com[z].start,"end")!=0) sprintf(com[z].start,"%d",n);
            if(com[z].start[0]!='+' && atoi(com[z].start)==0 && strcmp(com[z].start,"end")!=0 && atoi(com[z].decount)>0) sprintf(com[z].start,"%d",com[z].start2);
        }
        temps_attendre=0;  
    }
}
