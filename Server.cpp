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
// g++ Server.cpp -o Server
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
    int myPid;
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
const int MSG_CLIENT1_CHANNEL = 26;
const int MSG_CLIENT2_CHANNEL = 27;

int main()
{
    // declare variables
    key_t key = -1;
    int msqid = -1;
    MsgQueue msg;
    // List of task pid that was approved by system
    int approvedPID[5];
    for (int i = 0; i < 5; i++)
    {
        approvedPID[i] = -1;
    }

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
        if (msgrcv(msqid, &msg, sizeof(msg) - sizeof(long), 42, IPC_NOWAIT) < 0)
        {
            // msg.messageType = approvedPID;
            // msgsnd(msqid, )
            cout << "Waiting for client to send msg 42" << endl;
        }
        else
        {
            for (int i = 0; i < 5; i++)
            {
                if (msg.myPid != approvedPID[i])
                {
                    approvedPID[i] = msg.myPid;
                    msg.messageType = approvedPID[i];
                    cout << "Approved message from task id: " << approvedPID[i] << endl;
                }
            }
        }
        for (int i = 0; i < 5; i++)
        {
            if (msgrcv(msqid, &msg, sizeof(msg) - sizeof(long), approvedPID[i], IPC_NOWAIT) < 0)
            {
                perror("msgrcv");
            }
            else
            {

                shmPtr->no_of_process++;
                if (msg.affinity != shmPtr->taskInfos[0].currentAffinity)
                {
                    shmPtr->taskInfos[0].currentAffinity = msg.affinity;
                    shmPtr->affinityChanged++;
                    cout << "\nAffinity changed: " << shmPtr->affinityChanged << endl;
                }
                if (msg.cpu != shmPtr->taskInfos[0].currentCPU)
                {
                    shmPtr->taskInfos[0].currentCPU = msg.cpu;
                    shmPtr->cpuChanged++;
                    cout << "\nCPU core changed: " << shmPtr->cpuChanged << endl;
                }
                if (msg.priority != shmPtr->taskInfos[0].priority)
                {
                    shmPtr->taskInfos[0].priority = msg.priority;
                    shmPtr->priorityChanged++;
                    cout << "\nPriority changed: " << shmPtr->priorityChanged << endl;
                }

                cout << "Type: " << msg.messageType << endl;
            }
        }

        sleep(2);
    }

    // finally, deallocate the message queue
    /*
    if(msgctl(msqid, IPC_RMID, NULL) < 0)
    {
        perror("msgctl");
        exit(1);
    }
*/
    cout << "\nServer is now shutting down!\n";

    return 0;
}
