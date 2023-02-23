#include "../inc/analyzer.h"
#include "../inc/buffers.h"
#include "../inc/global.h"

#include "../inc/tests_analyzer.h"
#include "../inc/tests_basic.h"

//Tests if cpu usage calculated by addCpuUsageToBuffer() is correct, the value will always be 66.6% if read from statTest file created inside tests_reader.c
static void TEST_CalculatingCpuUsage(void){
    for(unsigned int core = 0; core < cpuCoreAmount; core++){
        myAssert(cpuUsageBuffer[core] >= ((double)2/3)*100, "ERROR Percentage calculated incorrectly!");
        myAssert(cpuUsageBuffer[core] <= ((double)2/3)*100, "ERROR Percentage calculated incorrectly!");
    }
}

void TEST_analyzerMethods(void){
    addCpuUsageToBuffer();
    TEST_CalculatingCpuUsage();
}
