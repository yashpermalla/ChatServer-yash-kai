#include "guard.h"
#include "message.h"
#include "message_queue.h"
#include "user.h"
#include "room.h"
#include <iostream>
#include "client_util.h" 

Room::Room(const std::string &room_name)
  : room_name(room_name) {
  // initialize the mutex
  pthread_mutex_init(&lock, NULL);
}

Room::~Room() {
  // destroy the mutex
  pthread_mutex_destroy(&lock);
}

void Room::add_member(User *user) {
  // add User to the room
  Guard g(lock);
  Room::members.insert(user);
  // Note: should unlock when Guard goes out-of-scope!
}

void Room::remove_member(User *user) {
  // remove User from the room
  Guard g(lock);
  Room::members.erase(user);
  // Note: should unlock when Guard goes out-of-scope!
}

void Room::broadcast_message(const std::string &sender_username, const std::string &message_text) {

  

  // send a message to every (receiver) User in the room
  Guard g(lock);


  for (User* user : members) {
    std::string payload = trim(room_name) + ":" + trim(sender_username) + ":" + message_text;
    Message* msg = new Message(TAG_DELIVERY, payload);
    user->mqueue.enqueue(msg);
  }
  // Note: should unlock when Guard goes out-of-scope!
}
