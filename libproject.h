#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include<sys/wait.h>

volatile sig_atomic_t usr1_receive = 0;
volatile sig_atomic_t usr2_receive = 0;
volatile sig_atomic_t alrm_receive = 0;
volatile sig_atomic_t chld_receive = 0;

int sortie, err, entree;
char s[32];
char e[32];
int temps_attendre=0;
int count=0,jj=0;


typedef struct{
    char** args;
    char* start;
    char* period;
    char* cmd;
    int start2;
    int num;
    char* decount;    
}commande;
commande *com;

int test_pid(int z);
void creat_fifo();
long now();
void add(commande *com, char** recv);
void sig_handler(int a);
void commande_a_executer(commande *com);
void remove_c(commande *com,int n);
