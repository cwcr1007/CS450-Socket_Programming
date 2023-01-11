a. Full Name: Zhibo Dai

b. Student ID: 4893231042

c. What I have done: 
   • Implemented a social matching application in C++ on Ubuntu, where clients can issue a request to get matching gap between other user if there is a compatibility.
   • Designed central server to handle the requests and interacts with three backend servers (database) for retrieving related data and data processing; Utilized backend servers to store the social network graph and compatibility test scores, and to calculate a social network path that has smallest matching gap.
   • Established TCP stream socket between clients and central servers, and UDP datagram socket between servers and backend servers to transfer data, thereby supporting communication between programs.

d. Code files and their functions:
   • clientA.cpp: user interface, where userA can send his/her name and send name throught TCP connection with central.
   • clientB.cpp: user interface, where userB can send his/her name and send name throught TCP connection with central.
   • central.cpp: receive names from both client over a TCP connection, and send request and receive information from  the Back-Servers T, S & P over a UDP connection, and finally send result to clients over TCP.
   • serverT.cpp: receive request from central server through UDP connection and find all the paths from userA to userB in the given graph using depth first search and send it back to central server.
   • serverS.cpp: receive request from central server through UDP connection and find all the scores of related names and send it back to central server.
   • serverP.cpp: receive request from central server through UDP connection and find all the shortest path using given paths and their scores and send it back to central server.
   
e. The format of all the messages exchanged between client/central/backend servers is char array. I convert char array (message) to local string to process and then get results in each phase, and convert string to char array back while communicating between client/central/backend servers.

g. Any idiosyncrasy of my project: if the graph is too large, the project might fail due to run time complexity.

h. Resused Code: I referenced Beej Guide Network Programming to write socket(), bind(), listen(), accept(), send(), recv(), sendto(), recvfrom(), sigchld_handler().
