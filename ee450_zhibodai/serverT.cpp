/**
 * serverT.cpp
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
#define SERVER_T_PORT 21042
#define CENTRAL_UDP_PORT 24042
#define MAXDATASIZE 1024
#define FILENAME "edgelist.txt"

using namespace std;

void dfs(map<string,vector<string>>& myMap,vector<vector<string>>& paths,vector<string>& path,string curr,string targ,set<string> visited){
  path.push_back(curr);
  visited.insert(curr);
  if(curr == targ){
    paths.push_back(path);
  }else{
    for(auto i: myMap[curr]){
      bool is_in = visited.find(i) != visited.end();
      if(is_in){
	continue;
      }
      dfs(myMap, paths, path, i, targ, visited);
    }
  }
  path.pop_back();
}

vector<vector<string>> allPath(map<string, vector<string>>& myMap, string user1, string user2){
  vector<vector<string>> paths;
  vector<string> path;
  set<string> visited;
  dfs(myMap, paths, path, user1, user2, visited);
  return paths;
}

int main(){
  int udp_fd;
  int numbytes;
  char recv_buf[MAXDATASIZE];
  struct sockaddr_in central_udp_addr;
  struct sockaddr_in server_addr;
  socklen_t addr_size = sizeof(struct sockaddr_in);

  // read file
  map<string, vector<string>> myMap;
  string line;
  ifstream myfile(FILENAME);
  if(myfile.is_open()){
    while(getline(myfile, line)){
      char char_array[line.length() + 1];
      strcpy(char_array, line.c_str());
      char *token = strtok(char_array, " ");
      string s1 = token;
      token = strtok(NULL, " ");
      string s2 = token;
      if(myMap.count(s1) == 0){
	vector<string> vect;
	myMap.insert(pair<string, vector<string>>(s1, vect));
      }
      myMap[s1].push_back(s2);
      if(myMap.count(s2) == 0){
	vector<string> vect;
	myMap.insert(pair<string, vector<string>>(s2, vect));
      }
      myMap[s2].push_back(s1);
    }
    myfile.close();
  }else{
    perror("Error: cannot open file.");
    exit(1);
  }

  // establish UDP socket() with three backend servers
  if((udp_fd = socket(PF_INET, SOCK_DGRAM, 0)) == -1){
    perror("Error: Server T UDP socket()");
    exit(1);
  }
  // initialize socket address
  memset(&server_addr, 0, sizeof server_addr);
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(SERVER_T_PORT);
  server_addr.sin_addr.s_addr = inet_addr(LOCAL_HOST);

  memset(&central_udp_addr, 0, sizeof central_udp_addr);
  central_udp_addr.sin_family = AF_INET;
  central_udp_addr.sin_port = htons(CENTRAL_UDP_PORT);
  central_udp_addr.sin_addr.s_addr = inet_addr(LOCAL_HOST);
  
  // bind() socket and port number
  if(bind(udp_fd, (struct sockaddr*) &server_addr, sizeof server_addr) == -1){
    perror("Error: server T UDP Bind()");
    exit(1);
  }
  printf("The ServerT is up and running using UDP on port %d.\n", SERVER_T_PORT);

  while(1){
    if((numbytes = recvfrom(udp_fd, recv_buf, sizeof recv_buf, 0, (struct sockaddr*)&central_udp_addr, &addr_size)) == -1){
      perror("Error: server T recvfrom().");
      exit(1);
    }
    recv_buf[numbytes] = '\0';
    printf("The ServerT received a request from Central to get the topology.\n");

    // process data
    string user1;
    string user2;
    char *token = strtok(recv_buf, " ");
    user1 = token;
    token = strtok(NULL, " ");
    user2 = token;
    
    // Use dfs to find all the pathes
    vector<vector<string>> paths;
    paths = allPath(myMap, user1, user2);

    // prepare message to send
    string message = "";
    for(int i=0; i<paths.size(); i++){
      for(int j=0; j<paths[i].size(); j++){
	message += paths[i][j];
	message += " ";
      }
      message += ",";
    }
    
    if((numbytes = sendto(udp_fd, message.c_str(), strlen(message.c_str()), 0, (struct sockaddr*)&central_udp_addr, (socklen_t) sizeof central_udp_addr)) == -1){
      perror("Error: server T sendto()");
      exit(1);
    }
    printf("The ServerT finished sending the topology to Central.\n");
    
    // Reset all variables used as message communication
    memset(recv_buf, 0, sizeof recv_buf);
  }
  
  close(udp_fd);
}
