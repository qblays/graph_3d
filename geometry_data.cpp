#include "geometry_data.h"
#include <QVector3D>
#include "vector3d_d.h"

void
geometry_data::draw (bool is_resid) const
{
  glColor4ub (180, 0, 0, 1);
  if (is_resid)
    glPolygonMode (GL_FRONT_AND_BACK, GL_POINT);
  else
    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
  glVertexPointer (3, GL_DOUBLE, 0, m_vertices.data ());
  glNormalPointer (GL_DOUBLE, 0, m_normals.data ());
  const GLuint *indices = m_faces.data ();
  //  printf ("size = %lu %lu %lu\n", m_faces.size (), m_vertices.size(),
  //  m_normals.size()); fflush (stdout);
  glDrawElements (GL_TRIANGLES, m_faces.size (), GL_UNSIGNED_INT, indices);
}
void
geometry_data::draw_lines (bool is_resid) const
{
  glColor4ub (150, 50, 50, 1);
  if (is_resid)
    {
      glPolygonMode (GL_FRONT_AND_BACK, GL_POINT);
    }
  else
    glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
  glVertexPointer (3, GL_DOUBLE, 0, m_vertices.data ());
  glNormalPointer (GL_DOUBLE, 0, m_normals.data ());
  const GLuint *indices = m_faces.data ();
  //  printf ("size = %lu %lu %lu\n", m_faces.size (), m_vertices.size(),
  //  m_normals.size()); fflush (stdout);
  glDrawElements (GL_TRIANGLES, m_faces.size (), GL_UNSIGNED_INT, indices);
}

void
geometry_data::append (const vector_3d &a, const vector_3d &n)
{
  int v = m_vertices.size ();
  m_vertices.push_back (a);
  m_normals.push_back (n);
  m_faces.push_back (v);
}

void
geometry_data::add_triangle (const vector_3d &a, const vector_3d &b,
                             const vector_3d &c)
{
  vector_3d norm = vector_3d::normal (a, b, c);
  append (a, norm);
  append (b, norm);
  append (c, norm);
}

vector_3d *
geometry_data::point_at (int pos)
{
  return &(m_vertices[pos]);
}

vector_3d *
geometry_data::normal_at (int pos)
{
  return &(m_normals[pos]);
}
