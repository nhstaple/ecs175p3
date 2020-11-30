//
// Created by nick on 11/30/2020.
//

#include "geometry_triangle.h"
extern TriangleArrayObjects* objects;

// sort this mesh with the swap
void
PainterAlgorithm(const TriangleArrayObjects::Mesh& mesh, const int axis=X_DISTANCE)
{
  return;
  const auto n = mesh.size_triangles;
  bool swapped = false;
  TriangleArrayObjects::Mesh m; m = mesh;

  do {
    swapped = false;
    for(int i = 1; i < n; i++) {
      if (mesh.compare(i - 1, i, axis) > 0) {
        m.swap(i - 1, i);
        swapped = true;
      }
    }
  } while(!swapped);

  objects->Render();
}