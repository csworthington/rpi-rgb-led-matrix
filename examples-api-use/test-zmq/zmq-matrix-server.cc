// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// Small example how to use the library.
// For more examples, look at demo-main.cc
//
// This code is public domain
// (but note, that the led-matrix library this depends on is GPL v2)

#include "led-matrix.h"

#include <unistd.h>
#include <math.h>
#include <stdio.h>
#include <signal.h>
#include <iostream>
#include <string>
#include <vector>
#include <zmq.hpp>

using rgb_matrix::GPIO;
using rgb_matrix::RGBMatrix;
using rgb_matrix::Canvas;
using rgb_matrix::FrameCanvas;

using rgb_matrix::WindowCanvas;

using namespace std;

volatile bool interrupt_received = false;
static void InterruptHandler(int signo) {
  interrupt_received = true;
}

int send_message_to_server (zmq::socket_t *socket, rgb_matrix::Canvas *canvas, int size) {
  // Create and send message
  zmq::message_t request(size);
  memcpy(request.data(), canvas, size);
  cout << "Sending byte array of size " << to_string(size) << "..." << endl;
  socket->send(request);

  // Wait for reply
  zmq::message_t reply;
  socket->recv(&reply);
  cout << "Received message of size : " << reply.size() << endl;

  return 0;
}

// const char ** serialize_canvas(rgb_matrix::FrameCanvas *frameCanvas) {
// void serialize_canvas(rgb_matrix::FrameCanvas *frameCanvas) {
//   cout << "attempting to serialize..." << endl;
//   size_t len;
//   vector<char> data;
//   // const char *data;
//   frameCanvas->Serialize(&data, &len);

//   cout << "serialized length = " << to_string(len) << endl;

//   // return &data;
// }

int main(int argc, char *argv[]) {
  // It is always good to set up a signal handler to cleanly exit when we
  // receive a CTRL-C for instance. The DrawOnCanvas() routine is looking
  // for that.
  signal(SIGTERM, InterruptHandler);
  signal(SIGINT, InterruptHandler);


  // Prepare context and socket
  zmq::context_t context (1);
  zmq::socket_t socket (context, ZMQ_REP);
  socket.bind ("tcp://*:5555");

  std::cout << "Server created at \"tcp://localhost:5555\"" << std::endl;

  // Prepare matrix
  RGBMatrix::Options defaults;
  defaults.hardware_mapping = "regular";  // or e.g. "adafruit-hat"
  defaults.rows = 32;
  defaults.chain_length = 1;
  defaults.parallel = 1;
  defaults.show_refresh_rate = false;
  defaults.brightness = 30;


  // Prepare canvas
  // Canvas *canvas = rgb_matrix::CreateMatrixFromFlags(&argc, &argv, &defaults);
  RGBMatrix *matrix = rgb_matrix::CreateMatrixFromFlags(&argc, &argv, &defaults);
  if (matrix == NULL)
    return 1;
  
  FrameCanvas *frameCanvas = matrix->CreateFrameCanvas();

  // const char* serialized_canvas;
  // size_t serialized_canvas_len; 
  // frameCanvas->Serialize(&serialized_canvas, &serialized_canvas_len);

  while (true) {
    zmq::message_t request;

    // Wait for request from client
    socket.recv(&request);
    cout << "Received request of size " << to_string(request.size()) << "..." << endl;

    frameCanvas->Deserialize(static_cast<char*>(request.data()), request.size());
    matrix->SwapOnVSync(frameCanvas);

    zmq::message_t reply (1);
    memcpy(reply.data(), nullptr, 1);
    socket.send(reply);
  }

  delete matrix;

  cout << "closing..." << endl;

  return 0;
}
