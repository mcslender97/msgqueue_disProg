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
// g++ Server.cpp -o Server -lstdc++
// ./Server

// the structure representing the message queue
// & must match the same layout as in the client.cpp
struct MsgQueue
{
    // IMPORTANT: every message structure must start with this
    long messageType;

    // these variables are optional & you can add
    // more or less if you wish
    // pid of process
    int mypid;
    //priority ( niceness )
    int priority;
    //CPU affinity
    int affinity;
    //current cpu assignment
    unsigned cpu;

    char buff[1024]; //string type, needs to be of a fixed size
};

// message queue flag
const int MSG_Q_KEY_FLAG = 0666;

// message queue data transfer channel

int main()
{
    // declare variables
    key_t key = -1;
    int msqid = -1;
    MsgQueue msg;
    // List of task pid that was approved by system
    int runningClients = 0;

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
    shmPtr->no_of_process = 0;

    // generate a unique key. The same parameters to this function will
    // always generate the same value. This is how multiple
    // processes can connect to the same queue.
    key = ftok("./bozo", 0);

    // was the key allocation successful ?
    if (key < 0)
    {
        perror("ftok error");
        exit(1);
    }

    // allocate the message queue if it does not already exist.
    // this function returns the id of the queue.
    msqid = msgget(key, MSG_Q_KEY_FLAG | IPC_CREAT);

    // was the allocation a success ?
    if (msqid < 0)
    {
        perror("msgget");
        exit(1);
    }

    // display info to the screen
    cout << "\nThe server has started!\n"
         << "\nWaiting for someone to connect to server id #" << msqid << " with "
         << "the key " << key << endl
         << endl;

    // recieve 10 messages from the client
    while (1) //ahhh we might miss messages :)
    {
        // this is where we receive messages:
        //  msqid - the id of the message queue
        //  msg - the message structure which stores the
        //   received message
        //  sizeof(msg) - sizeof(long) - size of the message
        //   excluding the required first member (messageType) which is
        //   required.
        //  MSG_Q_CHANNEL - receive all messages whose type parameter
        //   is set equal to "MSG_Q_CHANNEL"
        //  - flag values (not useful for this example).
        // this is a blocking example, working off the Q: possible thread option..
        if (msgrcv(msqid, &msg, sizeof(msg) - sizeof(long), 0l, 0) < 0)
        {
            // msg.messageType = approvedPID;
            // msgsnd(msqid, )
            cout << "Waiting for client to send msg 42" << endl;
        }
        else if (msg.messageType == 42)
        {
            msg.messageType = msg.mypid;
            cout << "\nNew connection request from cliend pid: " << msg.mypid << endl;

            if (msgsnd(msqid, &msg, sizeof(msg) - sizeof(long), 0) < 0)
            {
                perror("msgsnd");
            }
            else
            {
                runningClients++;
                shmPtr->no_of_process = runningClients;
                cout << "Number of running client is: " << runningClients;
                shmPtr->taskInfos[runningClients - 1].currentAffinity = msg.affinity;
                shmPtr->taskInfos[runningClients - 1].currentCPU = msg.cpu;
                shmPtr->taskInfos[runningClients - 1].priority = msg.priority;
                shmPtr->taskInfos[runningClients - 1].pid = msg.mypid;
            }
        }
        else
        {
            for (int i = 0; i < shmPtr->no_of_process; i++)
            {
                if (msg.mypid == shmPtr->taskInfos[i].pid)
                {

                    if (msg.affinity != shmPtr->taskInfos[i].currentAffinity)
                    {
                        shmPtr->taskInfos[i].currentAffinity = msg.affinity;
                        shmPtr->taskInfos[i].affinityChanged++;
                    }
                    if (msg.cpu != shmPtr->taskInfos[i].currentCPU)
                    {
                        shmPtr->taskInfos[i].currentCPU = msg.cpu;
                        shmPtr->taskInfos[i].cpuChanged++;
                        //cout << "\nCPU core changed: " << shmPtr->cpuChanged << endl;
                    }
                    if (msg.priority != shmPtr->taskInfos[i].priority)
                    {
                        shmPtr->taskInfos[i].priority = msg.priority;
                        shmPtr->taskInfos[i].priorityChanged++;
                        //cout << "\nPriority changed: " << shmPtr->priorityChanged << endl;
                    }

                    cout << "Type: " << msg.messageType << endl;
                }
            }
        }
        usleep(10);
    }
}

// finally, deallocate the message queue
/*
    if(msgctl(msqid, IPC_RMID, NULL) < 0)
    {
        perror("msgctl");
        exit(1);
    }
*/
