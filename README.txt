Sample README.txt

Milestone #1

TODO: write a brief summary of how each team member contributed to
the project.

Together: Implemented significant fixes to sender.cpp, receiver.cpp and connection.cpp . This included
error handling, send/receive communications, etc.

Yash: Implemented initial connection.cpp and sender.cpp

Kai: Implemented initial receiver.cpp and some error handling in sender.cpp


Milestone #2

TODO: write a brief summary of how each team member contributed to
the project.

Together: Implemented server.cpp, a lot of revising (especially for receiver_comms and sender_comms), error handling,
etc.

Yash: Implemented MessageQueue.cpp

Kai: Implemented room.cpp

Eventually your report about how you implemented thread synchronization
in the server should go here

Please include where your critical sections are, how you determined them, and why you chose the synchronization primitives for each section. 
You should also explain how your critical sections ensure that the synchronization requirements are met without introducing synchronization hazards 
(e.g. race conditions and deadlocks).

In the Room class, our critical sections involved any modifications to the UserSet members field, which was a set of User objects. Each receiver thread accesses
shared rooms to insert user objects, and inserting an object into a shared data structure like a set requires synchronization. So, guard objects are made in
the add_member and remove_member functions, to prevent more than one thread from accessing the set at the same time when adding/removing User objects to the room.
The Guard object locks a mutex, and unlocks it upon exiting the scope of the function, preventing deadlocks and allowing other threads to modify the set 
afterwards, which made Guard objects/mutexes ideal here.

The broadcast_message function is another critical section, due to a different data structure. Each User object has a MessageQueue field, which stores message
objects in a deque, waiting to be transmitted to the receiver. Each sender thread must access the message queue for each user in the room being transmitted to,
and different threads may need to access the same user's messagequeue for transmitting messages. So, we guarded this function with another mutex using a Guard
object, which destructs at the end and prevents deadlocks, while preventing any data races with multiple threads accessing the same queue.