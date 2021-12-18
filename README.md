# DistributedLock

Distributed locking is implemented to protect a shared file. The file only contains a counter that can be read and updated by processes. Implemented two operations: acquire and release on a lock variable to protect the file. At the beginning, each process opens the file and tries to update the counter in the file and verifies the update. Thus, the critical section includes the following operations: (1) point the file offset to the counter; (2) update the counter; (3) read and print out the counter value. 

## MutualExclustion
1) Mediator is the central server which allocates the resources to processes
2) Process requests for resource access to the mediator.
3) The syntax for mediator is ./Mediator
4) The syntax for process is ./Process processNumber
5) Mediator has to be started first before starting other processes<br>
Ex :<br>
./Mediator  (In terminal 1)<br>
./Process 1 (In terminal 2)<br>
./Process 2 (In terminal 3)<br>
./Process 3 (In terminal 4)<br>
