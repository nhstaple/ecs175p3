//===========================================================================//
//                                                                           //
// Copyright(c) ECS 175 (2020)                                               //
// University of California, Davis                                           //
// MIT Licensed                                                              //
//                                                                           //
//===========================================================================//

// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include "shaders.h"
#include "util.hpp"
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
using namespace glm;

#include "arcball_camera.h"
ArcballCamera camera(vec3(0, 0, 5), vec3(0, 0, 0), vec3(0, 1, 0));

#include "geometry_triangle.h"
TriangleArrayObjects* objects;

bool enable_painter_algorithm = false;

void
cursor(GLFWwindow* window, double xpos, double ypos)
{
}

void
init()
{
  // -----------------------------------------------------------
  // For reference only, feel free to make changes
  // -----------------------------------------------------------

  // Initialise GLFW
  if (!glfwInit()) {
    throw std::runtime_error("Failed to initialize GLFW");
  }

  const char* glsl_version = "#version 150"; // GL 3.3 + GLSL 150
  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  // Open a window and create its OpenGL context
  window = glfwCreateWindow(1024, 768, "ECS 175 (press 'g' to display GUI)", NULL, NULL);
  if (window == NULL) {
    glfwTerminate();
    throw std::runtime_error("Failed to open GLFW window. If you have a GPU that is "
                             "not 3.3 compatible, try a lower OpenGL version.");
  }

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  // Load GLAD symbols
  int err = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress) == 0;
  if (err) {
    throw std::runtime_error("Failed to initialize OpenGL loader!");
  }

  glfwSetCursorPosCallback(window, cursor);

  // Ensure we can capture the escape key being pressed below
  glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

  // Dark blue background (avoid using black)
  glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

  // Enable depth test
  glEnable(GL_DEPTH_TEST);
  // Accept fragment if it closer to the camera than the former one
  glDepthFunc(GL_LESS);
}

void
PainterAlgorithm(const TriangleArrayObjects::Mesh& mesh)
{
  // -----------------------------------------------------------
  // TODO: painter's algorithm
  // -----------------------------------------------------------
}

int
main(void)
{
  init();

  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  // Create and compile our GLSL program from the shaders
  GLuint program_id = LoadProgram_FromEmbededTexts((char*)vshader, vshader_size, (char*)fshader, fshader_size);

  // Get a handle for our "MVP" uniform
  GLuint MVP_id = glGetUniformLocation(program_id, "MVP");

  // TODO
  // Get a handles for the phong data
  // GLuint Tralala_id = glGetUniformLocation(program_id, "myTraLaLa");

  // Load the texture
  GLuint tex = loadTexture_from_file("uvmap.jpg");

  // Read our .obj file
  objects = ReadAsArrayObjects("suzanne.obj");
  objects->Create();

  do {

    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Use our shader
    glUseProgram(program_id);

    // -----------------------------------------------------------
    // TODO: render your scene
    // -----------------------------------------------------------

    // Send our transformation to the currently bound shader, in the "MVP" uniform
    mat4 P = perspective(radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);
    mat4 M = objects->GetModelMatrix();
    mat4 V = camera.transform();
    mat4 MVP = P * V * M;

    // Send our transformation to the currently bound shader,
    // in the "MVP" uniform
    glUniformMatrix4fv(MVP_id, 1, GL_FALSE, &MVP[0][0]);

    if (!enable_painter_algorithm) {
      glEnable(GL_DEPTH_TEST);
      objects->Render();
    }
    else {
      // -----------------------------------------------------------
      // NOTE: YOU HAVE TO DISABLE DEPTH TEST FOR P3 !!!!!!!
      // otherwise you get 0 for this part
      // -----------------------------------------------------------
      glDisable(GL_DEPTH_TEST);
      // -----------------------------------------------------------
      // TODO: painter's algorithm
      // -----------------------------------------------------------
      for (auto& m : objects->meshes)
        PainterAlgorithm(m);
    }

    // -----------------------------------------------------------
    // TODO: post processing (half-toning)
    // -----------------------------------------------------------

    // Swap buffers
    glfwSwapBuffers(window);
    glfwPollEvents();

  } // Check if the ESC key was pressed or the window was closed
  while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);

  // Cleanup VBO and shader
  objects->Clear();
  glDeleteProgram(program_id);
  glDeleteTextures(1, &tex);
  glDeleteVertexArrays(1, &vao);

  // Close OpenGL window and terminate GLFW
  glfwTerminate();

  delete objects;

  return 0;
}
