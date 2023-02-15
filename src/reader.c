#include <stdio.h>
#include <unistd.h>
#include "global.h"
#include "buffers.h"


void copyCpuInfoBuffer(){
    for(int i = 0; i < cpuCoreAmount; i++){
        previousCpuInfoBuffer[i] = currentCpuInfoBuffer[i];
    }
}

void readStatFileAndPutIntoBuffer(){
    //todo: read up on fopen and what to do to make it safer.
    FILE *statFile = fopen("/proc/stat", "r");
    int line = 0;
    //Go through all 10 numbers seperated by space ignoring the first string for every cpu core + 1 and add it to the corresponding index in cpuInfo.
    while(line < cpuCoreAmount){
        fscanf(
            statFile,
            "%*s %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu",
            &currentCpuInfoBuffer[line].user,
            &currentCpuInfoBuffer[line].nice,
            &currentCpuInfoBuffer[line].system,
            &currentCpuInfoBuffer[line].idle,
            &currentCpuInfoBuffer[line].iowait,
            &currentCpuInfoBuffer[line].irq,
            &currentCpuInfoBuffer[line].softirq,
            &currentCpuInfoBuffer[line].steal,
            &currentCpuInfoBuffer[line].guest,
            &currentCpuInfoBuffer[line].guest_nice
        );
        line++;
    }
    fclose(statFile);
}

void* getProcStatInfo(){
    while(1){
        sleep(1); //Read proc/stat once every second so the calculated cpu usage will be an average over that one second.
        mtx_lock(&cpuInfoMutex);//Lock the cpu information buffer for analyzeCpuInfo()

        copyCpuInfoBuffer();
        readStatFileAndPutIntoBuffer();

        mtx_unlock(&cpuInfoMutex);//Unlock the cpu information buffer
    }
    return NULL;
}