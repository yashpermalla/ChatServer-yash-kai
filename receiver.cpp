#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include "csapp.h"
#include "message.h"
#include "connection.h"
#include "client_util.h"

int main(int argc, char **argv) {
  if (argc != 5) {
    std::cerr << "Usage: ./receiver [server_address] [port] [username] [room]\n";
    return 1;
  }

  std::string server_hostname = argv[1];
  int server_port = std::stoi(argv[2]);
  std::string username = argv[3];
  std::string room_name = argv[4];

  Connection conn;
  Message msg;

  // connect to server
  conn.connect(server_hostname, server_port);
  
  // TODO: send rlogin and join messages (expect a response from
  //       the server for each one)
  msg = Message(TAG_RLOGIN, username);

  // rlogin messages
  if (conn.client_server_comm(msg)) { 
    // register receiver thread with username
  } else {
    std::cerr << msg.msg;
    return 1; // Exit with non-zero code
  }

  msg = Message(TAG_JOIN , room_name);

  // join messages
  if (conn.client_server_comm(msg)) { 
    // register receiver to room
  } else {
    std::cerr << msg.msg;
    return 1; // Exit with non-zero code
  }

  std::string currentLine;
  Message msg_loop = Message(TAG_DELIVERY, "");
  
  // TODO: loop waiting for messages from server
  //       (which should be tagged with TAG_DELIVERY)
  while (conn.receive(msg_loop) && msg_loop.tag == TAG_DELIVERY) {
    
    std::string str = msg_loop.msg;
    size_t pos = str.find_last_of(":");
    std::cout << username << ": " << str.substr(pos + 1);

  }

  return 0;
}