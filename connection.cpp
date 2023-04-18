#include <cstdio>
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
  // TODO: call rio_readinitb to initialize the rio_t object
  rio_readinitb(&m_fdbuf, fd);
  
}

void Connection::connect(const std::string &hostname, int port) {
  // TODO: call open_clientfd to connect to the server
  int fd = open_clientfd(hostname.c_str(), std::to_string(port).c_str());
  
  // TODO: call rio_readinitb to initialize the rio_t object
  m_fd = fd;
  rio_readinitb(&m_fdbuf, fd);
}

Connection::~Connection() {
  // TODO: close the socket if it is open
  if(is_open()){
    close();
  }
  
}

bool Connection::is_open() const {
  // TODO: return true if the connection is open
  return (m_fd > -1);
}

void Connection::close() {
  // TODO: close the connection if it is open
  Close(m_fd);
  m_fd = -1;
}

bool Connection::send(const Message &msg) {
  // TODO: send a message
  // return true if successful, false if not
  // make sure that m_last_result is set appropriately
  ssize_t bytes = rio_writen(m_fd, msg.msg.c_str(), msg.datasize);
  if(bytes < msg.datasize){
    m_last_result = EOF_OR_ERROR;
    return false;
  }
  else{
    m_last_result = SUCCESS;
    return true;
  }
  
}

bool Connection::receive(Message &msg) {
  // TODO: receive a message, storing its tag and data in msg
  // return true if successful, false if not
  // make sure that m_last_result is set appropriately
  char userbuf[msg.MAX_LEN + 1];
  ssize_t bytes = rio_readnb(&m_fdbuf, (void *) userbuf, msg.datasize);
  userbuf[msg.datasize] = 0;

  if(bytes < msg.datasize){
    m_last_result = EOF_OR_ERROR;
    return false;
  }
  else{
    std::string usrbf = std::string(userbuf);
    size_t colonindex = usrbf.find(':');
    if (colonindex == std::string::npos){
      m_last_result = INVALID_MSG;
      return false;
    }
    
  }

  //less than 256
  //

  //m_fdbuf.rio_buf.find(':');
}
