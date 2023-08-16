/* run using ./server <port> */
#include "http_server.h"
#include "myqueue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <pthread.h>

#define BUFFER_SIZE 4096
#define THREAD_POOL_SIZE 20

pthread_t thread_pool[THREAD_POOL_SIZE];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition_var = PTHREAD_COND_INITIALIZER;

void error(const char *msg) {
  perror(msg);
  exit(1);
}

void *connection_handler(void *newsockfd) {
  int client_sockfd = *((int *) newsockfd);
  free(newsockfd);

  char buffer[BUFFER_SIZE];
  int n;

  bzero(buffer, BUFFER_SIZE);
  n = recv(client_sockfd, buffer, BUFFER_SIZE-1, 0);
  if (n < 0)
      error("ERROR reading from socket");

  if (strncmp(buffer,"exit",4) == 0){
    close(client_sockfd);
    return 0;
  }
  printf("\nRequest : \n");
  printf("%s", buffer);

  HTTP_Response *server_response = handle_request(buffer);

  cout << endl;
  cout << "Response :" << endl;
  cout << server_response->get_string() << endl;

  string response_string = server_response->get_string();

  /* send reply to client */

  n = send(client_sockfd, response_string.c_str(), strlen(response_string.c_str()), 0);
  if (n < 0)
      error("ERROR writing to socket");

  close(client_sockfd);
  return 0;
}

void *thread_function(void *arg) {
  while (true) {
    pthread_mutex_lock(&mutex);
    pthread_cond_wait(&condition_var, &mutex);
    int* client_socket = dequeue();
    pthread_mutex_unlock(&mutex);
    if (client_socket != NULL) {
      connection_handler(client_socket);
    }
  }
}

int main(int argc, char *argv[]) {
  int sockfd, newsockfd, portno;
  socklen_t clilen;
  struct sockaddr_in serv_addr, cli_addr;
  // pthread_t thread_id;

  if (argc < 2) {
    fprintf(stderr, "ERROR, no port provided\n");
    exit(1);
  }

  /* create worker threads*/
  for (int i = 0; i < THREAD_POOL_SIZE; i++) {
    pthread_create(&thread_pool[i], NULL, &thread_function, NULL);
  }

  /* create socket */

  sockfd = socket(PF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
    error("ERROR opening socket");

  /* fill in port number to listen on. IP address can be anything (INADDR_ANY)
   */

  bzero((char *)&serv_addr, sizeof(serv_addr));
  portno = atoi(argv[1]);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);

  /* bind socket to this port number on this machine */

  if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    error("ERROR on binding");

  /* listen for incoming connection requests */

  listen(sockfd, 5);
  clilen = sizeof(cli_addr);

  while (1)
  {
    /* accept a new request, create a newsockfd */

    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
    if (newsockfd < 0)
      error("ERROR on accept");

    int *client_socket = (int *)malloc(sizeof(int));
    *client_socket = newsockfd;
    pthread_mutex_lock(&mutex);
    enqueue(client_socket);
    pthread_cond_signal(&condition_var);
    pthread_mutex_unlock(&mutex);

    // if(pthread_create(&thread_id, NULL, connection_handler, &newsockfd) < 0)
    //   error("Could not create thread");
  }
  close(sockfd);
  return 0;
}