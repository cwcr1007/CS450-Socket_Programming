/**
 * serverS.cpp
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
#include <fstream>
#include <vector>
#include <map>
#include <set>

#define LOCAL_HOST "127.0.0.1"
#define SERVER_S_PORT 22042
#define CENTRAL_UDP_PORT 24042
#define MAXDATASIZE 1024
#define FILENAME "scores.txt"

using namespace std;

int main(){
  int udp_fd;
  int numbytes;
  char recv_buf[MAXDATASIZE];
  struct sockaddr_in central_udp_addr;
  struct sockaddr_in server_addr;
  socklen_t addr_size = sizeof(struct sockaddr_in);

  //read score file to a map
  vector<string> score;
  string line;
  ifstream myfile(FILENAME);
  if(myfile.is_open()){
    while(getline(myfile, line)){
      score.push_back(line);
    }
    myfile.close();
  }else{
    perror("Error: cannot open file.");
    exit(1);
  }
  // for(int i=0; i<score.size(); i++){
  //   cout << score[i] << endl;
  // }
  
  // establish UDP socket() with central servers
  if((udp_fd = socket(PF_INET, SOCK_DGRAM, 0)) == -1){
    perror("Error: Server S UDP socket()");
    exit(1);
  }
  // initialize socket address
  memset(&server_addr, 0, sizeof server_addr);
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(SERVER_S_PORT);
  server_addr.sin_addr.s_addr = inet_addr(LOCAL_HOST);

  memset(&central_udp_addr, 0, sizeof central_udp_addr);
  central_udp_addr.sin_family = AF_INET;
  central_udp_addr.sin_port = htons(CENTRAL_UDP_PORT);
  central_udp_addr.sin_addr.s_addr = inet_addr(LOCAL_HOST);
  
  // bind() socket and port number
  if(bind(udp_fd, (struct sockaddr*) &server_addr, sizeof server_addr) == -1){
    perror("Error: server S UDP Bind()");
    exit(1);
  }
  printf("The ServerS is up and running using UDP on port %d.\n", SERVER_S_PORT);

  while(1){
    if((numbytes = recvfrom(udp_fd, recv_buf, sizeof recv_buf, 0, (struct sockaddr*)&central_udp_addr, &addr_size)) == -1){
      perror("Error: server S recvfrom().");
      exit(1);
    }
    recv_buf[numbytes] = '\0';
    printf("The ServerS received a request from Central to get the scores.\n");  
    printf("message from central: %s\n", recv_buf);
    
    // Find all the needed scores
    set<string> name;
    char char_array[strlen(recv_buf) + 1];
    strcpy(char_array, recv_buf);
    char delim[] = " ,";
    char *token = strtok(char_array, delim);
    while(token != NULL){
      name.insert(token);
      token = strtok(NULL, delim);
    }
    
    string message = "";
    for(int i=0; i<score.size(); i++){
      char temp[score[i].length() + 1];
      strcpy(temp, score[i].c_str());
      char *ptr = strtok(temp, " ");
      if(name.count(ptr)){
	message += score[i];
	message += ",";
      }
    }
    
    if(sendto(udp_fd, message.c_str(), strlen(message.c_str()), 0, (struct sockaddr*)&central_udp_addr, (socklen_t) sizeof central_udp_addr) == -1){
      perror("Error: serverT sendto()");
      exit(1);
    }
    printf("The ServerS finished sending the scores to Central.\n");
    printf("%s\n", message.c_str());
    
    // Reset all variables used as message communication
    memset(recv_buf, 0, sizeof recv_buf);
  }
  
  close(udp_fd);
}
