Assignment1: Multithreaded Chat Room

author: Hui-Jou Chou
last modified date: 2/19/2017

This is a multithreaded chat room implemented in C. There is no need to use makefile,
to compile this program,please go to terminal and direct to this application folder and enter the following commands:
$ gcc multithread_server.c -o mserver
$ gcc gcc multithread_client.c -o mclient

this will compile these two programs separately.

to start the char server, you can simply enter the following command on the terminal:
$ ./mserver

the server use a default ip address and port, and it allows user specify ip address and port as well. Following is an example:
$ ./mserver -i 192.111.1.1 -p 8000

the server support help function, so you can type -h to see the instruction:
$ ./mserver -h

To start the client program, please open another terminal and direct to this app folder and simply enter:
$ ./mclient

there will be an instruction show on terminal and tell you the default ip and port of this server, which ip is 127.0.0.1, port is 6000, and you will have to enter your name as the third argument, so you can type:
$ ./mclient 127.0.0.1 6000 name

This connects to the chat server running on 127.0.0.1, port 6000. you can start to enter your messages.



In server side, for each new connection, the server will create a new thread to accept incoming messages from this client. The chat server maintain a simple chat room with multiple users. There is a maximum of 20 concurrent users. So, the char server can open up to 20 threads concurrently.

In the server code, there is a global variable, which is a linked list used to save all users information. When the program need to use this linked list, pthread_mutex_lock() and pthread_mutex_unlock() will be used to avoid race condition.


In the client side, there are two threads will be created. One is for reading messages and commands from the user and sending them to the server, and another thread is for receiving messages from the server and show to the user.











