//===========================================================================//
//                                                                           //
// Copyright(c) ECS 175 (2020)                                               //
// University of California, Davis                                           //
// MIT Licensed                                                              //
//                                                                           //
//===========================================================================//

#pragma once

#include "geometry.h"

#include "tiny_obj_loader.h"

#include <memory>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#define X_DISTANCE 0
#define Y_DISTANCE 1
#define Z_DISTANCE 2

struct TriangleArrayObjects : public AbstractGeometry {
  struct Mesh {
    std::unique_ptr<float> vertices;
    std::unique_ptr<float> normals;
    std::unique_ptr<float> texcoords;
    size_t size_triangles;
    GLuint vbo_vertex, vbo_normal, vbo_texcoord;
    void swap(const int i, const int j) {
      if(i < 0 || j < 0 || i >= size_triangles || j >= size_triangles) {
        throw std::runtime_error("bad indexes");
      } else {
        auto* v = vertices.get();
        auto* n = normals.get();
        auto* t = texcoords.get();

        auto v_ = v[i];
        auto n_ = n[i];
        auto t_ = t[i];

        v[i] = v[j];
        n[i] = n[j];
        t[i] = t[j];

        v[j] = v_;
        n[j] = n_;
        t[j] = t_;
      }
    }
    short int compare(const int i, const int j, const int axis=X_DISTANCE) const {
      if(i < 0 || j < 0 || i >= size_triangles || j >= size_triangles) {
        throw std::runtime_error("bad indexes");
      } else {
        auto a = getDistance(i, axis);
        auto b = getDistance(j, axis);
        if(a < b) return -1;
        if(a == b) return 0;
        if(a > b) return 1;
      }
    }
    double getDistance(const int i, const int axis=X_DISTANCE) const {
      if(i < 0 || i + 2 >= size_triangles) {
        throw std::runtime_error("bad index!");
      }
      auto* v = vertices.get();
      auto x = abs(v[i]);
      auto y = abs(v[i+1]);
      auto z = abs(v[i+2]);

      if(axis == X_DISTANCE) x = 0xffff;
      if(axis == Y_DISTANCE) y = 0xffff;
      if(axis == Z_DISTANCE) z = 0xffff;

      auto min = x < y ? x : y;
      min = min < z ? min : z;
      return min;
    }
  };

  std::vector<Mesh> meshes;

  void
  Clear() override;

  void
  Create() override;

  void
  Render() override;
};

TriangleArrayObjects*
ReadAsArrayObjects(const std::string& inputfile);
