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
  int fd = Open_clientfd(hostname.c_str(), std::to_string(port).c_str());
  
  // TODO: call rio_readinitb to initialize the rio_t object
  m_fd = fd;
  rio_readinitb(&m_fdbuf, fd);
}

Connection::~Connection() {
  // TODO: close the socket if it is open
  close();
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
  Rio_writen(m_fd, msg.msg.c_str(), msg.datasize);
}

bool Connection::receive(Message &msg) {
  // TODO: receive a message, storing its tag and data in msg
  // return true if successful, false if not
  // make sure that m_last_result is set appropriately
  Rio_readinitb(&m_fdbuf, m_fd);
  //m_fdbuf.rio_buf.find(':');
}
