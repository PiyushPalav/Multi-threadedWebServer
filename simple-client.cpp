/* run client using: ./client localhost <server_port> */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <netdb.h>

#include <sys/socket.h>
#include <signal.h>
#include <errno.h>

#define BUFFER_SIZE 4096

int sockfd;

void error(const char *msg) {
  perror(msg);
  exit(1);
}

void signal_handler(int sig) {
  if(sig == SIGINT){
    if (send(sockfd, "exit", 4, 0) < 0)
        error("ERROR writing to socket");
    close(sockfd);
    exit(1);
  }
}

void init_signal_handler(){
	struct sigaction sa;
	sa.sa_handler = &signal_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;

	if (sigaction(SIGINT, &sa, 0) == -1) {
		perror("Unable to change signal action for sigint");
		exit(EXIT_SUCCESS);
	}
}

int main(int argc, char *argv[]) {
  //int sockfd, portno, n;
  int portno, n;

  struct sockaddr_in serv_addr;
  struct hostent *server;

  char buffer[BUFFER_SIZE];
  init_signal_handler();

  if (argc < 3) {
    fprintf(stderr, "usage %s hostname port\n", argv[0]);
    exit(0);
  }

  /* create socket, get sockfd handle */

  portno = atoi(argv[2]);
  sockfd = socket(PF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
    error("ERROR opening socket");

  /* fill in server address in sockaddr_in datastructure */

  server = gethostbyname(argv[1]);
  if (server == NULL) {
    fprintf(stderr, "ERROR, no such host\n");
    exit(0);
  }
  bzero((char *)&serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr_list[0], (char *)&serv_addr.sin_addr.s_addr,
        server->h_length);
  serv_addr.sin_port = htons(portno);

  /* connect to server */

  if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    error("ERROR connecting");

  /* ask user for input */

  bzero(buffer, BUFFER_SIZE);
  fgets(buffer, BUFFER_SIZE-1, stdin);
  
  /* send user message to server */

  n = send(sockfd, buffer, strlen(buffer), 0);
  if (n < 0){
      close(sockfd);
      error("ERROR writing to socket");
  }

  if (strncmp(buffer,"exit",4) == 0){
    close(sockfd);
    exit(1);
  }
  
  bzero(buffer, BUFFER_SIZE);

  /* read reply from server */

  n = recv(sockfd, buffer, BUFFER_SIZE-1, 0);
  if (n < 0){
      close(sockfd);
      error("ERROR reading from socket");
  }
  printf("%s", buffer);
  close(sockfd);
  return 0;
}
