Group members:
Aram Alsihli
Minh Cuong Do
Marc Lama
Mahesh Shrestha

Known issues:
pid from client is different from pid in the message queue
Server would still listen to client 
Explaination: is when Server receive message 42 and retrieve the pid in message queue to set it as the message type id for the return message. The pid of the client and the pid which is logged in the message queue are not the same number

How to run
./server
./client (max 5)
./monitor
Change priority
renice -n -(priority number from -20 to 19)  -p (pid)
renice -n -19 -p 3534

Change cpu assignemnt
taskset -cp (cpu range, or specific cpus) (pid)
taskset -cp 0,2 45678