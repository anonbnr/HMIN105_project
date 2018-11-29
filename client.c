#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include "init_IPC.h"
#include "client.h"

/*
****************************
CURRENT PLAN FOR THE CLIENT
****************************

Main thread
===========

1) ???create local WB???
2) Start Reception Thread
3) Start Emission Thread
4) Open socket
5) Connect to server
6) Ask the name of the client and save it in a variable

-- 
I started the threads at the beginning so that,
as soon as the client connects to the server, he can
receive the Welcome message.
--
--
I asked the client for his name in the main thread.
This is to append every message with the client's name
before it's sent to the server
--

Emission Thread
===============
This thread will be in an infinite while loop

.Wait for user input
.Create the message structure
.??Add it to the WB??
.Send it to the server
.Display it on the client's screen if it's sent


Emission Thread
===============
This thread will be in an infinite while loop

.Receive messages from server
.??Add it to the WB??
.display it
*/