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
#include "client_util.h"

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

// helper method to handle our communications for senders
void sender_comm(Connection* cn, Server* server, std::string username){

  // declared variables to help with navigating messages
  Message msg;
  std::string room = "";

  // infinite while loop for receving messages
  while(1){
    cn->receive(msg);
    // invalid message case where we encountered err or EOF
    if(cn->get_last_result() == Connection::EOF_OR_ERROR) break;

    // invalid message case where our message itself is invalid
    // with respect to our desired formatting
    if(cn->get_last_result() == Connection::INVALID_MSG){
      msg.modify(TAG_ERR, "Invalid message\n");
      if(!cn->send(msg)){
        break;
      }
    } // case where we wish to join a room
    else if(msg.tag == TAG_JOIN){
      room = msg.data;
      server->find_or_create_room(msg.data);
      msg.modify(TAG_OK, "joined\n");
      if(!cn->send(msg)){
        break;
      }
    } // case where user wish to leave a room
    else if(msg.tag == TAG_LEAVE){
      // check if the user is actually in a room
      if(room == ""){
        msg.modify(TAG_ERR, "you are not in a room!\n");
        if(!cn->send(msg)){
          break;
        }
      } // successful leaving of a room
      else{
        room = "";
        msg.modify(TAG_OK, "left room!\n");
        if(!cn->send(msg)){
          break;
        }
      }
    } // case where user wishes to quit altogether
    else if(msg.tag == TAG_QUIT){
      room = "";
      msg.modify(TAG_OK, "quit user\n");
      cn->send(msg);
      break;
    } // case where a user wishes to send a message to all
    else if(msg.tag == TAG_SENDALL){
      // failure to sendall because user is not in a room
      if(room == ""){
        msg.modify(TAG_ERR, "you are not in a room!\n");
        if(!cn->send(msg)){
          break;
        }
      } // successful sendall message
      else{
        server->find_or_create_room(room)->broadcast_message(username, msg.data);
        msg.modify(TAG_OK, "sent!\n");
        if(!cn->send(msg)){
          break;
        }
      }
    }
    else{
      //tag does not match protocol
      msg.modify(TAG_ERR, "Invalid tag\n");
      if(!cn->send(msg)){
        break;
      }
    }
  }
}

// helper method to handle our communications for receivers
void receiver_comm(Connection* cn, User* user, std::string room){

  // infinite loop intended to proceed through our MessageQueue for user,
  // recall that this loop breaking is a result of a fatal error or ctrl+c
  while (1) {
    // pointer to the next message in the queue for user
    Message* msg = user->mqueue.dequeue();
    // check to see if our message exists
    if (msg != nullptr) {
      // declared varaible to check whether the message sent to user was sucessful
      bool sent = cn->send(*msg);
      // delete the message now that it's been handled
      delete msg;
      // check if our sent was unsucessful, if so, break out of receiver communications
      if (!sent) {
        break;
      }
    }
  }
}

void *worker(void *arg) {
  pthread_detach(pthread_self());

  // use a static cast to convert arg from a void* to
  // whatever pointer type describes the object(s) needed
  // to communicate with a client (sender or receiver)
  server_help* new_arg = (server_help*) arg;

  // read login message (should be tagged either with
  // TAG_SLOGIN or TAG_RLOGIN), send response
  Message read;
  new_arg->connect->receive(read);

  while(read.tag != TAG_SLOGIN && read.tag != TAG_RLOGIN){
    if(new_arg->connect->get_last_result() == Connection::INVALID_MSG){
      Message msg(TAG_ERR, "Invalid message\n");
      if(!new_arg->connect->send(msg)){
        delete new_arg->connect;
        free(arg);
        return nullptr;  
      }
    }
    else if(new_arg->connect->get_last_result() == Connection::EOF_OR_ERROR){
      delete new_arg->connect;
      free(arg);
      return nullptr;
    }
    else if(read.tag != TAG_SLOGIN && read.tag != TAG_RLOGIN){
      Message msg(TAG_ERR, "Invalid tag\n");
      if(!new_arg->connect->send(msg)){
        delete new_arg->connect;
        free(arg);
        return nullptr;  
      }
    }
    new_arg->connect->receive(read);
  }


  bool sender = (read.tag == TAG_SLOGIN);
  std::string username = read.data;

  read.modify(TAG_OK, "logged in\n");
  if(!new_arg->connect->send(read)){
    delete new_arg->connect;
    free(arg);
    return nullptr;
  }

  
  
  // TODO: depending on whether the client logged in as a sender or
  //       receiver, communicate with the client (implementing
  //       separate helper functions for each of these possibilities
  //       is a good idea)
  if (sender) {
    sender_comm(new_arg->connect, new_arg->server, username);
  } else {
    User* user = new User(username);
    new_arg->connect->receive(read);

    while(read.tag != TAG_JOIN){
      if(new_arg->connect->get_last_result() == Connection::INVALID_MSG){
        Message msg(TAG_ERR, "Invalid message\n");
        if(!new_arg->connect->send(msg)){
          delete new_arg->connect;
          free(arg);
          return nullptr;
        }
      }
      else if(new_arg->connect->get_last_result() == Connection::EOF_OR_ERROR){
        delete new_arg->connect;
        free(arg);
        return nullptr;
      }
      else if(read.tag != TAG_JOIN){
        Message msg(TAG_ERR, "Invalid tag\n");
        if(!new_arg->connect->send(msg)){
          delete new_arg->connect;
          free(arg);
          return nullptr;
        }
      }
      new_arg->connect->receive(read);
    }
    
    std::string room_name = read.data;
    read = Message(TAG_OK, "You joined " + room_name);
    new_arg->connect->send(read);
    (new_arg->server->find_or_create_room(room_name))->add_member(user);

    receiver_comm(new_arg->connect, user, room_name);

    (new_arg->server->find_or_create_room(room_name))->remove_member(user);
    delete user;
    
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
  // initialize mutex
  pthread_mutex_init(&m_lock, NULL);
}

Server::~Server() {
  // destroy mutex
  pthread_mutex_destroy(&m_lock);
}

bool Server::listen() {
  // use open_listenfd to create the server socket, return true
  // if successful, false if not
  m_ssock = open_listenfd(std::to_string(m_port).c_str());
  // check if our server socket creation was successful
  if (m_ssock < 0) {
    return false;
  }
  return true;
}

void Server::handle_client_requests() {
  // infinite loop calling accept or Accept, starting a new
  // pthread for each connected client
  while (1) {
    // declare variables used as args for accept call
    socklen_t client_addr_len = sizeof(SA);
    SA sa;
    int fd = accept(m_ssock, &sa, &client_addr_len);
    // check to see if our fd is valid
    if(fd > 0){
      // new connection
      Connection* connect = new Connection(fd);
      // new server_help object and set its variables respectively
      struct server_help* sh = (server_help*) malloc(sizeof(struct server_help));
      sh->connect = connect;
      sh->server = this;
      // initialize pthread_t and create a new pthread
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
  if (m_rooms.count(room_name) == 0) {
    m_rooms[room_name] = new Room(room_name);
  } // case where room does exist
    return m_rooms[room_name];
  // Note: should unlock when Guard goes out-of-scope!  
}