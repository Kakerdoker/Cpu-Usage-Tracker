#include <stdio.h>
#include <unistd.h>

#include "../inc/reader.h"
#include "../inc/global.h"
#include "../inc/buffers.h"
#include "../inc/threads.h"
#include "../inc/logger.h"

static FILE *statFile;

void closeStatFile(void){
    if(statFile != NULL){
        fclose(statFile);
        statFile = NULL;
    }
}

void openStatFile(const char* fileDir){
    if(statFile == NULL){
         statFile = fopen(fileDir, "r");
    }
    else{
        logMessage("Tried to open opened file. (reader.c)");
    }
}

//What: Copies all the values from currentCpuInfoBuffer into previousCpuInfoBuffer
//What for: So analyzer can calculate the average cpu usage between the time this function was called twice
//(if it gets called every second then the average will be calculated for the given second)
void copyCpuInfoBuffer(void){
    for(unsigned int core = 0; core < cpuCoreAmount; core++){
        previousCpuInfoBuffer[core] = currentCpuInfoBuffer[core];
    }
}

//What: Reads only the important values from the currently opened stat file
//What for: So they can be used to calculate cpu usage
void readStatFileAndPutIntoBuffer(void){
    unsigned int line = 0;

    //Skip first line (we don't need it)
    fscanf(statFile, "%*s %*i %*i %*i %*i %*i %*i %*i %*i %*i %*i");

    //Go through all 10 numbers ignoring the first cpu string and last guest & guest_nice variables.
    //Do it for every cpu core and add it to the corresponding index in cpuInfo.
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

//What: Reads values from proc/stat
//What for: so they can be used by analyzer.c to calculate average cpu usage
static void readProcStat(void){
    openStatFile("/proc/stat");
    copyCpuInfoBuffer();
    readStatFileAndPutIntoBuffer();
}

//What: Calls readProcStat() every (args) microseconds and uses mutexes to lock analyzerLoop
//What for: So it can be synchronized with analyzerLoop so the data won't change when it's being read
int readerLoop(void* args){
    unsigned int delay = *(unsigned int *)args;

    while(threadsActive){
        updateWatchdogBuffer(0);

        usleep(delay); //Read proc/stat once every N microseconds so the calculated cpu usage will be an average over that period of time.
        mtx_lock(&cpuInfoMutex);//Lock the cpu information buffer for analyzerLoop()

        readProcStat();

        mtx_unlock(&cpuInfoMutex);
    }
    thrd_exit(0);
}
