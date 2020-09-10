//
//  Hello world client test in c++
//  Connects REQ socket to tcp://localhost:5555
//  Sends 'Hello' to server, expects 'World' back
//

#include <iostream>
#include <string>
#include <zmq.hpp>

int send_text_request(zmq::socket_t *socket, std::string text) {
  // Send the message
  std::cout << "Sending message [ " << text << " ]...";
  int text_length = text.length();
  zmq::message_t request (text_length);
  memcpy(request.data(), &text, text_length);
  socket->send(request);

  // Get the reply
  zmq::message_t reply;
  socket->recv(&reply);
  unsigned char* uc;
  uc = (unsigned char*)reply.data();
  std::string reply_text(reinterpret_cast<char const*>(uc), reply.size());
  std::cout << "Received message of size : " << reply.size() << std::endl;
  std::cout << "Message : " << reply_text << std::endl; 

  return 0;
}

int main () {
  // Prepare context and socket
  zmq::context_t context (1);
  zmq::socket_t socket (context, ZMQ_REQ);
  
  std::cout << "Connecting to test server..." << std::endl;
  socket.connect("tcp://localhost:5555");

  // do 10 requests, waiting each time for a response
  for (int request_nbr = 0; request_nbr != 10; request_nbr++) {
    std::string text = "msg ";
    text += std::to_string(request_nbr); 
    send_text_request(&socket, text);
  }

  return 0;
}