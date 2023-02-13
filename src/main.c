#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

struct CpuInfo{
    unsigned long user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
};

struct CpuInfo* cpuInfo;
int statCpuAmount;

int main(){
    statCpuAmount = getAmountOfCpus()+1;
    allocateCoresToCpuInfo();
    getProcStatInfo();
    printProcStatInfo();
    collectGarbage();
}

int getAmountOfCpus(){
    //If user has no cpu cores stop the application
    if(sysconf(_SC_NPROCESSORS_ONLN) == 0){
        //todo: Add sending message to logger
        exit(1);
    }
    return sysconf(_SC_NPROCESSORS_ONLN);
}

void allocateCoresToCpuInfo(){
    cpuInfo = malloc(sizeof(struct CpuInfo)*statCpuAmount);
}

void collectGarbage(){
    free(cpuInfo);
}

void printProcStatInfo(){
    for(int i = 0 ; i < statCpuAmount; i++){
        printf("%lu %lu %lu %lu %lu %lu %lu %lu %lu %lu\n",
        cpuInfo[i].user,
        cpuInfo[i].nice,
        cpuInfo[i].system,
        cpuInfo[i].idle,
        cpuInfo[i].iowait,
        cpuInfo[i].irq,
        cpuInfo[i].softirq,
        cpuInfo[i].steal,
        cpuInfo[i].guest,
        cpuInfo[i].guest_nice
        );
    }
}

void getProcStatInfo(){

    int line = 0;
    FILE *statFile = fopen("/proc/stat", "r");
    //Go through all 10 numbers seperated by space ignoring the first string for every cpu core + 1 and add it to the corresponding index in cpuInfo.
    while(line < statCpuAmount){
        fscanf(
            statFile,
            "%*s %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu",
            &cpuInfo[line].user,
            &cpuInfo[line].nice,
            &cpuInfo[line].system,
            &cpuInfo[line].idle,
            &cpuInfo[line].iowait,
            &cpuInfo[line].irq,
            &cpuInfo[line].softirq,
            &cpuInfo[line].steal,
            &cpuInfo[line].guest,
            &cpuInfo[line].guest_nice
        );
        line++;
    }
}



