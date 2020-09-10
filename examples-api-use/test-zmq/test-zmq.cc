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

#include <zmq.hpp>

using rgb_matrix::GPIO;
using rgb_matrix::RGBMatrix;
using rgb_matrix::Canvas;

using rgb_matrix::WindowCanvas;

using namespace std;

volatile bool interrupt_received = false;
static void InterruptHandler(int signo) {
  interrupt_received = true;
}

int main(int argc, char *argv[]) {
  RGBMatrix::Options defaults;
  defaults.hardware_mapping = "regular";  // or e.g. "adafruit-hat"
  defaults.rows = 32;
  defaults.chain_length = 1;
  defaults.parallel = 1;
  defaults.show_refresh_rate = false;
  defaults.brightness = 30;

  Canvas *canvas = rgb_matrix::CreateMatrixFromFlags(&argc, &argv, &defaults);
  if (canvas == NULL)
    return 1;

  // Prepare context and socket
  zmq::context_t context (1);
  zmq::socket_t socket (context, ZMQ_REQ);
  
  std::cout << "Connecting to test server..." << std::endl;
  socket.connect("tcp://localhost:5555");

  // do 10 requests, waiting each time for a response
  for (int request_nbr = 0; request_nbr != 10; request_nbr++) {
    zmq::message_t request (5);
    memcpy(request.data(), "Hello", 5);
    std::cout << "Sending Hello " << request_nbr << "..." << std::endl;
    socket.send(request);

    // Get the reply
    zmq::message_t reply;
    socket.recv(&reply);
    std::cout << "Received World " << request_nbr << std::endl;
  }

  canvas->Fill(0,0,255);
  cout << "sleeping..." << endl;
  sleep(2);


  cout << "Size of canvas: " << sizeof(canvas) << endl;
  cout << "Size of dereferenced canvas: " << sizeof(&canvas) << endl;

  // Create window canvas that delegates to underlying canvas
  WindowCanvas *windowCanvas = new WindowCanvas(canvas, 64, 32, 64, 0);

  // It is always good to set up a signal handler to cleanly exit when we
  // receive a CTRL-C for instance. The DrawOnCanvas() routine is looking
  // for that.
  signal(SIGTERM, InterruptHandler);
  signal(SIGINT, InterruptHandler);

  // DrawOnCanvas(canvas);    // Using the canvas.
  // DrawOnWindowCanvas(canvas, windowCanvas);

  // Animation finished. Shut down the RGB matrix.
  canvas->Clear();
  delete canvas;

  return 0;
}
