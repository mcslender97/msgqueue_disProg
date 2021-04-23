
//
/*
To compile and run:
 
gcc -g getcpuExample.c -o getcpuExample

//
*/
#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sched.h>
#include <sys/resource.h>
//
// use renice to chnage niceness priotrity
// use taskset to chnage core affinity
//
int main() {
 
    unsigned cpu, node;
    cpu_set_t cpuMask;
    pid_t myPid;   
 
    // Get current CPU core and NUMA node via system call
    // Note this has no glibc wrapper so we must call it directly
    while(1)
    {
    syscall(SYS_getcpu, &cpu, &node, NULL);
    printf("This program is running on CPU core %u \n", cpu);

    sched_getaffinity(0, sizeof(cpu_set_t), &cpuMask); 
    printf("CPU Affinity: 0x%x \n", cpuMask.__bits[0]);   

    myPid = getpid();
    printf("Piroity of Nice: %d\n",getpriority(PRIO_PROCESS, myPid));
    
    sleep(1);
    }
 
    return 0;
 
}
 

