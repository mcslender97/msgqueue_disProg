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
};
struct shared_use_mem
{
    int no_of_process;
    struct taskInfo taskInfos[5];
};
