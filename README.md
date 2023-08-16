# Multi-threadedWebServer

## Multi-threaded server

C/C++ client-server programs communicate with each other using the **socket** API.

The server program takes one command line argument: the **port number** on which to listen. The client program takes two arguments: the server hostname and port number. You can give localhost as the hostname if you are running the client and server on the same machine. Once the client and server are connected to each other, the client sends a HTTP request to the server and gets an HTTP response back.

Once the server accepts a new connection, it will create a new worker thread, and it will pass the client's socket file descriptor (returned by accept) to this new thread. This thread will then read and write messages from/to the client.

**Pthread library available in C/C++** is used to create threads. This document has a detailed explanation of the pthreads library and its functions: [introduction to Pthread API](https://pages.cs.wisc.edu/~remzi/OSTEP/threads-api.pdf)

Pthreads library is included with the header file:

`#include <pthread.h>`

`-lpthread` flag is used to compile the code.

The pthread library has several useful functions. You can create a thread using the pthread_create() function present in this library. When you create a worker thread to handle a client request at your server, you must pass the accepted client file descriptor as an argument to the thread function, so that it can read and write from the assigned client. Here is sample code that creates a thread and passes it an argument:

```C
 void *start_function(void *arg) {
   int my_arg = *((int *) arg);
   // ...thread processing...
 }

 int main(int argc, char *argv[]) {
   int thread_arg = 0;
   pthread_t thread_id;

   pthread_create(&thread_id, NULL, start_function, &thread_arg);

   // ...more code...
 }
```

The created thread will focus on communicating with the client given to it as an argument at creation time, while the main server thread can go back to accepting new connections. In this way, server can communicate with multiple clients at the same time.

## HTTP server specification

HTTP web server parses the data received from the client and constructs a HTTP request. It then generates a suitable HTTP response and sends it back to the client over the socket. Below are some specifications that the HTTP server handles.

1. It handles only **HTTP GET** requests for simple HTML files for now. The URL specified in the HTTP GET request is read from the local filesystem and returned in the HTTP response. A URL can resemble the path of a directory, e.g., `/dir_a/sub_dir_b/`, or that of a file, e.g., `/dir_a/sub_dir_b/index.html`. If the URL is that of a directory, it looks for an `index.html` file in that directory and serves that. User defined directory in code is used as starting root folder for HTML files.

2. The communication between the client and the server is through TCP protocol. In case of insufficient root permissions on machine to open a socket on port 80, a higher port number like 8080 can be used for the server to listen on. It supports HTTP 1.0 for now, in which the server closes the TCP connection immediately after responding to the client’s request.

3. The HTTP response returned by the server returns the status code of 200 in its initial line if the requested file is found and can be successfully returned. The server returns a response with status code 404 if the requested resource cannot be found. When returning the 404 error code, this error message is wrapped in simple HTML headers for the browser to display it correctly.

The HTTP server reads data from the socket, parses the received data to form a HTTP request structure from it, generatse a suitable HTTP response structure, converts this response back into a string, and writes it into the socket. The file http_server.h defines data structures to store the HTTP request and response at the server. The worker thread of server that is handling the client reads data from the client socket, creates a HTTP request object from it, calls the function `handle_request` to generate a HTTP response, calls the function `get_string` on this object to convert the response to a string, and writes it into the client socket before closing the connection.

## Enhancements to HTTP server - Worker thread pool

Till now, web server was handling multiple clients by creating a separate thread for each client. However, thread creation is a high overhead task. Therefore, real-life multi-threaded servers use a **pool of reusable worker threads** instead. The main server creates a pool of worker threads at the start. Whenever a new client connection is accepted, the server places the accepted client file descriptor in a queue/array shared with the workers. Each worker thread fetches a client from this queue, and serves it as long as the client is connected. Once the client finishes and terminates, the worker thread goes back to the queue to get the next client to handle. In this way, the same pool of worker threads can serve multiple clients.

To design a thread pool, given steps are followed:

* First, multiple threads are created depending on the requirement and server processing power.

* Next, a shared buffer or queue is created to store the accepted client file descriptors.

* The main server thread and the worker threads uses **locks (mutexes)** to access this queue without race conditions.

* The main server thread and worker threads use **condition variables** to signal each other when clients are added or removed from the queue.

* Once a worker thread dequeues a client socket file descriptor from the shared queue, the rest of the handling of the client request is done same as before. Locks that the thread is holding are released before performing operations like disk reads for files: otherwise, the other threads may stall for the lock. 

After these changes, the server is able to handle multiple clients concurrently as before, but without having to create a new thread for every client.
