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
#include <chrono>

#include "shaders.h"
#include "util.hpp"
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
using namespace glm;

// include ImGUI
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "arcball_camera.h"
ArcballCamera camera(vec3(0, 0, 5), vec3(0, 0, 0), vec3(0, 1, 0));

#include "geometry_triangle.h"
TriangleArrayObjects* objects;

vec3 normalize_vector(const vec3 v) {
  float mag = sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
  if(mag != 0) {
    return vec3(v.x/mag, v.y/mag, v.z/mag);
  } else {
    return vec3(0, 0, 0);
  }
}

// globals
// control states
struct PhongState {
  bool enable = true;
  bool enable_ambient = true;
  bool enable_diffuse = true;
  bool enable_specular = true;
  float ambient_coefficient = 0.5;
  float diffuse_coefficient = 0.5;
  float specular_coefficient = 0.5;
  int specular_factor = 2;
  vec3 ambient_rgb = vec3(0.5, 0.5, 0.5);

  float amb_color_float[3];

  PhongState() {
    amb_color_float[0] = ambient_rgb.x;
    amb_color_float[1] = ambient_rgb.y;
    amb_color_float[2] = ambient_rgb.z;
  }

  float* GetAmbientColor() {
    amb_color_float[0] = ambient_rgb.x;
    amb_color_float[1] = ambient_rgb.y;
    amb_color_float[2] = ambient_rgb.z;
    return amb_color_float;
  }
  void UpdateColor() {
    ambient_rgb.x = amb_color_float[0];
    ambient_rgb.y = amb_color_float[1];
    ambient_rgb.z = amb_color_float[2];
  }
} phong;

struct PaintersState {
  bool enable = false;
} painters;

struct GUIState {
  bool enable = true;
} gui;

struct PointLightSource {
  vec3 position = vec3(0, 0, 0);
  vec3 color_rgb = vec3(1, 0.25, 0.25);
  int color_intensity = 1;

  float color_float[3];
  float* GetColorArray() {
    color_float[0] = color_rgb.x;
    color_float[1] = color_rgb.y;
    color_float[2] = color_rgb.z;
    return color_float;
  }
  void UpdateRGB() {
    color_rgb.x = color_float[0];
    color_rgb.y = color_float[1];
    color_rgb.z = color_float[2];
  }
  float GetUniR() const {
    // return normalize_vector(color_rgb * intensity).x;
    return color_rgb.x; // * 16 * color_intensity;
  }
  float GetUniG() const {
    // return normalize_vector(color_rgb * intensity).y;
    return color_rgb.y; //  * 16 * color_intensity;
  }
  float GetUniB() const {
    // return normalize_vector(color_rgb * intensity).z;
    return color_rgb.z; // * 16 * color_intensity;
  }
} light;

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

  // ImGui
  {
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // or ImGui::StyleColorsClassic();

    // Initialize Dear ImGui
    ImGui_ImplGlfw_InitForOpenGL(window, true /* 'true' -> allow imgui to capture keyboard inputs */);
    ImGui_ImplOpenGL3_Init(glsl_version);
  }

  // Dark blue background (avoid using black)
  glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

  // Enable depth test
  glEnable(GL_DEPTH_TEST);
  // Accept fragment if it closer to the camera than the former one
  glDepthFunc(GL_LESS);
}

void
DrawGUI(bool* p_open=&gui.enable)
{
  // Initialization
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  // measure frame rate
  static float fps = 0.0f;
  {
    static bool opened = false;
    static int frames = 0;
    static auto start = std::chrono::system_clock::now();
    if (!opened) {
      start = std::chrono::system_clock::now();
      frames = 0;
    }
    std::chrono::duration<double> elapsed_seconds = std::chrono::system_clock::now() - start;
    ++frames;
    if (frames % 10 == 0 || frames == 1) // dont update this too frequently
      fps = frames / elapsed_seconds.count();
    opened = *p_open;
  }

  ivec2 window_size, framebuffer_size;
  glfwGetWindowSize(window, &window_size.x, &window_size.y);
  glfwGetFramebufferSize(window, &framebuffer_size.x, &framebuffer_size.y);

  // draw a fixed GUI window
  const float distance = 10.0f;
  static int corner = 0;
  ImVec2 window_pos = ImVec2((corner & 1) ? ImGui::GetIO().DisplaySize.x - distance : distance,
                             (corner & 2) ? ImGui::GetIO().DisplaySize.y - distance : distance);
  ImVec2 window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
  ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
  ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.3f)); // Transparent background
  const auto flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize |
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;
  if (ImGui::Begin("Information", NULL, flags)) {
    ImGui::Text("FPS (Hz): %.f\n", fps);

    /*
    if (ImGui::Checkbox("Render Depth", &render_depth)) {
      // As we are switching to a different mode, we need to setup the framebuffer object again
      fbo.Resize(framebuffer_size.x, framebuffer_size.y, render_depth);
    }
   */

    if(ImGui::Checkbox("Enable Phong", &phong.enable)) {
    }

    if(phong.enable) {
      // Phong Parameters
      if(ImGui::Checkbox("Enable Ambient", &phong.enable_ambient)) {
      }
      if(phong.enable_ambient) {
        // ambient coefficient
        ImGui::SliderFloat("k_a", &phong.ambient_coefficient, 0, 1.0);
        ImGui::ColorEdit3("Ambient Color", phong.GetAmbientColor()); phong.UpdateColor();
      }
      if(ImGui::Checkbox("Enable Diffuse", &phong.enable_diffuse)) {
      }
      if(phong.enable_diffuse) {
        // diffuse coefficient
        ImGui::SliderFloat("k_d", &phong.diffuse_coefficient, 0, 1.0);
      }
      if(ImGui::Checkbox("Enable Specular", &phong.enable_specular)) {
      }
      if(phong.enable_specular) {
        // specular coefficient
        ImGui::SliderFloat("k_s", &phong.specular_coefficient, 0, 1.0);
        // specular factor
        ImGui::SliderInt("Specular Factor", &phong.specular_factor, 0, 12);
      }
      // Camera
      // camera position
      ImGui::Text("Camera Position: TODO");
      // TODO

      // light position
      ImGui::Text("Light Position");
      ImGui::SliderFloat("x##light", &light.position.x, -100, 100);
      ImGui::SliderFloat("y##light", &light.position.y, -100, 100);
      ImGui::SliderFloat("z##light", &light.position.z, -100, 100);
      // color slider
      ImGui::ColorEdit3("Light Source Color", light.GetColorArray()); light.UpdateRGB();
      ImGui::SliderInt("light intensity", &light.color_intensity, 1, 500);
    }

    ImGui::End();
  }
  ImGui::PopStyleColor();

  // Render GUI
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
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
  // phong
  GLuint Phong_Enable_id = glGetUniformLocation(program_id, "enablePhong");
  // ambient
  GLuint Phong_Ambient_Enable_id = glGetUniformLocation(program_id, "enableAmbient");
  GLuint Ambient_Intensity_id = glGetUniformLocation(program_id, "I_a");
  GLuint Ambient_Coefficient_id = glGetUniformLocation(program_id, "k_a");
  // diffuse
  GLuint Phong_Diffuse_Enable_id = glGetUniformLocation(program_id, "enableDiffuse");
  GLuint Diffuse_Coefficient_id = glGetUniformLocation(program_id, "k_d");
  // specular
  GLuint Phong_Specular_Enable_id = glGetUniformLocation(program_id, "enableSpecular");
  GLuint Specular_Coefficient_id = glGetUniformLocation(program_id, "k_s");
  GLuint Specular_Level_id = glGetUniformLocation(program_id, "specularLevel");
  // camera
  GLuint Camera_Location_id = glGetUniformLocation(program_id, "cameraLocation");
  // light
  GLuint Light_Location_id = glGetUniformLocation(program_id, "lightLocation");
  GLuint Light_Color_id = glGetUniformLocation(program_id, "lightColor");

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
    if(phong.enable) {
      // phong flags
      glUniform1ui(Phong_Enable_id, 1);
    } else {
      glUniform1ui(Phong_Enable_id, 0);
    }

    glUniform1ui(Phong_Ambient_Enable_id, phong.enable_ambient);
    glUniform1ui(Phong_Diffuse_Enable_id, phong.enable_diffuse);
    glUniform1ui(Phong_Specular_Enable_id, phong.enable_specular);
    // phong parameters: k
    glUniform1f(Ambient_Coefficient_id, phong.ambient_coefficient);
    glUniform1f(Diffuse_Coefficient_id, phong.diffuse_coefficient);
    glUniform1f(Specular_Coefficient_id, phong.specular_coefficient);
    // phong camera
    glUniform3f(Camera_Location_id, camera.eye().x, camera.eye().y, camera.eye().z);
    // phong light
    glUniform3f(Light_Location_id, light.position.x, light.position.y, light.position.z);
    glUniform3f(Light_Color_id, light.GetUniR(), light.GetUniR(), light.GetUniB());
    // ambient
    glUniform3f(Ambient_Intensity_id, phong.ambient_rgb.x, phong.ambient_rgb.y, phong.ambient_rgb.z);
    // specular
    glUniform1ui(Specular_Level_id, phong.specular_factor);

    if (!painters.enable) {
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

    DrawGUI();

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
