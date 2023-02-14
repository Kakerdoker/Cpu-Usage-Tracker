#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <threads.h>
#include <semaphore.h>

struct CpuInfo{
    unsigned long user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
};

struct CpuInfo* currentCpuInfo;
struct CpuInfo* previousCpuInfo;

float** cpuUsageBuffer; 
int cpuUsageBufferIndex = 0;

float* avrgCpuUsage;

int cpuAmount;

void *analyzeCpuInfo();
void* getProcStatInfo();
void *printCpuUsagePercantages();

mtx_t cpuInfoMutex, cpuUsageMutex;
sem_t semUsageEmpty, semUsageFull;

int main(){

    cpuAmount = getAmountOfCpus()+1;
    allocateMemory();   

    mtx_init(&cpuInfoMutex, mtx_plain);
    mtx_init(&cpuUsageMutex, mtx_plain);

    sem_init(&semUsageEmpty, 0, 32);
    sem_init(&semUsageFull, 0, 0);

    thrd_t getThread, calcThread, printThread;

    thrd_create(&getThread, getProcStatInfo, NULL);
    thrd_create(&calcThread, analyzeCpuInfo, NULL);
    thrd_create(&printThread, printCpuUsagePercantages, NULL);
    
    thrd_join(getThread, NULL);
    thrd_join(calcThread, NULL);
    thrd_join(printThread, NULL);

    collectGarbage();

    mtx_destroy(&cpuInfoMutex);
    mtx_destroy(&cpuUsageMutex);

    sem_destroy(&semUsageEmpty);
    sem_destroy(&semUsageFull);
    
}

void allocateMemory(){
    currentCpuInfo = malloc(sizeof(struct CpuInfo)*cpuAmount);
    previousCpuInfo = malloc(sizeof(struct CpuInfo)*cpuAmount);

    avrgCpuUsage = malloc(sizeof(float)*cpuAmount);
    cpuUsageBuffer = malloc(sizeof(float*)*32);
    for(int i = 0; i < 32; i++){
        cpuUsageBuffer[i] = malloc(sizeof(float)*cpuAmount);
    }
    
}

void collectGarbage(){
    free(currentCpuInfo);
    free(previousCpuInfo);
    for(int i = 0; i < 32; i++){
        free(cpuUsageBuffer[i]);
    }
    free(cpuUsageBuffer);
    free(avrgCpuUsage);
}

/*
    Methods for reading proc/stat
*/
int getAmountOfCpus(){
    //If user has no cpu cores stop the application
    if(sysconf(_SC_NPROCESSORS_ONLN) == 0){
        //todo: Add sending message to logger
        exit(1);
    }
    return sysconf(_SC_NPROCESSORS_ONLN);
}

void copyCpuInfoBuffer(){
    for(int i = 0; i < cpuAmount; i++){
        previousCpuInfo[i] = currentCpuInfo[i];
    }
}

void readStatFileAndPutIntoBuffer(){
    //todo: read up on fopen and what to do to make it safer.
    FILE *statFile = fopen("/proc/stat", "r");
    int line = 0;
    //Go through all 10 numbers seperated by space ignoring the first string for every cpu core + 1 and add it to the corresponding index in cpuInfo.
    while(line < cpuAmount){
        fscanf(
            statFile,
            "%*s %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu",
            &currentCpuInfo[line].user,
            &currentCpuInfo[line].nice,
            &currentCpuInfo[line].system,
            &currentCpuInfo[line].idle,
            &currentCpuInfo[line].iowait,
            &currentCpuInfo[line].irq,
            &currentCpuInfo[line].softirq,
            &currentCpuInfo[line].steal,
            &currentCpuInfo[line].guest,
            &currentCpuInfo[line].guest_nice
        );
        line++;
    }
    fclose(statFile);
}

void* getProcStatInfo(){
    while(1){
        usleep(10000);//Sleep 10 milliseconds
        mtx_lock(&cpuInfoMutex);//Lock the cpu information buffer for analyzeCpuInfo()

        copyCpuInfoBuffer();
        readStatFileAndPutIntoBuffer();

        mtx_unlock(&cpuInfoMutex);//Unlock the cpu information buffer
    }
    return NULL;
}

/*
    Methods for analyzing cpu info
*/
long unsigned calculateIdle(struct CpuInfo* cpuInfo, int index){
    return cpuInfo[index].idle + cpuInfo[index].iowait;
}

long unsigned calculateNonIdle(struct CpuInfo* cpuInfo, int index){
    return cpuInfo[index].user + cpuInfo[index].nice + cpuInfo[index].system + cpuInfo[index].irq + cpuInfo[index].softirq + cpuInfo[index].steal;
}

float calculateCpuUsage(int index){

    long unsigned prevIdle = calculateIdle(previousCpuInfo,index);
    long unsigned currIdle = calculateIdle(currentCpuInfo,index);

    long unsigned prevNonIdle = calculateNonIdle(previousCpuInfo,index);
    long unsigned currNonIdle = calculateNonIdle(currentCpuInfo,index);
    
    long unsigned prevTotal = prevIdle + prevNonIdle;
    long unsigned currTotal = currIdle + currNonIdle;

    long totalDiff = currTotal - prevTotal;
    long idleDiff = currIdle - prevIdle;

    float CpuPercantage = 0;
    if(totalDiff != 0){
        CpuPercantage = ((float)(totalDiff-idleDiff)/(float)totalDiff)*100;
    }
    
    return CpuPercantage;
}

void addCpuUsageToBuffer(){
    //Repeat for every core
    for(int i = 0 ; i < cpuAmount; i++){
        cpuUsageBuffer[cpuUsageBufferIndex][i] = calculateCpuUsage(i);
    }
    cpuUsageBufferIndex++;
}

void *analyzeCpuInfo(){
    while(1){
        usleep(10000);//Wait 10 milliseconds
        mtx_lock(&cpuInfoMutex); //Lock the cpu information buffer for getProcStatInfo()

        sem_wait(&semUsageEmpty); //Lock semaphore and decrement it
        mtx_lock(&cpuUsageMutex); //Lock cpu usage buffer for printCpuUsagePercantages()
        
        addCpuUsageToBuffer();

        mtx_unlock(&cpuUsageMutex); //Unlock cpu usage buffer
        sem_post(&semUsageFull); //Unlock semaphore and increment it

        mtx_unlock(&cpuInfoMutex);//Unlock cpu information buffer
    }
    return NULL;
}

/*
    Methods for printing and calculating average
*/

void clearAvrgUsage(){
    for(int i = 0; i < cpuAmount; i++){
        avrgCpuUsage[i] = 0;
    }
}

void averageTheSum(){
    for(int i = 0; i < cpuAmount; i++){
        avrgCpuUsage[i] /= (float)cpuUsageBufferIndex;
    }
}

void calcAvrgFromUsageBuffer(){
    for(int buffer = 0; buffer < cpuUsageBufferIndex; buffer++){
        for(int cpu = 0; cpu < cpuAmount; cpu++){
            avrgCpuUsage[cpu] += cpuUsageBuffer[buffer][cpu];
        }
        sem_trywait(&semUsageFull);//Decrement semaphore without unlocking it until it reaches 0. Maybe put it in it's own loop and method for better readability?
    }
    averageTheSum();
}

void printCpuUsageAverages(){
    char percantageBar[10];

    for(int i = 0; i < cpuAmount; i++){
        int simplifiedPercent = (int)avrgCpuUsage[i]/10;
        for(int j = 0; j < 10; j++){
            percantageBar[j] = j < simplifiedPercent ? '#' : '-';
        }
        printf("CPU%i - [%s] - %.1f%%\n",i ,percantageBar, avrgCpuUsage[i]);
    }
}

//Takes the average from the entire usage buffer every second and then fully clears it and prints the averages.
void *printCpuUsagePercantages(){
    while(1){
        sleep(1);
        system("clear");

        sem_wait(&semUsageFull); //Lock semaphore and decrement it
        mtx_lock(&cpuUsageMutex); //Lock cpu usage buffer for analyzeCpuInfo()

        clearAvrgUsage();
        calcAvrgFromUsageBuffer();

        printCpuUsageAverages();

        cpuUsageBufferIndex = 0; //Reset index to zero

        mtx_unlock(&cpuUsageMutex);//Unlock cpu usage buffer
        sem_post(&semUsageEmpty); //Unlock semaphore and increment it
    }
    return NULL;
}

//todo: what next: split into files and write tests



