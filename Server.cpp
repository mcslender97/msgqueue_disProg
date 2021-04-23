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
    int mypid;
    //priority ( niceness )
    int priority;
    //CPU affinity
    int affinity;
    //current cpu assignment
    unsigned cpu;

    char buff[512]; //string type, needs to be of a fixed size
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

    //Setup shm, Server serve as the producer (write to shm)
    int running = 1;
    void *shared_memory = (void *)0;
    struct shared_use_mem *shared_memory;
    char buffer[512];
    int shmid;

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
    for (int x = 0; x < 10; ++x) //ahhh we might miss messages :)
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
        // this is a blocking example, working off the Q: possible thread option....
        if (msgrcv(msqid, &msg, sizeof(msg) - sizeof(long), MSG_CLIENT1_CHANNEL, IPC_NOWAIT) < 0)
        {

            perror("msgrcv1");
            //exit(1);
        }
        else
            cout << "Pid Client1 = " << msg.mypid << " buff = " << msg.buff << endl;

        if (msgrcv(msqid, &msg, sizeof(msg) - sizeof(long), MSG_CLIENT2_CHANNEL, IPC_NOWAIT) < 0)
        {
            perror("msgrcv2");
            //exit(1);
        }

        else
            cout << "Pid Client2  = " << msg.mypid << " buff = " << msg.buff << "Affinity = " << msg.affinity << "CPU assignment = " << msg.cpu << "Priority of nice = " << msg.priority << endl;
        cout << "Type: " << msg.messageType << endl;
        sleep(1);
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
