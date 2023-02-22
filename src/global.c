#include <stdlib.h>
#include <unistd.h>

#include "../inc/global.h"
#include "../inc/logger.h"
#include "../inc/destroyer.h"

unsigned int cpuCoreAmount;

//What: Reads the amount of active cpu cores
//What for: So the rest of the program knows how to carry out it's calculations
static void setAmountOfCpuCores(void){
    //If user has no cpu cores stop the application
    if(sysconf(_SC_NPROCESSORS_ONLN) == 0){
        closeProgramByError("The cpu somehow read that there are 0 threads available (global.c).");
    }
    cpuCoreAmount = (unsigned int)sysconf(_SC_NPROCESSORS_ONLN);
}

//What: Initialize every global variable
//What for: So it can be used by the rest of the program
void initializeGlobalVariables(void){
    setAmountOfCpuCores();
}
