/**
 * clientA.cpp
 *
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <iostream>
#include <string>
#include <vector>

#define LOCAL_HOST "127.0.0.1"
#define CENTRAL_A_TCP_PORT 25042
#define MAXDATASIZE 1024

using namespace std;

int main(int argc, char* argv[]){
  int tcpA_fd;    // descriptor number
  int numbytes;
  char recv_buf[MAXDATASIZE];

  struct sockaddr_in central_tcpA_addr;
  struct sockaddr_in my_addr;

  socklen_t addr_size;

  // process the input
  if(argc != 2){
    perror("Error: input value.");
    exit(1);
  }
  char* input = argv[1];
  
  // establish TCP socket() with central server 
  if((tcpA_fd = socket(PF_INET, SOCK_STREAM, 0)) == -1){
    perror("Error: client A TCP socket()");
    exit(1);
  }

  //initialize socket address
  memset(&central_tcpA_addr, 0, sizeof central_tcpA_addr);
  central_tcpA_addr.sin_family = AF_INET;
  central_tcpA_addr.sin_port = htons(CENTRAL_A_TCP_PORT);
  central_tcpA_addr.sin_addr.s_addr = inet_addr(LOCAL_HOST);

  // TCP socket connect()
  if(connect(tcpA_fd, (struct sockaddr*)&central_tcpA_addr, sizeof central_tcpA_addr) == -1){
    perror("Error: client A TCP connect()");
    exit(1);
  }
  
  printf("The client is up and running.\n");

  /*Retrieve the locally-bound name of the specified socket and
    store it in the sockaddr structure*/
  addr_size = sizeof my_addr;
  if(getsockname(tcpA_fd, (struct sockaddr*)&my_addr, (socklen_t*) &addr_size) == -1){
    perror("Error: client A TCP getsockname()");
    exit(1);
  }

  // send()
  if(send(tcpA_fd, input, strlen(input), 0) == -1){
    perror("Error: client A send()");
    exit(1);
  }

  printf("The client sent %s to the Central server.\n", input);
  
  // recv()
  if((numbytes = recv(tcpA_fd, recv_buf, sizeof recv_buf, 0)) == -1){
    perror("Error: client A recv()");
    exit(1);
  }
  recv_buf[numbytes] = '\0';

  // process the result
  string message;
  vector<string> path;
  if(recv_buf[0] != '0'){
    char delim[] = ", ";
    char* token = strtok(recv_buf, delim);
    while(token != NULL){
      path.push_back(token);
      token = strtok(NULL, delim);
    }
    printf("Found compatibility for %s and %s:\n", path[0].c_str(), path[path.size() - 2].c_str());
    for(int i=0; i<path.size() - 1; i++){
      message += path[i];
      if(i < path.size() - 2){
	message += " --- ";
      }
    }
    printf("%s\n", message.c_str());
    double score = atof(path[path.size() - 1].c_str());
    printf("Matching Gap : %.2f\n", score);
    
  }else{
    char delim[] = "0 ";
    char *ptr = strtok(recv_buf, delim);
    string userA = ptr;
    ptr = strtok(NULL, delim);
    string userB = ptr;
    printf("Found no compatibility for %s and %s\n", userA.c_str(), userB.c_str());
  }
  
  //printf("%s\n", recv_buf);
  close(tcpA_fd);
  return 0;
}
 
