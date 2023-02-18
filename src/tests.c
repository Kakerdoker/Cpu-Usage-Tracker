#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "../inc/buffers.h"
#include "../inc/global.h"
#include "../inc/reader.h"
#include "../inc/analyzer.h"
#include "../inc/printer.h"
#include "../inc/watchdog.h"


//Test if variables were corretly read and put into the buffer.
void TEST_VariablesReadFromFile(int number){
    for(int core = 0; core < cpuCoreAmount; core++){
        assert(currentCpuInfoBuffer[core].user == number);
        assert(currentCpuInfoBuffer[core].nice == number);
        assert(currentCpuInfoBuffer[core].system == number);
        assert(currentCpuInfoBuffer[core].idle == number);
        assert(currentCpuInfoBuffer[core].iowait == number);
        assert(currentCpuInfoBuffer[core].irq == 0);
        assert(currentCpuInfoBuffer[core].softirq == number);
        assert(currentCpuInfoBuffer[core].steal == 0);
    }
}

//Test if everything copied corretly.
void TEST_CopyBeforeNewRead(){
    for(int core = 0; core < cpuCoreAmount; core++){
        assert(currentCpuInfoBuffer[core].user == previousCpuInfoBuffer[core].user);
        assert(currentCpuInfoBuffer[core].nice == previousCpuInfoBuffer[core].nice);
        assert(currentCpuInfoBuffer[core].system == previousCpuInfoBuffer[core].system);
        assert(currentCpuInfoBuffer[core].idle == previousCpuInfoBuffer[core].idle);
        assert(currentCpuInfoBuffer[core].iowait == previousCpuInfoBuffer[core].iowait);
        assert(currentCpuInfoBuffer[core].irq == previousCpuInfoBuffer[core].irq);
        assert(currentCpuInfoBuffer[core].softirq == previousCpuInfoBuffer[core].softirq);
        assert(currentCpuInfoBuffer[core].steal == previousCpuInfoBuffer[core].steal);
    }
}

//Test if newly read variables changed correctly.
void TEST_CopyAfterNewRead(){
    for(int core = 0; core < cpuCoreAmount; core++){
        assert(currentCpuInfoBuffer[core].user != previousCpuInfoBuffer[core].user);
        assert(currentCpuInfoBuffer[core].nice != previousCpuInfoBuffer[core].nice);
        assert(currentCpuInfoBuffer[core].system != previousCpuInfoBuffer[core].system);
        assert(currentCpuInfoBuffer[core].idle != previousCpuInfoBuffer[core].idle);
        assert(currentCpuInfoBuffer[core].iowait != previousCpuInfoBuffer[core].iowait);
        assert(currentCpuInfoBuffer[core].softirq != previousCpuInfoBuffer[core].softirq);
    }
}

//Test if cpu usage is calculated correctly.
void TEST_CalculatingCpuUsage(){
    for(int core = 0; core < cpuCoreAmount; core++){
        assert(cpuUsageBuffer[core] == ((float)2/3)*100 && "ERROR Percentage calculated incorrectly!");
    }
}

//Test if a percentage bar has the right amount of #'s at edge cases.
void TEST_percentageBarAtEdge(char* bar, int num){
    char percentageBar[9];

    //Create a bar and compare it with bar defined by bar argument.
    cpuUsageBuffer[0] = num;
    strcpy(percentageBar, makePercentageBar(0));
    assert(strcmp(percentageBar, bar) == 0);

    cpuUsageBuffer[0] = 0.99+num;
    strcpy(percentageBar, makePercentageBar(0));
    assert(strcmp(percentageBar, bar) == 0);
}

//Test if all percentage bar edgecases have the right amount of #'s.
void TEST_everyPercentageBarEdge(){
    TEST_percentageBarAtEdge("---------", 0);
    TEST_percentageBarAtEdge("#--------", 10);
    TEST_percentageBarAtEdge("##-------", 20);
    TEST_percentageBarAtEdge("###------", 30);
    TEST_percentageBarAtEdge("####-----", 40);
    TEST_percentageBarAtEdge("#####----", 50);
    TEST_percentageBarAtEdge("######---", 60);
    TEST_percentageBarAtEdge("#######--", 70);
    TEST_percentageBarAtEdge("########-", 80);
    TEST_percentageBarAtEdge("#########", 90);
}

//Test if what's going to be displayed using printer is done so correctly.
void TEST_percentagePrints(){
    for(int core = 0; core < cpuCoreAmount; core++){
        //Generate cpu usage bar using buffer values.
        char percentageBar[9];
        strcpy(percentageBar, makePercentageBar(core));

        //Create a a string with 66.7% cpu usage.
        char* correctString = malloc(sizeof(char)*64);
        sprintf(correctString, "CPU%i - [######---] - 66.7%%\n",core+1);

        //Create cpu usage string using buffer values.
        char* testedString = malloc(sizeof(char)*64);
        sprintf(testedString, "CPU%i - [%s] - %.1f%%\n",core+1 ,percentageBar, cpuUsageBuffer[core]);

        //Compare them
        assert(strcmp(correctString, testedString) == 0 && "ERROR comparing percentage bars!");

        free(correctString);
        free(testedString);
    }
}

void TEST_threadMessages(){
    assert(strcmp(getMessageFromThread(0), "Reader thread stopped responding, closing program!") == 0 && "ERROR got incorrect message from reader thread!");
    assert(strcmp(getMessageFromThread(1), "Analyzer thread stopped responding, closing program!") == 0 && "ERROR got incorrect message from analyzer thread !");
    assert(strcmp(getMessageFromThread(2), "Printer thread stopped responding, closing program!") == 0 && "ERROR got incorrect message from printer thread!");
    assert(strcmp(getMessageFromThread(999), "Unknown thread stopped responding, closing program!") == 0 && "ERROR got incorrect message from unknown thread!");    
    assert(strcmp(getMessageFromThread(-999), "Unknown thread stopped responding, closing program!") == 0 && "ERROR got incorrect message from unknown thread!");    
}

//Checks if checkAllThreadResponses() returns appropriate value for every thread being responsive in the last 2 seconds.
void TEST_twoSecondsResponse(){
    sem_post(&messageBuffEmpty);//So logger doesn't get blocked

    for(int thread = 0; thread < THREAD_AMOUNT; thread++){
        updateBuffer[thread] = time(NULL) - 2;
    }
    assert(checkAllThreadResponses() == -1 && "ERROR One of the threads being responsive returned an unresponsive value!");
    
    sem_wait(&messageBuffEmpty);
}

//Tests if checkAllThreadResponses() returns appropriate value for every thread being unresponsive for 3 seconds.
void TEST_threeSecondsResponse(){
    sem_post(&messageBuffEmpty);//So logger doesn't get blocked

    //Since checkAllThreadResponses() checks threads from 0 to THREAD_AMOUNT and returns the first thread that doesn't respond,
    //then the for loop below should check them in reverse order to make sure that every thread gets checked for not responding once, instead of the first thread getting checked THREAD_AMOUNT of times.
    for(int thread = THREAD_AMOUNT-1; thread >= 0; thread--){
        updateBuffer[thread] = time(NULL) - 3;
        assert(checkAllThreadResponses() == thread && "ERROR One of the threads being unresponsive for three seconds returned a responsive value!");
    }
    sem_wait(&messageBuffEmpty);
}

//Test to see if watchdog still checks for responsiveness even if semEmpty == 0, which means that logger is waiting for new messages to log.
void TEST_loggerBlockForUnresponsivness(){
    updateBuffer[THREAD_AMOUNT-1] = time(NULL) - 3;
    assert(checkLoggerThread() == -1 && "Logger was waiting for message but watchdog still checked to see if it was responsive. Did you sem_post(&messageBuffEmpty) anywhere in the test without sen_wait after?");
}

void TEST_readerMethods(){
    
    //Fill out statFile with an example proc/stat file (with some gibberish added to make it a little bit harder for the program).
    FILE *statFile = fopen("tests/statTest", "w");
    fprintf(statFile,"gfdklgjfdlkgjdflgf  4000 4000 4000 4000 4000 0 4000 0 0 0\nAAAAAAAAAAAAAAAAAA 1000 1000 1000 1000 1000 0 1000 0 0 0\nBBBBBBBBBBBBB 1000 1000 1000 1000 1000 0 1000 0 0 0\nC 1000 1000 1000 1000 1000 0 1000 0 0 0\nDDDDDDDDDDDDDD 1000 1000 1000 1000 1000 0 1000 0 0 0\n\nintr 23529732 12 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 5 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 829383 110335 21 499676 91 56 0 68 15 23 2804435 42 7649 916 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n\nDOESN'T\nMATTER\nSHOULD\nIGNORE\nTHIS\nPART\n\nctxt 120642943\nbtime 1676472680\nprocesses 17608\nprocs_running 1\nprocs_blocked 0\n");
    fclose(statFile);

    //Open it inside reader.c and read it.
    openStatFile("tests/statTest");
    readStatFileAndPutIntoBuffer();

    //Make sure all the variables are corretly read.
    TEST_VariablesReadFromFile(1000);

    //Copy currentBuffer to previousBuffer and test if they are the same.
    copyCpuInfoBuffer();
    TEST_CopyBeforeNewRead();

    //Fill out statFile with an example proc/stat file that's different from the previous one.
    statFile = fopen("tests/statTest", "w");
    fprintf(statFile,"gfdklgjfdlkgjdflgf  8000 8000 8000 8000 8000 0 8000 0 0 0\nAAAAAAAAAAAAAAAAAA 2000 2000 2000 2000 2000 0 2000 0 0 0\nBBBBBBBBBBBBB 2000 2000 2000 2000 2000 0 2000 0 0 0\nC 2000 2000 2000 2000 2000 0 2000 0 0 0\nDDDDDDDDDDDDDD 2000 2000 2000 2000 2000 0 2000 0 0 0\n\nintr 23529732 12 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 5 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 829383 110335 21 499676 91 56 0 68 15 23 2804435 42 7649 916 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n\nDOESN'T\nMATTER\nSHOULD\nIGNORE\nTHIS\nPART\n\nctxt 120642943\nbtime 1676472680\nprocesses 17608\nprocs_running 1\nprocs_blocked 0\n");
    fclose(statFile);

    //Open it inside reader.c and read it.
    openStatFile("tests/statTest");
    readStatFileAndPutIntoBuffer();

    //Make sure all the new variables are corretly read.
    TEST_VariablesReadFromFile(2000);

    //Make sure the current buffer and previous buffer are different from eachother.
    TEST_CopyAfterNewRead();
}

void TEST_analyzerMethods(){
    //Calculate the usage
    addCpuUsageToBuffer();
    //Check if it's equal to 66.6% on all cores.
    TEST_CalculatingCpuUsage();
}

void TEST_printerMethods(){
    //Make sure printer prints out everything correctly. Since TEST_everyPercentageBarEdge() changes cpuUsageBuffer[0] and TEST_percentagePrints() uses it, make sure TEST_percentagePrints() is always tested first.
    TEST_percentagePrints();
    TEST_everyPercentageBarEdge(); 
}

void TEST_watchdogMethods(){
    TEST_twoSecondsResponse();
    TEST_threeSecondsResponse();
    TEST_threadMessages();
    TEST_loggerBlockForUnresponsivness();
}

int main(){
    cpuCoreAmount = 4;//Testing as if someone had 4 cores, adding more without changing statFile will not work.

    allocateBufferMemory();
    initializeSemaphores();

    TEST_readerMethods();
    TEST_analyzerMethods();
    TEST_printerMethods();
    TEST_watchdogMethods();

    collectBufferGarbage();
    destroySemaphores();

    printf("\nTEST RAN SUCCESFULLY! YAY\n\n");
}