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

#define LIGHT_LOCATION_SCALE 4

// globals
// control states
struct PhongState {
  bool enable = true;
  bool enable_ambient = true;
  bool enable_diffuse = true;
  bool enable_specular = true;
  int specular_factor = 2;
  vec3 ambient_rgb = vec3(0.5, 0.5, 0.5);
  vec3 diffuse_rgb = vec3(1, 1, 1);
  float ambient_intensity = 1;
  float K = 0;
  bool cycle_k = false;

  float amb_color_float[3];
  float dif_color_float[3];

  PhongState() {
    amb_color_float[0] = ambient_rgb.x;
    amb_color_float[1] = ambient_rgb.y;
    amb_color_float[2] = ambient_rgb.z;
    dif_color_float[0] = diffuse_rgb.x;
    dif_color_float[1] = diffuse_rgb.y;
    dif_color_float[2] = diffuse_rgb.z;
  }

  float* GetAmbientColor() {
    amb_color_float[0] = ambient_rgb.x;
    amb_color_float[1] = ambient_rgb.y;
    amb_color_float[2] = ambient_rgb.z;
    return amb_color_float;
  }

  float* GetDiffuseColor() {
    dif_color_float[0] = diffuse_rgb.x;
    dif_color_float[1] = diffuse_rgb.y;
    dif_color_float[2] = diffuse_rgb.z;
    return dif_color_float;
  }

  void UpdateColor() {
    ambient_rgb.x = amb_color_float[0];
    ambient_rgb.y = amb_color_float[1];
    ambient_rgb.z = amb_color_float[2];
    diffuse_rgb.x = dif_color_float[0];
    diffuse_rgb.y = dif_color_float[1];
    diffuse_rgb.z = dif_color_float[2];
  }
} phong;

struct GouraudState {
  bool enable;
} gouraud;

struct PaintersState {
  bool enable = false;
} painters;

struct GUIState {
  bool enable = true;
} gui;

struct PointLightSource {
  vec3 position = vec3(0, 0, 0);
  vec3 color_rgb = vec3(1, 0.25, 0.25);
  float intensity = 1;

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

} light;

struct CameraState {
  ArcballCamera camera = ArcballCamera(vec3(0, 0, 5), vec3(0, 0, 0), vec3(0, 1, 0));
  float mouse_scroll_sensitivty = 1;
  bool lock_ortho = false;
} camera;

void scroll(GLFWwindow* window, double xoff, double yoff)
{
  ImGuiIO& io = ImGui::GetIO();
  if (!io.WantCaptureMouse) {
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    static float zoom;
    float new_zoom = (zoom + yoff * camera.mouse_scroll_sensitivty);

    // right click -> zoom
    if (zoom != new_zoom) {
      camera.camera.zoom(-zoom);
      camera.camera.zoom(new_zoom);
      zoom = new_zoom;
    }
  }
}

void
cursor(GLFWwindow* window, double xpos, double ypos)
{
  ImGuiIO& io = ImGui::GetIO();
  if (!io.WantCaptureMouse) {
    int left_state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
    int right_state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);

    int width, height;
    glfwGetWindowSize(window, &width, &height);

    static vec2 prev_cursor;
    vec2 cursor((xpos / width - 0.5f) * 2.f, (0.5f - ypos / height) * 2.f);
    cursor = camera.mouse_scroll_sensitivty * cursor;

    // right click -> zoom
    if (right_state == GLFW_PRESS || right_state == GLFW_REPEAT) {
      camera.camera.zoom(cursor.y - prev_cursor.y);
    }

    // left click -> rotate
    if (left_state == GLFW_PRESS || left_state == GLFW_REPEAT) {
      camera.camera.rotate(prev_cursor, cursor);
    }

    prev_cursor = cursor;
  }
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
  glfwSetScrollCallback(window, scroll);

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

void glOrtho(
  const float &b, const float &t, const float &l, const float &r,
  const float &n, const float &f,
  mat4 &M)
{
  // set OpenGL perspective projection matrix
  M[0][0] = 2 / (r - l);
  M[0][1] = 0;
  M[0][2] = 0;
  M[0][3] = 0;

  M[1][0] = 0;
  M[1][1] = 2 / (t - b);
  M[1][2] = 0;
  M[1][3] = 0;

  M[2][0] = 0;
  M[2][1] = 0;
  M[2][2] = -2 / (f - n);
  M[2][3] = 0;

  M[3][0] = -(r + l) / (r - l);
  M[3][1] = -(t + b) / (t - b);
  M[3][2] = -(f + n) / (f - n);
  M[3][3] = 1;
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

    // Camera
    ImGui::Text("Camera Control Parameters");
    if(camera.mouse_scroll_sensitivty != 0) {
      ImGui::SliderFloat("Sensitivity##cam_control", &camera.mouse_scroll_sensitivty, 0, 1);
    } else {
      if(ImGui::Button("Enable##cam_control")) {
        camera.mouse_scroll_sensitivty = 1;
      }
    }
    ImGui::Checkbox("Lock Ortho View", &camera.lock_ortho);

    ImGui::Text("Painter's Algorithm");
    ImGui::Checkbox("Enable##painters", &painters.enable);

    // Light
    ImGui::Text("Light Position");
    ImGui::SliderFloat("x##light", &light.position.x, -1, 1);
    ImGui::SliderFloat("y##light", &light.position.y, -1, 1);
    ImGui::SliderFloat("z##light", &light.position.z, -1, 1);
    // color slider
    ImGui::SliderFloat("I_l", &light.intensity, 0, 1);
    ImGui::ColorEdit3("Light Source Color", light.GetColorArray()); light.UpdateRGB();
    ImGui::SliderFloat("K", &phong.K, -1, 1);
    ImGui::Checkbox("Cycle K", &phong.cycle_k);

    if(ImGui::Checkbox("Enable Phong", &phong.enable)) {
      if(phong.enable && gouraud.enable) {
        gouraud.enable = false;
      }
    }

    if(ImGui::Checkbox("Enable Gouraud", &gouraud.enable)) {
      if(gouraud.enable && phong.enable) {
        phong.enable = false;
      }
    }

    // Lighting Parameters
    ImGui::Checkbox("Enable Ambient", &phong.enable_ambient);

    if(phong.enable_ambient) {
      // ambient coefficient
      ImGui::SliderFloat("I_a", &phong.ambient_intensity, 0, 1);
      ImGui::ColorEdit3("Ambient Color", phong.GetAmbientColor()); phong.UpdateColor();
    }
    ImGui::Checkbox("Enable Diffuse", &phong.enable_diffuse);

    if(phong.enable_diffuse) {
      // diffuse coefficient
      ImGui::ColorEdit3("Material Color", phong.GetDiffuseColor()); phong.UpdateColor();
    }

    ImGui::Checkbox("Enable Specular", &phong.enable_specular);

    if(phong.enable_specular) {
      // specular factor
      ImGui::SliderInt("Specular Factor", &phong.specular_factor, 0, 16);
    }

    ImGui::End();
  }
  ImGui::PopStyleColor();

  // Render GUI
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

#include <gl/gl.h>

extern void PainterAlgorithm(const TriangleArrayObjects::Mesh& mesh, const int axis=X_DISTANCE);

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
  GLuint Gouraud_Enable_id = glGetUniformLocation(program_id, "enableGouraud");
  // ambient
  GLuint Phong_Ambient_Enable_id = glGetUniformLocation(program_id, "enableAmbient");
  GLuint Ambient_Intensity_id = glGetUniformLocation(program_id, "I_a");
  GLuint Ambient_Color_id = glGetUniformLocation(program_id, "k_a");
  // diffuse
  GLuint Phong_Diffuse_Enable_id = glGetUniformLocation(program_id, "enableDiffuse");
  GLuint Diffuse_Color_id = glGetUniformLocation(program_id, "k_d");
  // specular
  GLuint Phong_Specular_Enable_id = glGetUniformLocation(program_id, "enableSpecular");
  GLuint Specular_Color_id = glGetUniformLocation(program_id, "k_s");
  GLuint Specular_Level_id = glGetUniformLocation(program_id, "specularLevel");
  // camera
  GLuint Camera_Location_id = glGetUniformLocation(program_id, "cameraLocation");
  // light
  GLuint Light_Location_id = glGetUniformLocation(program_id, "lightLocation");
  GLuint Light_Intensity_id = glGetUniformLocation(program_id, "I_l");
  GLuint K_id = glGetUniformLocation(program_id, "K");

  // Load the texture
  GLuint tex = loadTexture_from_file("uvmap.jpg");

  GLuint Time_id = glGetUniformLocation(program_id, "time");

  // Read our .obj file
  objects = ReadAsArrayObjects("suzanne.obj");
  objects->Create();

  float tick = 0;

  int width, height;
  glfwGetWindowSize(window, &width, &height);

  const vec3 xy_pos = vec3(0, 0, -10);
  const vec3 yz_pos = vec3(0, 0, -10);
  const vec3 xz_pos = vec3(0, 0, -10);

  const auto SWAP_XY = mat4({1, 0, 0, 0},
                            {0, 1, 0, 0},
                            {0, 0, 1, 0},
                            {0, 0, 0, 1});

  const auto SWAP_YZ = mat4({0, 1, 0, 0},
                            {0, 0, 1, 0},
                            {1, 0, 0, 0},
                            {0, 0, 0, 1});

  const auto SWAP_XZ = mat4({1, 0, 0, 0},
                            {0, 0, 1, 0},
                            {0, 1, 0, 0},
                            {0, 0, 0, 1});

  const auto I = mat4({1, 0, 0, 0},
                      {0, 1, 0, 0},
                      {0, 0, 1, 0},
                      {0, 0, 0, 1});

  const mat4 XY = rotate<float>(SWAP_XY,0,{0,0, 1});
  const mat4 YZ = rotate<float>(SWAP_YZ,-90*0.01745329252,{0, 0, 1});
  const mat4 XZ = rotate<float>(SWAP_XZ,180*0.01745329252,{0,1, 0});

  // const mat4 XY = I;
  // const mat4 YZ = rotate<float>(rotate<float>(I, 90, {1, 0, 0}), 90, {0, 0, 1});
  // const mat4 XZ = rotate<float>(rotate<float>(I, 90, {0, 1, 0}), 90, {0, 0, 1});
  mat4 O = mat4(); glOrtho(-3, 3, -3, 3, -100, 100, O);

  const auto boop = camera.camera.transform();

  const mat4 M_xy_o = O * boop * XY * objects->GetModelMatrix();
  const mat4 M_yz_o = O * boop * YZ * objects->GetModelMatrix();
  const mat4 M_xz_o = O * boop * XZ * objects->GetModelMatrix();

  do {
    if(phong.cycle_k || (!phong.enable && !gouraud.enable)) tick += 1e-2;
    if(tick > 9000) tick = tick - 9000;
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
    mat4 V = camera.camera.transform();
    mat4 MVP = P * V * M;

    mat4 MVP_xy;
    mat4 MVP_yz;
    mat4 MVP_xz;

    if(!camera.lock_ortho) {
      MVP_xy = O * camera.camera.transform() * XY * objects->GetModelMatrix();
      MVP_yz = O * camera.camera.transform() * YZ * objects->GetModelMatrix();
      MVP_xz = O * camera.camera.transform() * XZ * objects->GetModelMatrix();
    } else {
      MVP_xy = O * boop * XY * objects->GetModelMatrix();
      MVP_yz = O * boop * YZ * objects->GetModelMatrix();
      MVP_xz = O * boop * XZ * objects->GetModelMatrix();
    }

    // flags
    glUniform1ui(Phong_Enable_id, phong.enable);
    glUniform1ui(Phong_Ambient_Enable_id, phong.enable_ambient);
    glUniform1ui(Phong_Diffuse_Enable_id, phong.enable_diffuse);
    glUniform1ui(Phong_Specular_Enable_id, phong.enable_specular);
    glUniform1ui(Gouraud_Enable_id, gouraud.enable);
    // colors
    glUniform3f(Ambient_Color_id, phong.ambient_rgb.x, phong.ambient_rgb.y, phong.ambient_rgb.z);
    glUniform3f(Diffuse_Color_id, phong.diffuse_rgb.x, phong.diffuse_rgb.y, phong.diffuse_rgb.z);
    glUniform3f(Specular_Color_id, light.color_rgb.x, light.color_rgb.y, light.color_rgb.z);
    // intensities
    glUniform1f(Light_Intensity_id, light.intensity);
    glUniform1f(Ambient_Intensity_id, phong.ambient_intensity);
    // specular n
    glUniform1ui(Specular_Level_id, phong.specular_factor);
    // time
    glUniform1f(Time_id, tick);
    if(phong.cycle_k) phong.K = (float)glm::sin(tick/2);
    glUniform1f(K_id, 10 * phong.K);

    { // first view port
      glViewport(0, 0, width / 2, height / 2);

      // Send our transformation to the currently bound shader,
      // in the "MVP" uniform
      glUniformMatrix4fv(MVP_id, 1, GL_FALSE, &MVP[0][0]);

      // camera
      glUniform3f(Camera_Location_id, camera.camera.eye().x, camera.camera.eye().y, camera.camera.eye().z);
      // light
      glUniform3f(Light_Location_id,
                  LIGHT_LOCATION_SCALE * light.position.x,
                  LIGHT_LOCATION_SCALE * light.position.y,
                  LIGHT_LOCATION_SCALE * light.position.z);

      if(!painters.enable) {
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

        for (auto& m : objects->meshes) {
          // PainterAlgorithm(m, -1);
        }
      }
    }


    { // second viewport
      glViewport(width / 2, 0, width / 2, height / 2);
      glUniformMatrix4fv(MVP_id, 1, GL_FALSE, &MVP_xy[0][0]);
      glUniform3f(Camera_Location_id, xy_pos.x, xy_pos.y, xy_pos.z);
      //glUniform3f(Light_Location_id,
      //            LIGHT_LOCATION_SCALE * light.position.x,
      //            LIGHT_LOCATION_SCALE * light.position.y,
      //            LIGHT_LOCATION_SCALE * light.position.z);

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
        for (auto& m : objects->meshes) {
          // PainterAlgorithm(m, Z_DISTANCE);
        }
      }
    }

    { // third viewport
      glViewport(0, height / 2, width / 2, height / 2);
      glUniformMatrix4fv(MVP_id, 1, GL_FALSE, &MVP_xz[0][0]);
      glUniform3f(Camera_Location_id, xz_pos.x, xz_pos.y, xz_pos.z);
      //glUniform3f(Light_Location_id,
      //            LIGHT_LOCATION_SCALE * light.position.x,
      //            LIGHT_LOCATION_SCALE * light.position.z,
      //            LIGHT_LOCATION_SCALE * light.position.y);

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
        for (auto& m : objects->meshes) {
          // PainterAlgorithm(m, Y_DISTANCE);
        }
      }
    }

    { // fourth viewport
      glViewport(width / 2, height / 2, width / 2, height / 2);
      glUniformMatrix4fv(MVP_id, 1, GL_FALSE, &MVP_yz[0][0]);
      glUniform3f(Camera_Location_id, yz_pos.x, yz_pos.y, yz_pos.z);
      //glUniform3f(Light_Location_id,
      //            LIGHT_LOCATION_SCALE * light.position.y,
      //            LIGHT_LOCATION_SCALE * light.position.z,
      //            LIGHT_LOCATION_SCALE * light.position.x);

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
        for (auto& m : objects->meshes) {
          // PainterAlgorithm(m, X_DISTANCE);
        }
      }
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
