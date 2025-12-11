/*
 *  CSCI 441, Computer Graphics, Fall 2025
 *
 *  Project: MP
 *  File: main.cpp
 *
 *  Description:
 *      This file contains the basic setup to work with GLSL shaders and
 *      implement diffuse lighting.
 *
 *  Author: Dr. Paone, Colorado School of Mines, 2025
 *
 */

#include "A4Engine.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

///*****************************************************************************
//
// Our main function
int main() {
  const auto labEngine = new A4Engine();
  labEngine->initialize();
  if (labEngine->getError() ==
      CSCI441::OpenGLEngine::OPENGL_ENGINE_ERROR_NO_ERROR) {
    labEngine->run();
  }
  labEngine->shutdown();
  delete labEngine;
  return EXIT_SUCCESS;
}
