#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
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
    int mypid;
    int priority;
    int affinity;
    unsigned cpu;

    char buff[100];
};

// message queue flag
const int MSG_Q_KEY_FLAG = 0664;

// message queue type
const int MSG_Q_CHANNEL = 26;
//only from this client, could use a PID or special task indiactor

int main()
{

    // declare variables
    key_t key = -1;
    int msqid = -1;
    MsgQueue msg;

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

    // send 10 messages to the server
    for (int x = 0; x < 10; ++x)
    {
        // set the message type - this must match
        // the 4th parameter of msgrcv() in the server.cpp code
        msg.messageType = MSG_Q_CHANNEL;

        // place data into the message queue structure to send to the server
        //pid
        msg.mypid = (int)getpid();
        //CPU assignment

        strncpy(msg.buff, "I am client1", sizeof(msg.buff));

        // this is where we send messages:
        //  msqid - the id of the message queue
        //  msg - the message structure which stores the
        //   message to send
        //  sizeof(msg) - sizeof(long) - size of the message
        //   excluding the required first member (messageType) which is
        //   required.
        // 0 - flag values (not useful for this example).
        if (msgsnd(msqid, &msg, sizeof(msg) - sizeof(long), 0) < 0)
        {
            perror("msgsnd");
            exit(1);
        }
        sleep(3);
    }

    cout << "Sending complete!\n";

    return 0;
}
