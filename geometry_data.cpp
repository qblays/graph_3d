#include "geometry_data.h"
#include <QVector3D>

void geometry_data::draw () const
{
  glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
  glVertexPointer (3, GL_FLOAT, 0, m_vertices.data ());
  glNormalPointer (GL_FLOAT, 0, m_normals.data ());
  const GLuint *indices = m_faces.data ();
  glDrawElements (GL_TRIANGLES, m_faces.size (), GL_UNSIGNED_INT, indices);
}

void geometry_data::append (const QVector3D &a, const QVector3D &n)
{
  int v = m_vertices.size ();
  m_vertices.push_back (a);
  m_normals.push_back (n);
  m_faces.push_back (v);
}

void geometry_data::add_triangle (const QVector3D &a, const QVector3D &b, const QVector3D &c)
{
  QVector3D norm = QVector3D::normal (a, b, c);
  append (a, norm);
  append (b, norm);
  append (c, norm);
}

QVector3D *geometry_data::point_at (int pos)
{
  return &(m_vertices[pos]);
}

QVector3D *geometry_data::normal_at (int pos)
{
  return &(m_normals[pos]);
}
