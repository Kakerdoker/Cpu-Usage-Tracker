#include <stdio.h>
#include <unistd.h>

#include "../inc/global.h"
#include "../inc/buffers.h"
#include "../inc/threads.h"

FILE *statFile;

void closeStatFile(){
    if(statFile != NULL){
        fclose(statFile);
        statFile = NULL;
    }
}

//Put it into a differen't function to make writing tests easier.
void openStatFile(char* fileDir){
    //todo: read up on fopen and what to do to make it safer.
    statFile = fopen(fileDir, "r");
}

void copyCpuInfoBuffer(){
    for(int i = 0; i < cpuCoreAmount; i++){
        previousCpuInfoBuffer[i] = currentCpuInfoBuffer[i];
    }
}

void readStatFileAndPutIntoBuffer(){
    int line = 0;
    
    //Skip first line
    fscanf(statFile, "%*s %*i %*i %*i %*i %*i %*i %*i %*i %*i %*i");

    //Go through all 10 numbers seperated by space ignoring the first cpu string and last guest & guest_nice variables. Do it for every cpu core and add it to the corresponding index in cpuInfo.
    while(line < cpuCoreAmount){
        fscanf(
            statFile,
            "%*s %lu %lu %lu %lu %lu %lu %lu %lu %*i %*i",
            &currentCpuInfoBuffer[line].user,
            &currentCpuInfoBuffer[line].nice,
            &currentCpuInfoBuffer[line].system,
            &currentCpuInfoBuffer[line].idle,
            &currentCpuInfoBuffer[line].iowait,
            &currentCpuInfoBuffer[line].irq,
            &currentCpuInfoBuffer[line].softirq,
            &currentCpuInfoBuffer[line].steal
        );
        line++;
    }
    closeStatFile();
}

int getProcStatInfo(){
    while(threadsActive){
        updateWatchdogBuffer(0);

        sleep(1); //Read proc/stat once every second so the calculated cpu usage will be an average over that one second.
        mtx_lock(&cpuInfoMutex);//Lock the cpu information buffer for analyzeCpuInfo()

        openStatFile("/proc/stat");
        copyCpuInfoBuffer();
        readStatFileAndPutIntoBuffer();

        mtx_unlock(&cpuInfoMutex);
    }
    thrd_exit(0);
}
