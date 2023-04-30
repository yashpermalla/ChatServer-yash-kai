#include <pthread.h>
#include <iostream>
#include <sstream>
#include <memory>
#include <set>
#include <vector>
#include <cctype>
#include <cassert>
#include "message.h"
#include "connection.h"
#include "user.h"
#include "room.h"
#include "guard.h"
#include "server.h"

////////////////////////////////////////////////////////////////////////
// Server implementation data types
////////////////////////////////////////////////////////////////////////

// add any additional data types that might be helpful
// for implementing the Server member functions
struct server_help {

  Connection* connect;
  Server* server;

};

////////////////////////////////////////////////////////////////////////
// Client thread functions
////////////////////////////////////////////////////////////////////////

namespace {

void sender_comm(Connection* cn, Server* server, std::string username){
  
  Message msg;
  std::string room = "";
  while(cn->receive(msg)){
    
    if(msg.tag == TAG_JOIN){
      room = msg.data;
      server->find_or_create_room(msg.data);
      msg.modify(TAG_OK, "joined\n");
      cn->send(msg);
    }
    else if(msg.tag == TAG_LEAVE){
      if(room == ""){
        msg.modify(TAG_ERR, "You are not in a room!\n");
        cn->send(msg);
      }
      else{
        room = "";
        msg.modify(TAG_OK, "Left room!\n");
        cn->send(msg);
      }
    }
    else if(msg.tag == TAG_QUIT){
      room = "";
      msg.modify(TAG_OK, "Quit user\n");
      cn->send(msg);
      break;
    }
    else if(msg.tag == TAG_SENDALL){
      if(room == ""){
        msg.modify(TAG_ERR, "You are not in a room!\n");
        cn->send(msg);
      }
      else{
        server->find_or_create_room(room)->broadcast_message(username, msg.data);
        msg.modify(TAG_OK, "Sent!\n");
        cn->send(msg);
      }
    }
  }

}

void receiver_comm(Connection* cn, User* user, std::string room){

  while (1) {
    Message* msg = user->mqueue.dequeue();
    if (msg != nullptr) {
      bool sent = cn->send(*msg);

      delete msg;
      
      if (!sent) {

        while(!user->mqueue.is_empty()){
          delete user->mqueue.dequeue();
        }

        break;
      }
    }
  }
  
}

void *worker(void *arg) {
  pthread_detach(pthread_self());

  // TODO: use a static cast to convert arg from a void* to
  //       whatever pointer type describes the object(s) needed
  //       to communicate with a client (sender or receiver)
  server_help* new_arg = (server_help*) arg;

  // TODO: read login message (should be tagged either with
  //       TAG_SLOGIN or TAG_RLOGIN), send response
  Message read;
  new_arg->connect->receive(read);

  bool sender = (read.tag == TAG_SLOGIN);

  read.modify(TAG_OK, "logged in\n");
  new_arg->connect->send(read);
  
  // TODO: depending on whether the client logged in as a sender or
  //       receiver, communicate with the client (implementing
  //       separate helper functions for each of these possibilities
  //       is a good idea)
  if (sender) {

    sender_comm(new_arg->connect, new_arg->server, read.data);
    
  } else {
    
    User* user = new User(read.data);
    new_arg->connect->receive(read);
    if (read.tag == TAG_JOIN) {
      std::string room_name = read.data;
      read = Message(TAG_OK, "You joined " + room_name);
      new_arg->connect->send(read);
      receiver_comm(new_arg->connect, user, room_name);
    }
  }
  
  delete new_arg->connect;
  free(arg);
  
  return nullptr;
}

}

////////////////////////////////////////////////////////////////////////
// Server member function implementation
////////////////////////////////////////////////////////////////////////

Server::Server(int port)
  : m_port(port)
  , m_ssock(-1) {
  // TODO: initialize mutex
  pthread_mutex_init(&m_lock, NULL);
}

Server::~Server() {
  // TODO: destroy mutex
  pthread_mutex_destroy(&m_lock);
}

bool Server::listen() {
  // TODO: use open_listenfd to create the server socket, return true
  //       if successful, false if not
  m_ssock = open_listenfd(std::to_string(m_port).c_str());

  if (m_ssock < 0) {
    return false;
  }
  return true;
}

void Server::handle_client_requests() {
  // TODO: infinite loop calling accept or Accept, starting a new
  //       pthread for each connected client
  while (1) {
    socklen_t client_addr_len = sizeof(SA);
    SA sa;
    int fd = accept(m_ssock, &sa, &client_addr_len);
    if(fd > 0){
      Connection* connect = new Connection(fd);
      struct server_help* sh = (server_help*) malloc(sizeof(struct server_help));
      sh->connect = connect;
      sh->server = this;

      pthread_t thr_id;
      pthread_create(&thr_id, NULL, worker, sh);
    }
  }
}

Room *Server::find_or_create_room(const std::string &room_name) {
  // return a pointer to the unique Room object representing
  // the named chat room, creating a new one if necessary
  Guard g(m_lock);
  // case where the room doesn't exist
  // RoomMap::iterator it = m_rooms.find(room_name);
  if (m_rooms.count(room_name) == 0) {
    return new Room(room_name);
  } else {
    return m_rooms[room_name];
  }

}