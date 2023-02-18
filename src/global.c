#include <stdlib.h>
#include <unistd.h>

#include "../inc/logger.h"

/*
    global.c is used for storing and calculating general variables used by multiple files
*/
unsigned int cpuCoreAmount;

void setAmountOfCpuCores(){
    //If user has no cpu cores stop the application
    if(sysconf(_SC_NPROCESSORS_ONLN) == 0){
        logMessage("The cpu somehow read that there are 0 threads available (global.c).");
        exit(1);//todo: exit from destroyer
    }
    cpuCoreAmount = sysconf(_SC_NPROCESSORS_ONLN);
}

void initializeGlobalVariables(){
    setAmountOfCpuCores();
}