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

bool is_same_data(const char* a1, size_t size1, const char* a2, size_t size2) {
  if (size1 != size2) {
    return false;
  }
  for (int i = 0; i < size1; ++i) {
    if (a1[i] != a2[i]) {
      cout << "a1[" << to_string(i) << "] = " << to_string(a1[i]) << ", a2[" << to_string(i) << "] = " << to_string(a2[i]) << endl;
      return false;
    }
  }
  return true;
}

int send_message_to_server (zmq::socket_t *socket, const char* serialized_canvas, size_t size) {
  // Create and send message
  zmq::message_t request(size);
  memcpy(request.data(), serialized_canvas, size);

  // if (is_same_data(serialized_canvas, size, static_cast<char*>(request.data()), request.size())) {
  //   cout << "data is same" << endl;
  // } else {
  //   cout << "data is not same" << endl;
  //   return 0;
  // }

  cout << "Sending byte array of size " << to_string(size) << "..." << endl;
  socket->send(request);

  // Wait for reply
  zmq::message_t reply;
  socket->recv(&reply);
  cout << "Received message of size : " << reply.size() << endl;

  return 0;
}

int main(int argc, char *argv[]) {
  // It is always good to set up a signal handler to cleanly exit when we
  // receive a CTRL-C for instance. The DrawOnCanvas() routine is looking
  // for that.
  signal(SIGTERM, InterruptHandler);
  signal(SIGINT, InterruptHandler);


  // Prepare context and socket
  zmq::context_t context (1);
  zmq::socket_t socket (context, ZMQ_REQ);
  
  std::cout << "Connecting to test server..." << std::endl;
  socket.connect("tcp://localhost:5555");

  

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

  // const char** serialized_canvas = serialize_canvas(frameCanvas);
  // const char** serialized_canvas = NULL;
  // serialize_canvas(frameCanvas);
  const char* serialized_canvas;
  size_t serialized_canvas_len; 
  frameCanvas->Serialize(&serialized_canvas, &serialized_canvas_len);

  frameCanvas->Fill(0,255,255);
  frameCanvas->Serialize(&serialized_canvas, &serialized_canvas_len);
  send_message_to_server(&socket, serialized_canvas, serialized_canvas_len);
  // send_message_to_server(&socket, frameCanvas, sizeof(frameCanvas));

  cout << "sleeping..." << endl;
  sleep(2);
  // canvas->Clear();
  // send_message_to_server(&socket, canvas, sizeof(canvas));
  frameCanvas->Clear();
  frameCanvas->Serialize(&serialized_canvas, &serialized_canvas_len);
  // send_message_to_server(&socket, frameCanvas, sizeof(frameCanvas));
  send_message_to_server(&socket, serialized_canvas, serialized_canvas_len);

  cout << "finished sending messages..." << endl;
  cout << "matrix = " << matrix << endl;

  

  // Create window canvas that delegates to underlying canvas
  // WindowCanvas *windowCanvas = new WindowCanvas(canvas, 64, 32, 64, 0);

  // DrawOnCanvas(canvas);    // Using the canvas.
  // DrawOnWindowCanvas(canvas, windowCanvas);

  // Animation finished. Shut down the RGB matrix.
  // delete canvas;
  delete matrix;

  cout << "closing..." << endl;

  return 0;
}
