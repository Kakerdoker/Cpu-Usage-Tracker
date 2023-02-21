#include <stdlib.h>
#include <stdio.h>

#include "../inc/global.h"
#include "../inc/buffers.h"

#include "../inc/tests_basic.h"
#include "../inc/tests_analyzer.h"
#include "../inc/tests_logger.h"
#include "../inc/tests_printer.h"
#include "../inc/tests_reader.h"
#include "../inc/tests_watchdog.h"


static void initializeVariables(void){
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
