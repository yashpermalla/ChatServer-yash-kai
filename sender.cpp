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
  // check number of arguments
  if (argc != 4) {
    std::cerr << "Usage: ./sender [server_address] [port] [username]\n";
    return 1;
  }

  // declare variables to hold arguments for connecting
  std::string server_hostname;
  int server_port;
  std::string username;

  server_hostname = argv[1];
  server_port = std::stoi(argv[2]);
  username = argv[3];

  // connect to server
  Connection connection;
  connection.connect(server_hostname, server_port);
  if(!connection.is_open()){
    std::cerr << "Failed to connect to server!";
  }

  // initialize message to err
  Message msg = Message(TAG_ERR, "");

  // send slogin message
  msg.modify("slogin", std::string(argv[3]) + "\n");
  if(!connection.client_server_comm(msg) || msg.tag != TAG_OK){
    //std::cerr << "Failed to login!\n";
    exit(1);
  }

  std::string sendinput;
  std::string command;
  // loop reading commands from user, sending messages to
  // server as appropriate
  while(std::getline(std::cin, sendinput)){

    // joining a room
    if(sendinput.substr(0, 5) == "/join"){
      msg.modify("join", rtrim(sendinput.substr(6)) + "\n");
      connection.client_server_comm(msg);
    } // leave a room
    else if(sendinput.substr(0, 6) == "/leave"){
      msg.modify("leave", "\n");
      connection.client_server_comm(msg);
    } // disconnect from server
    else if(sendinput.substr(0, 6) == "/quit"){
      msg.modify("quit", "\n");
      if(connection.client_server_comm(msg)){
        exit(0);
      }
      else{
        exit(1);
      }
    } // check formatting of command
    else if(sendinput[0] == '/'){
      std::cerr << "Invalid command!\n";
    } // send to all
    else{
      msg.modify("sendall", sendinput + "\n");
      connection.client_server_comm(msg);
    }
  }
  // Successful sender
  return 0;
}