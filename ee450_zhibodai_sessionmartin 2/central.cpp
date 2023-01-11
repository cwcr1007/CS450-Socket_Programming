/**
 * central.cpp
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

#define LOCAL_HOST "127.0.0.1"
#define SERVER_T_PORT 21042
#define SERVER_S_PORT 22042
#define SERVER_P_PORT 23042
#define CENTRAL_UDP_PORT 24042
#define CENTRAL_A_TCP_PORT 25042
#define CENTRAL_B_TCP_PORT 26042

#define MAXDATASIZE 1024
#define BACKLOG 10

using namespace std;

void sigchld_handler(int s) {
 // waitpid() might overwrite errno, so we save and restore it:
  int saved_errno = errno;
  
  while(waitpid(-1, NULL, WNOHANG) > 0);
  errno = saved_errno;
} 

int main(){
  int tcpA_fd;    // descriptor number
  int tcpB_fd;
  int udp_fd;
  int newA_fd;    // child socket descripter
  int newB_fd;
  int numbytes;

  char recv_userA[MAXDATASIZE];
  char recv_userB[MAXDATASIZE];
  char recv_graph[MAXDATASIZE];
  char recv_score[MAXDATASIZE];
  char recv_result[MAXDATASIZE];

  struct sockaddr_in central_tcpA_addr;
  struct sockaddr_in central_tcpB_addr;  
  struct sockaddr_in central_udp_addr;
  struct sockaddr_in clientA_addr;
  struct sockaddr_in clientB_addr;
  struct sockaddr_in serverT_addr;
  struct sockaddr_in serverS_addr;
  struct sockaddr_in serverP_addr;

  socklen_t addr_size = sizeof(struct sockaddr_in);
  struct sigaction sa;

  // establish TCP socket() with client A 
  if((tcpA_fd = socket(PF_INET, SOCK_STREAM, 0)) == -1){
    perror("Error: Central server TCP socket with client A");
    exit(1);
  }
  memset(&central_tcpA_addr, 0, sizeof central_tcpA_addr);
  central_tcpA_addr.sin_family = AF_INET;
  central_tcpA_addr.sin_port = htons(CENTRAL_A_TCP_PORT);
  central_tcpA_addr.sin_addr.s_addr = inet_addr(LOCAL_HOST);

  // establish TCP socket() with client B
  if((tcpB_fd = socket(PF_INET, SOCK_STREAM, 0)) == -1){
    perror("Error: Central server TCP socket with client B");
    exit(1);
  }
  memset(&central_tcpB_addr, 0, sizeof central_tcpB_addr);
  central_tcpB_addr.sin_family = AF_INET;
  central_tcpB_addr.sin_port = htons(CENTRAL_B_TCP_PORT);
  central_tcpB_addr.sin_addr.s_addr = inet_addr(LOCAL_HOST);

  // establish UDP socket() with three backend servers
  if((udp_fd = socket(PF_INET, SOCK_DGRAM, 0)) == -1){
    perror("Error: Central server UDP socket");
    exit(1);
  }
  memset(&central_udp_addr, 0, sizeof central_udp_addr);
  central_udp_addr.sin_family = AF_INET;
  central_udp_addr.sin_port = htons(CENTRAL_UDP_PORT);
  central_udp_addr.sin_addr.s_addr = inet_addr(LOCAL_HOST);

  memset(&serverT_addr, 0, sizeof serverT_addr);
  serverT_addr.sin_family = AF_INET;
  serverT_addr.sin_port = htons(SERVER_T_PORT);
  serverT_addr.sin_addr.s_addr = inet_addr(LOCAL_HOST);

  memset(&serverS_addr, 0, sizeof serverS_addr);
  serverS_addr.sin_family = AF_INET;
  serverS_addr.sin_port = htons(SERVER_S_PORT);
  serverS_addr.sin_addr.s_addr = inet_addr(LOCAL_HOST);

  memset(&serverP_addr, 0, sizeof serverP_addr);
  serverP_addr.sin_family = AF_INET;
  serverP_addr.sin_port = htons(SERVER_P_PORT);
  serverP_addr.sin_addr.s_addr = inet_addr(LOCAL_HOST);
  
  // bind() socket and port number
  if(bind(tcpA_fd, (struct sockaddr*) &central_tcpA_addr, sizeof central_tcpA_addr) == -1){
    perror("Error: central TCP Bind() with client A");
    exit(1);
  }
  if(bind(tcpB_fd, (struct sockaddr*) &central_tcpB_addr, sizeof central_tcpB_addr) == -1){
    perror("Error: central TCP Bind() with client B");
    exit(1);
  }
  if(bind(udp_fd, (struct sockaddr*) &central_udp_addr, sizeof central_udp_addr) == -1){
    perror("Error: central UDP Bind()");
    exit(1);
  }

  // TCP socket listen() from client A & B
  if(listen(tcpA_fd, BACKLOG) == -1){
    perror("Error: central TCP Listen() from client A");
    exit(1);
  }
  if(listen(tcpB_fd, BACKLOG) == -1){
    perror("Error: central TCP Listen() from client B");
    exit(1);
  }
  printf("The Central server is up and running.\n");

  // reap all dead processes sigemptyset(&sa.sa_mask);
  sa.sa_handler = sigchld_handler; 
  sa.sa_flags = SA_RESTART;
  if (sigaction(SIGCHLD, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }
  
  while(1){
    // TCP connection with client A
    addr_size = sizeof clientA_addr;
    newA_fd = accept(tcpA_fd, (struct sockaddr*)&clientA_addr, &addr_size);
    if(newB_fd == -1){
      perror("Error: central TCP accept A");
      exit(1);
    }
    // TCP: Receives user name from client A
    if((numbytes = recv(newA_fd, recv_userA, sizeof recv_userA, 0)) == -1){
      perror("Error: central recv() from client A");
      exit(1);
    }
    recv_userA[numbytes] = '\0';
    printf("The Central server received input=\"%s\" from the client using TCP over port %d.\n", recv_userA, CENTRAL_A_TCP_PORT);
    
    // TCP connection with client B
    addr_size = sizeof clientB_addr;
    newB_fd = accept(tcpB_fd, (struct sockaddr*)&clientB_addr, &addr_size);
    if(newB_fd == -1){
      perror("Error: central TCP accept B");
      exit(1);
    }
    // TCP: Receives user name from client B
    if((numbytes = recv(newB_fd, recv_userB, sizeof recv_userB, 0)) == -1){
      perror("Error: central recv() from client B");
      exit(1);
    }
    recv_userB[numbytes] = '\0';
    printf("The Central server received input=\"%s\" from the client using TCP over port %d.\n", recv_userB, CENTRAL_B_TCP_PORT);
    
    // sendto and recvfrom with server T
    string userA(recv_userA);
    string userB(recv_userB);
    string messageT = userA + " " + userB;
    if(sendto(udp_fd, messageT.c_str(), strlen(messageT.c_str()), 0, (struct sockaddr*)&serverT_addr, (socklen_t) sizeof serverT_addr) == -1){
      perror("Error: central server sendto() server T");
      exit(1);
    }
    printf("The Central server sent a request to Backend-Server T.\n");
    if((numbytes = recvfrom(udp_fd, recv_graph, sizeof recv_graph, 0, (struct sockaddr*)&serverT_addr, &addr_size)) == -1){
      perror("Error: central server recvfrom() server T");
      exit(1);
    }
    printf("numbytes: %d\n", numbytes);
    recv_graph[numbytes] = '\0';
    printf("The Central server received information from Backend-Server T using UDP over port %d.\n", CENTRAL_UDP_PORT);
    printf("graph: %s\n", recv_graph);
    
    // sendto and recvfrom with server S
    if(sendto(udp_fd, recv_graph, strlen(recv_graph), 0, (struct sockaddr*)&serverS_addr, (socklen_t) sizeof serverS_addr) == -1){
      perror("Error: central server sendto() server S");
      exit(1);
    }
    printf("The Central server sent a request to Backend-Server S.\n");
    if((numbytes = recvfrom(udp_fd, recv_score, sizeof recv_score, 0, (struct sockaddr*)&serverS_addr, &addr_size)) == -1){
      perror("Error: central server recvfrom() server S");
      exit(1);
    }
    recv_score[numbytes] = '\0';
    printf("The Central server received information from Backend-Server S using UDP over port %d.\n", CENTRAL_UDP_PORT);
    printf("score: %s\n", recv_score);

    //  sendto and recvfrom with server P
    if(sendto(udp_fd, recv_graph, strlen(recv_graph), 0, (struct sockaddr*)&serverP_addr, (socklen_t) sizeof serverP_addr) == -1){
      perror("Error: central server sendto() server P");
      exit(1);
    }
    if(sendto(udp_fd, recv_score, strlen(recv_score), 0, (struct sockaddr*)&serverP_addr, (socklen_t) sizeof serverP_addr) == -1){
      perror("Error: central server sendto() server P");
      exit(1);
    }
    printf("The Central server sent a request to Backend-Server P.\n");
    if((numbytes = recvfrom(udp_fd, recv_result, sizeof recv_result, 0, (struct sockaddr*)&serverP_addr, &addr_size)) == -1){
      perror("Error: central server recvfrom() server P");
      exit(1);
    }
    recv_result[numbytes] = '\0';
    printf("The Central server received the results from backend server P.\n");
    printf("result: %s\n", recv_result);

    // process the result
    string message;
    if(numbytes == 0){
      message = "0" + userA + " " + userB; 
    }else{
      message = recv_result;
    }
    
    // send result to client A
    if(send(newA_fd, message.c_str(), strlen(message.c_str()), 0) == -1){
      perror("Error: central send() client A");
      exit(1);
    }
    printf("The Central server sent the results to client A.\n");
    
    // send result to client B
    if(send(newB_fd, message.c_str(), strlen(message.c_str()), 0) == -1){
      perror("Error: central send() client B");
      exit(1);
    }
    printf("The Central server sent the results to client B.\n");

    // reset all receive buffer to 0
    memset(recv_userA, 0, sizeof recv_userA);
    memset(recv_userB, 0, sizeof recv_userB);
    memset(recv_graph, 0, sizeof recv_graph);
    memset(recv_score, 0, sizeof recv_score);
    memset(recv_result, 0, sizeof recv_result);
    
    close(newA_fd);
    close(newB_fd);
  }
  close(udp_fd);
  close(tcpA_fd);
  close(tcpB_fd);
  return 0;
}
