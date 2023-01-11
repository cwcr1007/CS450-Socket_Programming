/**
 * serverP.cpp
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
#include <vector>
#include <map>
#include <set>
#include <math.h>
#include<limits>

#define LOCAL_HOST "127.0.0.1"
#define SERVER_P_PORT 23042
#define CENTRAL_UDP_PORT 24042
#define MAXDATASIZE 1024

using namespace std;

//void readTopology();

int main(){
  int udp_fd;
  int numbytes;
  char bufferT[MAXDATASIZE];
  char bufferS[MAXDATASIZE];
  struct sockaddr_in central_udp_addr;
  struct sockaddr_in server_addr;
  socklen_t addr_size = sizeof(struct sockaddr_in);

  // establish UDP socket() with three backend servers
  if((udp_fd = socket(PF_INET, SOCK_DGRAM, 0)) == -1){
    perror("Error: Server P UDP socket()");
    exit(1);
  }
  // initialize socket address
  memset(&server_addr, 0, sizeof server_addr);
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(SERVER_P_PORT);
  server_addr.sin_addr.s_addr = inet_addr(LOCAL_HOST);

  memset(&central_udp_addr, 0, sizeof central_udp_addr);
  central_udp_addr.sin_family = AF_INET;
  central_udp_addr.sin_port = htons(CENTRAL_UDP_PORT);
  central_udp_addr.sin_addr.s_addr = inet_addr(LOCAL_HOST);
  
  // bind() socket and port number
  if(bind(udp_fd, (struct sockaddr*) &server_addr, sizeof server_addr) == -1){
    perror("Error: server P UDP Bind()");
    exit(1);
  }
  printf("The ServerP is up and running using UDP on port %d.\n", SERVER_P_PORT);

  while(1){
    // recvfrom() central
    if((numbytes = recvfrom(udp_fd, bufferT, sizeof bufferT, 0, (struct sockaddr*)&central_udp_addr, &addr_size)) == -1){
      perror("Error: server S recvfrom().");
      exit(1);
    }
    bufferT[numbytes] = '\0';

    if((numbytes = recvfrom(udp_fd, bufferS, sizeof bufferS, 0, (struct sockaddr*)&server_addr, &addr_size)) == -1){
      perror("Error: server S recvfrom().");
      exit(1);
    }
    bufferS[numbytes] = '\0';
    printf("The ServerP received the topology and score information.\n");
    printf("graph: %s\n", bufferT);
    printf("score: %s\n", bufferS);

    // condition on if there is a match
    string message;
    if(numbytes != 0){    // process the graph
      char delim[] = " ,";
      vector<vector<string>> paths;
      vector<string> path;
      char *token = strtok(bufferT, delim);
      string src = token;
      while(token != NULL){
	if(token == src && !path.empty()){
	  paths.push_back(path);
	  path.clear();
	}
	path.push_back(token);
	token = strtok(NULL, delim);
      }
      if(!path.empty()){
	paths.push_back(path);
      }

      // process the scores
      map<string, int> scores;
      char *tok = strtok(bufferS, delim);
      int idx = 0;
      string key;
      string val;
      while(tok != NULL){
	if(idx % 2 != 0){
	  val = tok;
	  scores.insert(pair<string, int>(key, stoi(val)));
	}else{
	  key = tok;
	}
	tok = strtok(NULL, delim);
	idx += 1;
      }
      // calculate the matching gap
      double min = numeric_limits<double>::infinity();
      double sum = 0.0;
      int first;
      int second;
      int shortest_idx;
      for(int i=0; i<paths.size(); i++){
	sum = 0;
	for(int j=0; j<paths[i].size()-1; j++){
	  first = scores[paths[i][j]];
	  second = scores[paths[i][j+1]];
	  sum += double(abs(first - second)) / (first + second);
	}
	if(sum < min){
	  min = sum;
	  shortest_idx = i;
	}
      }
      for(int i=0; i<paths[shortest_idx].size(); i++){
	message += paths[shortest_idx][i];
	if(i < paths[shortest_idx].size() - 1){
	  message += " ";
	}
      }
      message += ",";
      message += to_string(min);
      printf("%s\n", message.c_str());
    }
    
    if(sendto(udp_fd, message.c_str(), strlen(message.c_str()), 0, (struct sockaddr*)&central_udp_addr, (socklen_t) sizeof central_udp_addr) == -1){
      perror("Error: serverT sendto()");
      exit(1);
    }
    printf("The ServerP finished sending the results to Central.\n");
    
    // Reset all variables used as message communication
    memset(bufferT, 0, sizeof bufferT);
    memset(bufferS, 0, sizeof bufferS);
  }
  close(udp_fd);
}


