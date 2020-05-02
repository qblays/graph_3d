#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <cstdlib>
#include <QGLWidget>

class QVector3D;

class geometry_data
{
public:
  void draw (bool is_resid) const;
  void add_triangle (const QVector3D &a, const QVector3D &b, const QVector3D &c);
  QVector3D *point_at (int pos);
  QVector3D *normal_at (int pos);

private:
  void append (const QVector3D &a, const QVector3D &n);
  std::vector<GLuint> m_faces;
  std::vector<QVector3D> m_vertices;
  std::vector<QVector3D> m_normals;
};

#endif // GEOMETRY_H
