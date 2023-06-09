#include <cstdio>
#include <iostream>
#include <sstream>
#include <cctype>
#include <cassert>
#include "csapp.h"
#include "message.h"
#include "connection.h"

Connection::Connection()
  : m_fd(-1)
  , m_last_result(SUCCESS) {
}

Connection::Connection(int fd)
  : m_fd(fd)
  , m_last_result(SUCCESS) {
  // call rio_readinitb to initialize the rio_t object
  rio_readinitb(&m_fdbuf, fd);
}

void Connection::connect(const std::string &hostname, int port) {
  // call open_clientfd to connect to the server
  int fd = open_clientfd(hostname.c_str(), std::to_string(port).c_str());
  // call rio_readinitb to initialize the rio_t object
  m_fd = fd;
  rio_readinitb(&m_fdbuf, fd);
}

Connection::~Connection() {
  // close the socket if it is open
  if(is_open()){
    close();
  }
}

bool Connection::is_open() const {
  // return true if the connection is open
  return (m_fd > 0);
}

void Connection::close() {
  // close the connection if it is open
  Close(m_fd);
}

bool Connection::send(const Message &msg) {
  // send a message
  // return true if successful, false if not
  // make sure that m_last_result is set appropriately
  ssize_t bytes = rio_writen(m_fd, msg.msg.c_str(), msg.datasize);
  // Message is too large
  if(bytes < msg.datasize){
    m_last_result = EOF_OR_ERROR;
    return false;
  }
  // Successful send
  else{
    m_last_result = SUCCESS;
    return true;
  }
}

bool Connection::receive(Message &msg) {
  // receive a message, storing its tag and data in msg
  // return true if successful, false if not
  // make sure that m_last_result is set appropriately
  char userbuf[msg.MAX_LEN + 1];
  ssize_t bytes = rio_readlineb(&m_fdbuf, (void *) userbuf, msg.MAX_LEN);
  if(bytes >= 0) userbuf[bytes] = 0;

  // Must consist of char values
  if(bytes <= 0){
    m_last_result = EOF_OR_ERROR;
    return false;
  }
  else{
    std::string usrbf = std::string(userbuf);
    size_t colonindex = usrbf.find(":");
    // Cannot find colon char value
    if (colonindex == std::string::npos || usrbf.length() > (int) Message::MAX_LEN){
      m_last_result = INVALID_MSG;
      return false;
    }
    // Successful receive
    else{
      msg.modify(usrbf.substr(0, colonindex), usrbf.substr(colonindex+1));
      m_last_result = SUCCESS;
      return true;
    }
  }
}

// Helper method to handle send/receive calls along
// with checking for err handles
bool Connection::client_server_comm(Message &msg){

      // Send message failure
      if(!send(msg)){
        std::cerr << "Failed to " << msg.tag << "!\n";
        return false;
      }
      // Receive message failure
      if(!receive(msg)){
        std::cerr << "Failed to " << "receive from server!\n";
        return false;
      }
      // Message is an error tag
      if(msg.tag == "err"){
        std::cerr << msg.data;
        return false;
      }
      return true;
}