#include <cstdio>
#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>
#include "csapp.h"
#include "message.h"
#include "connection.h"
#include "client_util.h"

int main(int argc, char **argv) {
  if (argc != 4) {
    std::cerr << "Usage: ./sender [server_address] [port] [username]\n";
    return 1;
  }

  std::string server_hostname;
  int server_port;
  std::string username;

  server_hostname = argv[1];
  server_port = std::stoi(argv[2]);
  username = argv[3];

  // TODO: connect to server

  int fd = open_clientfd(argv[1], argv[2]);

  Connection* connection = new Connection(fd);

  connection->connect(server_hostname, server_port);

  // TODO: send slogin message
  Message* msg = new Message("slogin", argv[3]);
  connection->send(*msg);

  // TODO: loop reading commands from user, sending messages to
  //       server as appropriate

  while(connection->get_last_result() == Connection::SUCCESS){
    
  }
  
  rio_t rio; 
  rio_readinitb(&rio, fd);
  char buf[1000];
  ssize_t n = rio_readlineb(&rio, buf, sizeof(buf));

  while(n > 0){
    Rio_writen(fd, buf, strlen(buf));
    rio_writen(fd, "\n", 1);
    n = rio_readlineb(&rio, buf, sizeof(buf));
  }
  

  return 0;
}
