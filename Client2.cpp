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
using namespace std;

// Compile & Run
// g++ Client.cpp -o Client
// ./Client

// the structure representing the message queue
// & must match the same layout as in the server.cpp
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
const int MSG_Q_KEY_FLAG = 0664;

// message queue type
const int MSG_Q_CHANNEL = 27;
//Queue type for init connection
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
    int mypid = getpid();
    pid_t myPid;

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

    //msg received: poll task info
    while (1)
    {
        int err = 0;
        // set the message type - this must match
        // the 4th parameter of msgrcv() in the server.cpp code

        //Request to connect message//
        //msg.messageType = 42;

        //if Accepted message include pid of client is received from server
        msg.messageType = mypid;

        // place data into the message queue structure to send to the server
        //pid
        msg.mypid = (int)getpid();
        //CPU assignment
        syscall(SYS_getcpu, &cpu, &node, NULL);
        msg.cpu = cpu;
        //Affinity
        sched_getaffinity(0, sizeof(cpu_set_t), &cpuMask);
        msg.affinity = cpuMask.__bits[0];
        //Priority

        //message printout
        strncpy(msg.buff, "I am client2", sizeof(msg.buff));
        myPid = getpid();
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
        if ((err = msgsnd(msqid, &msg, sizeof(msg) - sizeof(long), IPC_NOWAIT)) < 0)
        {
            perror("msgsnd");
            //exit(1);
        }
        cout << " clirent 2 sent msg: " << err << endl;
        sleep(3);
    }

    cout << "Sending complete!\n";

    return 0;
}
