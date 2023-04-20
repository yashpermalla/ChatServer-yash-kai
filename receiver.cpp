#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include "csapp.h"
#include "message.h"
#include "connection.h"
#include "client_util.h"

int main(int argc, char **argv) {
  // check number of arguments
  if (argc != 5) {
    std::cerr << "Usage: ./receiver [server_address] [port] [username] [room]\n";
    return 1;
  }

  // declare variables to hold arguments for connecting
  std::string server_hostname = argv[1];
  int server_port = std::stoi(argv[2]);
  std::string username = argv[3];
  std::string room_name = argv[4];

  Connection conn;
  Message msg;

  // connect to server
  conn.connect(server_hostname, server_port);
  if(!conn.is_open()){
    std::cerr << "Failed to connect to server!";
    exit(1);
  }
  
  // send rlogin and join messages (expect a response from
  // the server for each one)

  // rlogin message
  msg = Message(TAG_RLOGIN, username + "\n");
  // check to see if rlogin message is received
  if (!conn.client_server_comm(msg)) { 
    //std::cerr << "Failed to connect!\n";
    return 1; // Exit with non-zero code
  }

  // join message
  msg = Message(TAG_JOIN , room_name + "\n");
  // check to see if join message is received
  if (!conn.client_server_comm(msg)) { 
    //std::cerr << "Failed to join room!\n";
    return 1; // Exit with non-zero code
  }

  // Declare variables to hold user input
  // and message variable
  std::string currentLine;
  Message msg_loop = Message(TAG_ERR, "");
  
  // loop waiting for messages from server
  // (which should be tagged with TAG_DELIVERY)
  while (conn.receive(msg_loop)) {
    
    // Proper tag of TAG_DELIVERY
    if(msg_loop.tag == TAG_DELIVERY){
      // Identify our username index from our msg
      std::string str = msg_loop.msg; 
      size_t pos = str.find_last_of(":");
      // Identify our username index from our msg (cont.)
      std::string sub = str.substr(0, pos);
      size_t pos1 = sub.find_last_of(":");

      // Identify our username from our msg using indices
      std::cout << str.substr(pos1 + 1, pos - pos1 - 1) << ": " << str.substr(pos + 1);
    } // Tag of err but do not exit
    else if(msg_loop.tag == TAG_ERR){
      std::cerr << msg.data;
    }
  }
  // Successful receiver
  return 0;
}