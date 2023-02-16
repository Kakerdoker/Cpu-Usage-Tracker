#ifndef READER_H
#define READER_H

void* getProcStatInfo();
void openStatFile(char*);
void readStatFileAndPutIntoBuffer();
void copyCpuInfoBuffer();

#endif