#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <sched.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include "msq.h"
using namespace std;

// Compile & Run
// g++ Client.cpp -o Client -lstdc++
// ./Client

// message queue flag
const int MSG_Q_KEY_FLAG = 0666;

//MSG Queue type for init connection
const int OPEN_REQUEST_CHANNEL = 42;
//only from this client, could use a PID or special task indiactor

int main()
{
    // declare variables
    cpu_set_t cpuMask;
    unsigned cpu, node;

    key_t key = -1;
    int msqid = -1;
    MsgQueue msg;

    pid_t myPid = getpid();

    // generate a unique key. The same parameters to this function will
    // always generate the same value. This is how multiple
    // processes can connect to the same queue.
    key = ftok("./bozo", 0);

    // was the key allocation successful ?
    if (key < 0)
    {
        perror("ftok");
        exit(1);
    }

    // connect to the message queue; fail if the
    // there is no message queue associated with
    // this key. This function returns the id of
    // the queue.
    msqid = msgget(key, MSG_Q_KEY_FLAG);

    // was the allocation a success ?
    if (msqid < 0)
    {
        perror("msgget");
        exit(1);
    }

    // display info to the screen
    cout << "\nSuccessfully connected to server id #" << msqid << " with "
         << "the key " << key
         << "\n\nNow sending messages....";
    //todo: block read msg queue until server use the msg queue to send msg wit type of pid
    //Request to connect message//
    msg.messageType = 42;

    //if Accepted message include pid of client is received from server
    myPid = getpid();
    if (msgsnd(msqid, &msg, sizeof(msg) - sizeof(long), 0) < 0) //if client does not succesfully sent the message
    {
        cout << "Request failed. Client ID is: " << myPid << endl;
        exit(-1);
    }
    else
    {
        msg.mypid = getpid(); //sent succesfully,
        cout << getpid() << endl;
        usleep(10);
        //known error myPid of client != msg.mypid, could be type casting issue
        cout << "MSG sent from current client: " << myPid << endl;
        if (msgrcv(msqid, &msg, sizeof(msg) - sizeof(long), myPid, 0) >= 0) //get approved message from server
        {
            //msg.messageType = myPid; //msg type is now the pid
            cout << "Server approved request!" << endl;
        }
    }
    //msg received: poll task info
    while (1)
    {
        int err = 0;
        // set the message type - this must match
        // the 4th parameter of msgrcv() in the server.cpp code
        //
        // place data into the message queue structure to send to the server
        //pid

        //CPU assignment
        syscall(SYS_getcpu, &cpu, &node, NULL);
        msg.cpu = cpu;
        //Affinity
        sched_getaffinity(0, sizeof(cpu_set_t), &cpuMask);
        msg.affinity = cpuMask.__bits[0];
        //Priority
        //message printout
        //strncpy(msg.buff, "I am client", sizeof(msg.buff));

        msg.priority = getpriority(PRIO_PROCESS, myPid);
        // this is where we send messages:
        //  msqid - the id of the message queue
        //  msg - the message structure which stores the
        //   message to send
        //  sizeof(msg) - sizeof(long) - size of the message
        //   excluding the required first member (messageType) which is
        //   required.
        // 0 - flag values (not useful for this example).
        //       if(msgsnd(msqid, &msg, sizeof(msg) - sizeof(long), 0) < 0)
        if ((err = msgsnd(msqid, &msg, sizeof(msg) - sizeof(long), 0)) < 0)
        {
            perror("msgsnd");
            //exit(1);
        }
        cout << " client 2 sent msg type: " << msg.messageType << endl;
        sleep(3);
    }

    cout << "Sending complete!\n";

    return 0;
}
