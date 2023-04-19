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
  Connection connection;
  connection.connect(server_hostname, server_port);

  // expect an "ok" message
  Message msg = Message(TAG_ERR, "");

  // TODO: send slogin message
  msg.modify("slogin", std::string(argv[3]) + "\n");
  if(!connection.client_server_comm(msg) || msg.tag != TAG_OK){
    std::cerr << "Failed to login!\n";
    exit(1);
  }

  // TODO: loop reading commands from user, sending messages to
  //       server as appropriate

  std::string sendinput;

  //std::stringstream ss;

  std::string command;

  while(std::getline(std::cin, sendinput)){

    if(sendinput.substr(0, 5) == "/join"){
      msg.modify("join", rtrim(sendinput.substr(6)) + "\n");
      connection.client_server_comm(msg);
    }
    else if(sendinput.substr(0, 6) == "/leave"){
      msg.modify("leave", "\n");
      connection.client_server_comm(msg);
    }
    else if(sendinput.substr(0, 6) == "/quit"){
      msg.modify("quit", "\n");
      if(connection.client_server_comm(msg)){
        exit(0);
      }
      else{
        exit(1);
      }
    }
    else if(sendinput[0] == '/'){
      std::cerr << "Invalid command!\n";
    }
    else{
      msg.modify("sendall", sendinput + "\n");
      connection.client_server_comm(msg);
    }

  }

  return 0;
}