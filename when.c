#include <stdio.h>
#include <time.h>

int main (){
    time_t seconds;
    seconds = time(NULL);
    printf ("Number of seconds since 1st January 1970 %ld\n", seconds);
    printf ("Date and local time: %s\n", ctime(&seconds));
    
}