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
  rio_t rio;
  Message msg;


  // connect to server
  conn.connect(server_hostname, server_port);
  
  // TODO: send rlogin and join messages (expect a response from
  //       the server for each one)
  msg = Message(TAG_RLOGIN, username);
  conn.send(msg);

  // expect an "ok" message
  Message msg3 = Message(TAG_OK, "You're in!");

  // rlogin messages
  if (conn.receive(msg3)) { 
    // register receiver thread with username
  } else {
    msg = Message(TAG_ERR, "Failed to connect to server");
    Rio_writen(STDERR_FILENO, msg.msg.c_str(), msg.datasize);
    return 1; // Exit with non-zero code
  }

  msg = Message(TAG_JOIN , room_name);
  conn.send(msg);

  // join messages
  if (conn.receive(msg3)) { 
    // register receiver to room
    
  } else {
    msg = Message(TAG_ERR, "Failed to join the room");
    Rio_writen(STDERR_FILENO, msg.msg.c_str(), msg.datasize);
    return 1; // Exit with non-zero code
  }

  std::string currentLine;
  Message msg_loop = Message(TAG_DELIVERY, "");
  
  // TODO: loop waiting for messages from server
  //       (which should be tagged with TAG_DELIVERY)
  while (conn.receive(msg_loop) && msg_loop.tag == TAG_DELIVERY) {
    
    std::string str = msg_loop.msg;
    size_t pos = str.find_last_of(":");
    std::cout << str.substr(pos + 1, str.length());


  }

  return 0;
}
