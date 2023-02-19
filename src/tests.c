#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "../inc/buffers.h"
#include "../inc/global.h"
#include "../inc/reader.h"
#include "../inc/analyzer.h"
#include "../inc/printer.h"
#include "../inc/watchdog.h"
#include "../inc/logger.h"

char *tempString1, *tempString2, *realString, *correctString, *testedString;

thrd_t logger, tester;

void collectTestGarbage(){
    free(correctString);
    free(testedString);
    free(tempString1);
    free(tempString2);
    free(realString);
}

void cleanUp(){
    collectTestGarbage();
    collectBufferGarbage();
    destroySemaphores();
}

//Works like an assert but cleans up everything and prints a specified message before exiting
void myAssert(int statement, char* message){
    if(statement != 1){
        cleanUp();
        printf("TEST FAILED: %s\n", message);
        exit(1);
    }
    
};

/*



    READER TESTING METHODS BELOW
*/

void readFromStatFileWithSetContent(char* content){
    //Fill out statFile with an example proc/stat file (with some gibberish added to make it a little bit harder for the program).
    FILE *statFile = fopen("tests/statTest", "w");
    fprintf(statFile, "%s", content);
    fclose(statFile);

    //Open it inside reader.c and read it.
    openStatFile("tests/statTest");
    readStatFileAndPutIntoBuffer();
}

//Test if variables were corretly read and put into the buffer.
void TEST_VariablesReadFromFile(int number){
    for(int core = 0; core < cpuCoreAmount; core++){
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
void TEST_CopyBeforeNewRead(){
    for(int core = 0; core < cpuCoreAmount; core++){
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
void TEST_CopyAfterNewRead(){
    for(int core = 0; core < cpuCoreAmount; core++){
        myAssert(currentCpuInfoBuffer[core].user != previousCpuInfoBuffer[core].user, "User cpu usage wasn't read properly the second time");
        myAssert(currentCpuInfoBuffer[core].nice != previousCpuInfoBuffer[core].nice, "Nice cpu usage wasn't read properly the second time");
        myAssert(currentCpuInfoBuffer[core].system != previousCpuInfoBuffer[core].system, "System cpu usage wasn't read properly the second time");
        myAssert(currentCpuInfoBuffer[core].idle != previousCpuInfoBuffer[core].idle, "Idle cpu usage wasn't read properly the second time");
        myAssert(currentCpuInfoBuffer[core].iowait != previousCpuInfoBuffer[core].iowait, "Iowait cpu usage wasn't copied properly");
        myAssert(currentCpuInfoBuffer[core].softirq != previousCpuInfoBuffer[core].softirq, "Softirq cpu usage wasn't copied properly");
    }
}

/*



    ANALYZER TESTING METHODS BELOW
*/

//Test if cpu usage is calculated correctly.
void TEST_CalculatingCpuUsage(){
    for(int core = 0; core < cpuCoreAmount; core++){
        myAssert(cpuUsageBuffer[core] == ((float)2/3)*100, "ERROR Percentage calculated incorrectly!");
    }
}

/*



    PRINTER TESTING METHODS BELOW
*/

//Test if a percentage bar has the right amount of #'s at edge cases.
void TEST_percentageBarAtEdge(char* bar, int num){
    char percentageBar[9];

    //Create a bar and compare it with bar defined by bar argument.
    cpuUsageBuffer[0] = num;
    strcpy(percentageBar, makePercentageBar(0));
    myAssert(strcmp(percentageBar, bar) == 0, "Percentage bar wasn't the same as the one provided in this functions argument");

    cpuUsageBuffer[0] = 0.99+num;
    strcpy(percentageBar, makePercentageBar(0));
    myAssert(strcmp(percentageBar, bar) == 0, "Percentage bar wasn't the same as the one provided in this functions argument");
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

        //Create a a string with 66.7% cpu usage. (Also make sure it doesn't go over allocated size of 128)
        myAssert(sprintf(correctString, "CPU%i - [######---] - 66.7%%\n",core+1) < 128, "correctString too big in TEST_percentagePrints()");

        //Create cpu usage string using buffer values. (Also make sure it doesn't go over allocated size of 128)
        myAssert(sprintf(testedString, "CPU%i - [%s] - %.1f%%\n",core+1 ,percentageBar, cpuUsageBuffer[core]) < 128, "testedString too big in TEST_percentagePrints()");

        //Compare them
        myAssert(strcmp(correctString, testedString) == 0, "ERROR comparing percentage bars!");
    }
}

/*



    WATCHDOG TESTING METHODS BELOW
*/

void TEST_threadMessages(){
    myAssert(strcmp(getMessageFromThread(0), "Reader thread stopped responding, closing program!") == 0, "ERROR got incorrect message from reader thread!");
    myAssert(strcmp(getMessageFromThread(1), "Analyzer thread stopped responding, closing program!") == 0, "ERROR got incorrect message from analyzer thread !");
    myAssert(strcmp(getMessageFromThread(2), "Printer thread stopped responding, closing program!") == 0, "ERROR got incorrect message from printer thread!");
    myAssert(strcmp(getMessageFromThread(999), "Unknown thread stopped responding, closing program!") == 0, "ERROR got incorrect message from unknown thread!");    
    myAssert(strcmp(getMessageFromThread(-999), "Unknown thread stopped responding, closing program!") == 0, "ERROR got incorrect message from unknown thread!");    
}

//Checks if checkAllThreadResponses() returns appropriate value for every thread being responsive in the last 2 seconds.
void TEST_twoSecondsResponse(){
    sem_post(&messageBuffEmpty);//So logger doesn't get blocked

    for(int thread = 0; thread < THREAD_AMOUNT; thread++){
        updateBuffer[thread] = time(NULL) - 2;
    }
    myAssert(checkAllThreadResponses() == -1, "ERROR One of the threads being responsive returned an unresponsive value!");
    
    sem_wait(&messageBuffEmpty);
}

//Tests if checkAllThreadResponses() returns appropriate value for every thread being unresponsive for 3 seconds.
void TEST_threeSecondsResponse(){
    sem_post(&messageBuffEmpty);//So logger doesn't get blocked

    //Since checkAllThreadResponses() checks threads from 0 to THREAD_AMOUNT and returns the first thread that doesn't respond,
    //then the for loop below should check them in reverse order to make sure that every thread gets checked for not responding once, instead of the first thread getting checked THREAD_AMOUNT of times.
    for(int thread = THREAD_AMOUNT-1; thread >= 0; thread--){
        updateBuffer[thread] = time(NULL) - 3;
        myAssert(checkAllThreadResponses() == thread, "ERROR One of the threads being unresponsive for three seconds returned a responsive value!");
    }
    sem_wait(&messageBuffEmpty);
}

//Test to see if watchdog still checks for responsiveness even if semEmpty == 0, which means that logger is waiting for new messages to log.
void TEST_loggerBlockForUnresponsivness(){
    updateBuffer[THREAD_AMOUNT-1] = time(NULL) - 3;
    myAssert(checkLoggerThread() == -1, "Logger was waiting for message but watchdog still checked to see if it was responsive. Did you sem_post(&messageBuffEmpty) anywhere in the test without sem_wait afterwards?");
}

/*



    LOGGER TESTING METHODS BELOW
*/

char logF[20] = "testLogs/logs.txt";

void TEST_initializeLogger(){
    //Delete files
    remove("testLogs/logs.txt");
    rmdir("testLogs");

    initializeLogger("testLogs","logs.txt");//Create again the just deleted files
    myAssert(access(logF, F_OK) == 0, "initializeLogger() hasn't sucesfully created the logs.txt file");//Check if they exist

    //Read first 2 strings of the newly created file and put them inside testedString
    FILE *loggerFile = fopen(logF, "r");
    fscanf(loggerFile, "%s %s %*s %*s %*s %*s %*s %*s", tempString1, tempString2);    

    //Make sure combined string doesn't go over allocated size of 128
    myAssert(sprintf(testedString, "%s %s",tempString1, tempString2) < 128, "testedString too big in TEST_initializeLogger()");
    fclose(loggerFile);

    //Compare read string with what should be in logs.txt
    myAssert(strcmp("PROGRAM STARTED",testedString) == 0, "initializeLogger() didn't log the message PROGRAM STARTED");
}

void spamTwentyLogMessages(){
    //Buffer size for messages is 16, so 20 will be enough to test if it leaks/overwrites/ignores messages.
    for(int i = 0; i < 20; i++){
        system("clear");
        printf("Putting messages into logger: %i\n", i);

        //Make sure combined string doesn't go over allocated size of 128
        myAssert(sprintf(testedString, "testmessage %i", i) < 128, "testedString too big in spamTwentyLogMessages()");
        logMessage(testedString);
    }
}

//It takes a little under 9 seconds for the logger to finish logging all the messages into logs.txt
void giveLoggerTenSecondsToFinish(){
    for(int i = 9; i >= 0; i--){
        sleep(1);
        system("clear");
        printf("Testing logger %i\n", i);
    }
    system("clear");
}

int putMessagesIntoLogger(){
    spamTwentyLogMessages();
    giveLoggerTenSecondsToFinish();

    thrd_detach(logger);
    thrd_exit(0);
}

void TEST_readMessagesFromLogger(){

    FILE *loggerFile = fopen(logF, "r");

    //Skip first line
    fscanf(loggerFile, "%*s %*s %*s %*s %*s %*s %*s %*s");

    //Go through all the spammed messages
    for(int i = 0; i < 20; i++){
        fscanf(loggerFile, "%s %s %*s %*s %*s %*s %*s %*s", tempString1, tempString2);

        //Make sure combined strings don't go over allocated size of 128
        myAssert(sprintf(testedString, "%s %s",tempString1, tempString2) < 128, "testedString too big in TEST_readMessagesFromLogger()");
        myAssert(sprintf(realString, "testmessage %i", i) < 128, "realString too big in TEST_readMessagesFromLogger()");

        //Check if the message was logged correctly, if it wasn't then break so we can close the file, after which we assert the untrue statement.
        if(strcmp(realString,testedString) != 0){
            break;
        }
    }
    fclose(loggerFile);
    myAssert(strcmp(realString,testedString) == 0, "Logger didn't log a spammed message correctly.");
}

//Asynchronously ask 20 messages to be logged while also logging these messages at the same time
void TEST_overflowingMessageBuffer(){
    printf("Starting threads to test logger\n");
    thrd_create(&tester, putMessagesIntoLogger, NULL);//Function to ask for the messages to be logged
    thrd_create(&logger, waitForNewMessagesToLog, NULL);//Function from logger.c to be tested
   
    thrd_join(tester, NULL);
    thrd_join(logger, NULL);
    
    //After logger logged the messages, check if they were correct.
    TEST_readMessagesFromLogger();
}

//Check if logger correctly logs that the program has closed
void TEST_closing(){
    //Call the function we are testing
    closeLogger();
    
    FILE *loggerFile = fopen(logF, "r");
    //Skip first 21 lines
    for(int i = 0; i < 21; i++){
        fscanf(loggerFile, "%*s %*s %*s %*s %*s %*s %*s %*s");
    }
    fscanf(loggerFile, "%s %s %*s %*s %*s %*s %*s %*s", tempString1, tempString2);   
    fclose(loggerFile);

    //Make sure combined string doesn't go over allocated size of 128
    myAssert(sprintf(testedString, "%s %s",tempString1, tempString2) < 128, "testedString too big in TEST_closing()");
    
    myAssert(strcmp(testedString,"PROGRAM CLOSED") == 0, "closeLogger() didn't log the message \"PROGRAM CLOSED\"");
}

/*



    METHODS WITH TESTS FOR EACH SEGMENT OF THE PROGRAM
*/

void TEST_readerMethods(){
    
    readFromStatFileWithSetContent("gfdklgjfdlkgjdflgf  4000 4000 4000 4000 4000 0 4000 0 0 0\nAAAAAAAAAAAAAAAAAA 1000 1000 1000 1000 1000 0 1000 0 0 0\nBBBBBBBBBBBBB 1000 1000 1000 1000 1000 0 1000 0 0 0\nC 1000 1000 1000 1000 1000 0 1000 0 0 0\nDDDDDDDDDDDDDD 1000 1000 1000 1000 1000 0 1000 0 0 0\n\nintr 23529732 12 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 5 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 829383 110335 21 499676 91 56 0 68 15 23 2804435 42 7649 916 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n\nDOESN'T\nMATTER\nSHOULD\nIGNORE\nTHIS\nPART\n\nctxt 120642943\nbtime 1676472680\nprocesses 17608\nprocs_running 1\nprocs_blocked 0\n");

    //Make sure all the variables are corretly read.
    TEST_VariablesReadFromFile(1000);

    //Copy currentBuffer to previousBuffer and test if they are the same.
    copyCpuInfoBuffer();
    TEST_CopyBeforeNewRead();

    readFromStatFileWithSetContent("gfdklgjfdlkgjdflgf  8000 8000 8000 8000 8000 0 8000 0 0 0\nAAAAAAAAAAAAAAAAAA 2000 2000 2000 2000 2000 0 2000 0 0 0\nBBBBBBBBBBBBB 2000 2000 2000 2000 2000 0 2000 0 0 0\nC 2000 2000 2000 2000 2000 0 2000 0 0 0\nDDDDDDDDDDDDDD 2000 2000 2000 2000 2000 0 2000 0 0 0\n\nintr 23529732 12 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 5 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 829383 110335 21 499676 91 56 0 68 15 23 2804435 42 7649 916 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n\nDOESN'T\nMATTER\nSHOULD\nIGNORE\nTHIS\nPART\n\nctxt 120642943\nbtime 1676472680\nprocesses 17608\nprocs_running 1\nprocs_blocked 0\n");

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
    //If logger is waiting for messages then watchdog shouldn't count logger as unresponsive.
    TEST_loggerBlockForUnresponsivness();
}

void TEST_loggerMethods(){
    TEST_initializeLogger();
    TEST_overflowingMessageBuffer();
    TEST_closing();
}


void initializeTestVariables(){
    correctString = malloc(sizeof(char)*128);
    testedString = malloc(sizeof(char)*128);
    tempString1 = malloc(sizeof(char)*128);
    tempString2 = malloc(sizeof(char)*128);
    realString = malloc(sizeof(char)*128);
}

void initializeVariables(){
    allocateBufferMemory();
    initializeSemaphores();
    initializeTestVariables();
}


int main(){
    cpuCoreAmount = 4;//Testing as if someone had 4 cores, adding more without changing statFile will not work.
    system("clear");
    printf("Test started\n");
    
    initializeVariables();

    TEST_readerMethods();
    TEST_analyzerMethods();
    TEST_printerMethods();
    TEST_watchdogMethods();
    TEST_loggerMethods();

    cleanUp();

    printf("TEST RAN SUCCESFULLY! YAY\n\n");
}