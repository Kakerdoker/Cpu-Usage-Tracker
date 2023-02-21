#include <string.h>
#include <stdio.h>

#include "../inc/printer.h"
#include "../inc/buffers.h"
#include "../inc/global.h"

#include "../inc/tests_printer.h"
#include "../inc/tests_basic.h"

//Test if a percentage bar has the right amount of #'s at edge cases (e.g. 1.99, 2.00).
static void TEST_percentageBarAtEdge(const char* bar, const int num){
    char percentageBar[10];

    //Create a bar and compare it with bar defined by bar argument.
    cpuUsageBuffer[0] = num;
    strcpy(percentageBar, makePercentageBar(0));
    myAssert(strcmp(percentageBar, bar) == 0, "Percentage bar wasn't the same as the one provided in this functions argument");

    cpuUsageBuffer[0] = 0.99+num;
    strcpy(percentageBar, makePercentageBar(0));
    myAssert(strcmp(percentageBar, bar) == 0, "Percentage bar wasn't the same as the one provided in this functions argument");
}

//Test if all percentage bar edgecases have the right amount of #'s.
static void TEST_everyPercentageBarEdge(void){
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

//Test if the value that was calculated inside analyzer will be displayed correctly.
static void TEST_percentagePrints(void){
    for(unsigned int core = 0; core < cpuCoreAmount; core++){
        
        //Generate cpu usage bar using buffer values.
        char percentageBar[10];
        strcpy(percentageBar, makePercentageBar(core));
        //Create a a string with 66.7% cpu usage. (Also make sure it doesn't go over allocated size of 128)
        myAssert(sprintf(correctString, "CPU%i - [######---] - 66.7%%\n",core+1) < 128, "correctString too big in TEST_percentagePrints()");

        //Create cpu usage string using buffer values. (Also make sure it doesn't go over allocated size of 128)
        myAssert(sprintf(testedString, "CPU%i - [%s] - %.1f%%\n",core+1 ,percentageBar, cpuUsageBuffer[core]) < 128, "testedString too big in TEST_percentagePrints()");
        
        myAssert(strcmp(correctString, testedString) == 0, "ERROR comparing percentage bars!");
        
    }
}


void TEST_printerMethods(void){
    //Make sure printer prints out everything correctly. Since TEST_everyPercentageBarEdge() changes cpuUsageBuffer[0] and TEST_percentagePrints() uses it, make sure TEST_percentagePrints() is always tested first.
    TEST_percentagePrints();
    TEST_everyPercentageBarEdge(); 
}
