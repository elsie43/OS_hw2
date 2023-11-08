# HW2 Simple Key-value Store

## Directories
- /server ->	server program related sources
- /client ->	client program related sources
- /common ->	common inclusions
- /util ->	common utilization
- /build ->	target build directory

## Building the Project
Code out your `/server/server.c` and `/client/client.c`, then
```shell
$ make
```
Test your `/build/server` and `build/client`.

## Implementations
### Please briefly describe your multi-threading design pattern
When client entered, create new thread, call thread_routine and execute. Use pthread_detach() in thread_routine instead of pthread_join() when the thread exits.
### Please briefly describe your data structure implementation
Use linked list to implement the database. Each node in the linked list contains char* key, char* value, and a pointer to point to the next node.

## References
* [POSIX thread man pages](https://man7.org/linux/man-pages/man7/pthreads.7.html)
* [socket man pages](https://linux.die.net/man/7/socket)

