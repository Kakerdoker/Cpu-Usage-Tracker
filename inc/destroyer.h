#ifndef DESTROYER_H
#define DESTROYER_H

//Closes program, ideally to be called from the watchdog thread
void closeProgramByWatchdog(const char*) __attribute__ ((noreturn));

//Initializes sigaction to catch termination signals
void initializeSigaction(void);

//Closes the program with given error message
void closeProgramByError(char*) __attribute__ ((noreturn));

#endif
