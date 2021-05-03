#include <sys/types.h>

// the structure representing the message queue
// & must match the same layout as in the client.cpp
struct MsgQueue
{
    // IMPORTANT: every message structure must start with this
    long messageType;

    // these variables are optional & you can add
    // more or less if you wish
    // pid of process
    pid_t mypid;
    //priority ( niceness )
    int priority;
    //CPU affinity
    int affinity;
    //current cpu assignment
    unsigned cpu;

    //char buff[1024]; //string type, needs to be of a fixed size
};
