#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <cstdlib>
#include <QGLWidget>
#include "vector3d_d.h"

class QVector3D;

class geometry_data
{
public:
  void draw (bool is_resid) const;
  void add_triangle (const vector_3d &a, const vector_3d &b, const vector_3d &c);
  vector_3d *point_at (int pos);
  vector_3d *normal_at (int pos);

  void draw_lines(bool is_resid) const;
private:
  void append (const vector_3d &a, const vector_3d &n);
  std::vector<GLuint> m_faces;
  std::vector<vector_3d> m_vertices;
  std::vector<vector_3d> m_normals;
};

#endif // GEOMETRY_H
