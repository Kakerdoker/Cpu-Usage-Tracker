#include <stdio.h>
#include <string.h>

#include "../inc/reader.h"
#include "../inc/global.h"
#include "../inc/buffers.h"

#include "../inc/tests_reader.h"
#include "../inc/tests_basic.h"


static void readFromStatFileWithSetContent(const char* content){
    //Fill out statFile with an example proc/stat file taken as the content argument.
    FILE *statFile = fopen("tests/statTest", "w");
    fprintf(statFile, "%s", content);
    fclose(statFile);

    //Open it inside reader.c and read it.
    openStatFile("tests/statTest");
    readStatFileAndPutIntoBuffer();
}

//Test if variables were corretly read and put into the buffer.
static void TEST_VariablesReadFromFile(const long unsigned int number){
    for(unsigned int core = 0; core < cpuCoreAmount; core++){
        myAssert(currentCpuInfoBuffer[core].user == number, "User cpu usage wasn't read properly");
        myAssert(currentCpuInfoBuffer[core].nice == number, "Nice cpu usage wasn't read properly");
        myAssert(currentCpuInfoBuffer[core].system == number, "System cpu usage wasn't read properly");
        myAssert(currentCpuInfoBuffer[core].idle == number, "Idle cpu usage wasn't read properly");     
        myAssert(currentCpuInfoBuffer[core].iowait == number, "Iowait cpu usage wasn't read properly");
        myAssert(currentCpuInfoBuffer[core].irq == 0, "Irq cpu usage wasn't read properly");
        myAssert(currentCpuInfoBuffer[core].softirq == number, "Softirq cpu usage wasn't read properly");
        myAssert(currentCpuInfoBuffer[core].steal == 0, "Steal cpu usage wasn't read properly");            
    }
}

//Test if everything copied corretly.
static void TEST_CopyBeforeNewRead(void){
    for(unsigned int core = 0; core < cpuCoreAmount; core++){
        myAssert(currentCpuInfoBuffer[core].user == previousCpuInfoBuffer[core].user, "User cpu usage wasn't copied properly");
        myAssert(currentCpuInfoBuffer[core].nice == previousCpuInfoBuffer[core].nice, "Nice cpu usage wasn't copied properly");
        myAssert(currentCpuInfoBuffer[core].system == previousCpuInfoBuffer[core].system, "System cpu usage wasn't copied properly");
        myAssert(currentCpuInfoBuffer[core].idle == previousCpuInfoBuffer[core].idle, "Idle cpu usage wasn't copied properly");
        myAssert(currentCpuInfoBuffer[core].iowait == previousCpuInfoBuffer[core].iowait, "Iowait cpu usage wasn't copied properly");
        myAssert(currentCpuInfoBuffer[core].irq == previousCpuInfoBuffer[core].irq, "Irq cpu usage wasn't copied properly");
        myAssert(currentCpuInfoBuffer[core].softirq == previousCpuInfoBuffer[core].softirq, "Softirq cpu usage wasn't copied properly");
        myAssert(currentCpuInfoBuffer[core].steal == previousCpuInfoBuffer[core].steal, "Steal cpu usage wasn't copied properly");
    }
}

//Test if newly read variables changed correctly.
static void TEST_CopyAfterNewRead(void){
    for(unsigned int core = 0; core < cpuCoreAmount; core++){
        myAssert(currentCpuInfoBuffer[core].user != previousCpuInfoBuffer[core].user, "User cpu usage wasn't read properly the second time");
        myAssert(currentCpuInfoBuffer[core].nice != previousCpuInfoBuffer[core].nice, "Nice cpu usage wasn't read properly the second time");
        myAssert(currentCpuInfoBuffer[core].system != previousCpuInfoBuffer[core].system, "System cpu usage wasn't read properly the second time");
        myAssert(currentCpuInfoBuffer[core].idle != previousCpuInfoBuffer[core].idle, "Idle cpu usage wasn't read properly the second time");
        myAssert(currentCpuInfoBuffer[core].iowait != previousCpuInfoBuffer[core].iowait, "Iowait cpu usage wasn't copied properly");
        myAssert(currentCpuInfoBuffer[core].softirq != previousCpuInfoBuffer[core].softirq, "Softirq cpu usage wasn't copied properly");
    }
}

void TEST_readerMethods(void){
    
    readFromStatFileWithSetContent("gfdklgjfdlkgjdflgf  4000 4000 4000 4000 4000 0 4000 0 0 0\nAAAAAAAAAAAAAAAAAA 1000 1000 1000 1000 1000 0 1000 0 0 0\nBBBBBBBBBBBBB 1000 1000 1000 1000 1000 0 1000 0 0 0\nC 1000 1000 1000 1000 1000 0 1000 0 0 0\nDDDDDDDDDDDDDD 1000 1000 1000 1000 1000 0 1000 0 0 0\n\nintr 23529732 12 \n\nDOESN'T\nMATTER\nSHOULD\nIGNORE\nTHIS\nPART\n\nctxt 120642943\nbtime 1676472680\nprocesses 17608\nprocs_running 1\nprocs_blocked 0\n");

    //Make sure all the variables are corretly read.
    TEST_VariablesReadFromFile(1000);

    //Copy currentBuffer to previousBuffer and test if they are the same.
    copyCpuInfoBuffer();
    TEST_CopyBeforeNewRead();

    readFromStatFileWithSetContent("gfdklgjfdlkgjdflgf  8000 8000 8000 8000 8000 0 8000 0 0 0\nAAAAAAAAAAAAAAAAAA 2000 2000 2000 2000 2000 0 2000 0 0 0\nBBBBBBBBBBBBB 2000 2000 2000 2000 2000 0 2000 0 0 0\nC 2000 2000 2000 2000 2000 0 2000 0 0 0\nDDDDDDDDDDDDDD 2000 2000 2000 2000 2000 0 2000 0 0 0\n\nintr 23529732 12 \n\nDOESN'T\nMATTER\nSHOULD\nIGNORE\nTHIS\nPART\n\nctxt 120642943\nbtime 1676472680\nprocesses 17608\nprocs_running 1\nprocs_blocked 0\n");

    //Make sure all the new variables are corretly read.
    TEST_VariablesReadFromFile(2000);

    //Make sure the current buffer and previous buffer are different from eachother.
    TEST_CopyAfterNewRead();
}
