#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>

struct taskInfo
{
    int currentCPU;
    int currentAffinity;
    int priority;
    int pid;
    int affinityChanged = 0;
    int cpuChanged = 0;
    int priorityChanged = 0;
};
struct shared_use_mem
{

    int no_of_process = 0;
    struct taskInfo taskInfos[5];
};
