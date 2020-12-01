//
// Created by nick on 11/30/2020.
//

#include "geometry_triangle.h"
extern TriangleArrayObjects* objects;

#include <vector>
using std::vector;

vector<float> v;
vector<float> n;
vector<float> t;
GLuint count;
void reset() {
  v.clear();
  n.clear();
  t.clear();
  count = 0;
}

short int compare(GLuint i, GLuint j) {
  if(i < 0 || j < 0 || i >= count || j >= count) {
    throw std::runtime_error("bad index!");
  }

}

void swap(GLuint i, GLuint j) {
  if(i < 0 || j < 0 || i >= count || j >= count || i % 3 != 0 || j % 3 != 0) {
    throw std::runtime_error("bad index!");
  } else {
    float v_temp_x = v[i];
    float v_temp_y = v[i+1];
    float v_temp_z = v[i+2];
    v[i] = v[j];
    v[i+1] = v[j+1];
    v[i+2] = v[j+2];
    v[j] = v_temp_x;
    v[j+1] = v_temp_y;
    v[j+2] = v_temp_z;

    float n_temp_x = n[i];
    float n_temp_y = n[i+1];
    float n_temp_z = n[i+2];
    n[i] = n[j];
    n[i+1] = n[j+1];
    n[i+2] = n[j+2];
    n[j] = n_temp_x;
    n[j+1] = n_temp_y;
    n[j+2] = n_temp_z;

    float t_temp_x_u = t[i];
    float t_temp_x_v = t[i+1];
    float t_temp_y_u = t[i+2];
    float t_temp_y_v = t[i+3];
    float t_temp_z_u = t[i+4];
    float t_temp_z_v = t[i+5];
    t[i] = n[j];
    t[i+1] = n[j+1];
    t[i+2] = n[j+2];
    n[j] = n_temp_x;
    n[j+1] = n_temp_y;
    n[j+2] = n_temp_z;
  }
}

// sort this mesh with the swap
void
PainterAlgorithm(const TriangleArrayObjects::Mesh& mesh, const int axis=X_DISTANCE)
{
  reset();
  auto* v_ = mesh.vertices.get();
  auto* n_ = mesh.normals.get();
  auto* t_ = mesh.texcoords.get();
  count = mesh.size_triangles;
  for(int i = 0; i + 2 < count*9; i = i + 3) {
    v.push_back(v_[i]);
    n.push_back(n_[i + 1]);
    t.push_back(t_[i + 2]);
  }

  for(int i = 0; i < count*9; i++) {
    v.push_back(v_[i]);
    n.push_back(n_[i]);
  }
  for(int i = 0; i < count*6; i++) {
    t.push_back(t_[i]);
  }

  bool swapped = false;
  /*
  do {
    swapped = false;
    for(int i = 1; i < count; i++) {
      if (mesh.compare(i - 1, i, axis) > 0) {
        m.swap(i - 1, i);
        swapped = true;
      }
    }
  } while(!swapped);
  */
}