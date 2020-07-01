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

using rgb_matrix::GPIO;
using rgb_matrix::RGBMatrix;
using rgb_matrix::Canvas;

using rgb_matrix::WindowCanvas;

using namespace std;

volatile bool interrupt_received = false;
static void InterruptHandler(int signo) {
  interrupt_received = true;
}

static void DrawOnCanvas(Canvas *canvas) {
  /*
   * Let's create a simple animation. We use the canvas to draw
   * pixels. We wait between each step to have a slower animation.
   */
  canvas->Fill(0, 0, 255);

  int center_x = canvas->width() / 2;
  int center_y = canvas->height() / 2;
  float radius_max = canvas->width() / 2;
  float angle_step = 1.0 / 360;
  for (float a = 0, r = 0; r < radius_max; a += angle_step, r += angle_step) {
    if (interrupt_received)
      return;
    float dot_x = cos(a * 2 * M_PI) * r;
    float dot_y = sin(a * 2 * M_PI) * r;
    canvas->SetPixel(center_x + dot_x, center_y + dot_y,
                     255, 0, 0);
    usleep(1 * 1000);  // wait a little to slow down things.
  }
}

static void DrawOnWindowCanvas(Canvas *mainCanvas, WindowCanvas *windowCanvas) {
  // windowCanvas->Fill(0, 255, 0);
  mainCanvas->Fill(255, 0, 0);
  windowCanvas->FillWindow(0, 255, 0);
  
  usleep(3000 * 1000);

  cout << "Main Canvas Height: " << mainCanvas->height() << "\n";
  cout << "Main Canvas Width: " << mainCanvas->width() << "\n";

  cout << "WindowCanvas Height: " << windowCanvas->height() << "\n";
  cout << "WindowCanvas Width: " << windowCanvas->width() << "\n";
  cout << "WindowCanvas offset x: " << windowCanvas->offset_x() << "\n";
  cout << "WindowCanvas offset y: " << windowCanvas->offset_y() << "\n";

  for (int i = 0; i < 10; ++i) {
    if (interrupt_received)
      return;
    
    mainCanvas->SetPixel(20+i, 20+1, 0, 0, 255);
    windowCanvas->SetPixel(10+i, 10+1, 255, 0, 0);
  }

  usleep(1000 * 1000);
}

int main(int argc, char *argv[]) {
  RGBMatrix::Options defaults;
  defaults.hardware_mapping = "regular";  // or e.g. "adafruit-hat"
  defaults.rows = 32;
  defaults.chain_length = 1;
  defaults.parallel = 1;
  defaults.show_refresh_rate = true;
  defaults.brightness = 30;


  
  Canvas *canvas = rgb_matrix::CreateMatrixFromFlags(&argc, &argv, &defaults);
  if (canvas == NULL)
    return 1;

  // Create window canvas that delegates to underlying canvas
  WindowCanvas *windowCanvas = new WindowCanvas(canvas, 64, 32, 64, 0);

  // It is always good to set up a signal handler to cleanly exit when we
  // receive a CTRL-C for instance. The DrawOnCanvas() routine is looking
  // for that.
  signal(SIGTERM, InterruptHandler);
  signal(SIGINT, InterruptHandler);

  // DrawOnCanvas(canvas);    // Using the canvas.
  DrawOnWindowCanvas(canvas, windowCanvas);

  // Animation finished. Shut down the RGB matrix.
  canvas->Clear();
  delete canvas;

  return 0;
}
