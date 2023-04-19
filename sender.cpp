#include <cstdio>
#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>
#include "csapp.h"
#include "message.h"
#include "connection.h"
#include "client_util.h"



bool sender_server(Connection &connection, ){
  
}



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

  if(!connection.receive(msg) || msg.tag != TAG_OK){
    std::cerr << "Failed to connect!";
    exit(1);
  }

  // TODO: send slogin message
  msg.modify("slogin", argv[3]);
  connection.send(msg);
  if(connection.get_last_result() != Connection::SUCCESS){
    std::cerr << "Failed to login!";
    exit(1);
  }
  connection.receive(msg);
  if(connection.get_last_result() != Connection::SUCCESS){
    std::cerr << "Failed to login!";
    exit(1);
  }
  if(msg.tag == "err"){
    std::cerr << msg.data;
    exit(1);
  }



  // TODO: loop reading commands from user, sending messages to
  //       server as appropriate

  std::string sendinput;

  //std::stringstream ss;

  std::string command;

  while(std::getline(std::cin, sendinput)){

    if(sendinput.substr(0, 5) == "/join"){
      msg.modify("join", rtrim(sendinput.substr(6)));
      connection.client_server_comm(msg);
    }
    else if(sendinput.substr(0, 6) == "/leave"){
      msg.modify("leave", "");
      connection.client_server_comm(msg);
    }
    else if(sendinput.substr(0, 6) == "/quit"){
      msg.modify("quit", "");
      if(connection.client_server_comm(msg)){
        exit(0);
      }
      else{
        exit(1);
      }
    }
    else if(sendinput[0] == '/'){
      std::cerr << "Invalid command!";
    }
    else{
      msg.modify("sendall", sendinput);
      connection.client_server_comm(msg);
    }

  }

  return 0;
}
