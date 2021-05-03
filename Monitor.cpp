#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <sched.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include "shm.h"
using namespace std;

// Compile & Run
//gcc Monitor.cpp -o monitor -lstdc++
// ./monitor

// the structure representing the shm
// & must match the same layout as in the server.cpp

//Monitor attatch to shm and print out the result

int main()
{
    // declare variables
    //Setup shm, Server serve as the producer (write to shm)
    int running = 1;
    void *shared_memory = (void *)0;
    struct shared_use_mem *shmPtr = NULL;
    char buffer[1024];
    int shmid;
    //Create shared mem
    shmid = shmget((key_t)1234, sizeof(shared_use_mem), 0666 | IPC_CREAT);
    if (shmid == -1)
    {
        fprintf(stderr, "shmget failed\n");
        exit(EXIT_FAILURE);
    }
    //attatch shared mem
    shared_memory = shmat(shmid, (void *)0, 0);
    if (shared_memory == (void *)-1)
    {
        fprintf(stderr, "shmat failed\n");
        exit(EXIT_FAILURE);
    }
    printf("Memory attached at %X\n", shared_memory);
    if (shmctl(shmid, SHM_LOCK, 0) == -1)
    {
        return (-1);
    }
    shmPtr = (shared_use_mem *)shared_memory;
    while (1)
    {
        for (int i = 0; i <= shmPtr->no_of_process - 1; i++)
        {
            cout << "Pid Client  = " << shmPtr->taskInfos[i].pid << " \nAffinity = " << shmPtr->taskInfos[i].currentAffinity << " \nCPU assignment = " << shmPtr->taskInfos[i].currentCPU << " \nPriority of nice = " << shmPtr->taskInfos[i].priority << endl;
            cout << "\nCPU core changed times: " << shmPtr->taskInfos[i].cpuChanged << endl;
            cout << "\nPriority changed times: " << shmPtr->taskInfos[i].priorityChanged << endl;
            cout << "\nAffinity changed times: " << shmPtr->taskInfos[i].affinityChanged << endl;
            sleep(2);
        }
    }
}