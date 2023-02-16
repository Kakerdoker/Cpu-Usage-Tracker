#include <stdlib.h>
#include <unistd.h>

/*
    global.c is used for storing and calculating variables used by multiple files
*/
unsigned int cpuCoreAmount;

void setAmountOfCpuCores(){
    //If user has no cpu cores stop the application
    if(sysconf(_SC_NPROCESSORS_ONLN) == 0){
        //todo: Add sending message to logger
        exit(1);
    }
    cpuCoreAmount = sysconf(_SC_NPROCESSORS_ONLN);
}

void initializeGlobalVariables(){
    setAmountOfCpuCores();
}