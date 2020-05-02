#include <QVector3D>
#include <QVector4D>
#include <cmath>

#include "sparse_tools.h"
#include "surface.h"
#include "geometry_data.h"
#include "basic_matrix_tools.h"

#define RESOLUTION 150

static inline int get_triangle_pos_in_buf_by_i_j (grid_data &grid, int i, int j)
{
  //int m = grid.m;
  //int pos = (i * m + j) * 6;
  //return pos;

  int pos = 0;
  int n = grid.n;
  int m = grid.m;
  if (i <= n / HOLE_SIZE)
    {
      pos = i * m + j;
    }
  else if (i >= n - n / HOLE_SIZE)
    {
      //pos = i * m + j - (n - 2 * n / HOLE_SIZE) * (m - 2 * m / HOLE_SIZE);
      pos = (i + 1 - (n - 2 * n / HOLE_SIZE)) * m + (n - 1 - 2 * n / HOLE_SIZE) * (1 + 2 * m / HOLE_SIZE) + j;
      //pos = (i - (n - 2 * n / HOLE_SIZE)) * m + (n - 2 * n / HOLE_SIZE) * (2 * m / HOLE_SIZE) + j;
    }
  else
    {
      //pos = 0;
      pos = (n / HOLE_SIZE + 1) * m + (1 + 2 * m / HOLE_SIZE) * (i - (1 + n / HOLE_SIZE)) + j;
      if (j >= m - m / HOLE_SIZE)
        pos -= (m - 1 - 2 * m / HOLE_SIZE);
        //pos -= (-1 + 2 * m / HOLE_SIZE);
    }
  return pos * 6;
}

void surface::update_ij (int i, int j, QVector4D &vals)
{
  QVector3D *a, *b, *c;
  int pos = get_triangle_pos_in_buf_by_i_j (m_grid, i, j);
  a = m_geom_ptr->point_at (pos);
  b = m_geom_ptr->point_at (pos + 1);
  c = m_geom_ptr->point_at (pos + 2);
  a->setZ (vals.x ());
  b->setZ (vals.y ());
  c->setZ (vals.z ());
  QVector3D normal = QVector3D::normal (*a, *b, *c);
  *(m_geom_ptr->normal_at (pos + 0)) = normal;
  *(m_geom_ptr->normal_at (pos + 1)) = normal;
  *(m_geom_ptr->normal_at (pos + 2)) = normal;

  a = m_geom_ptr->point_at (pos + 3);
  b = m_geom_ptr->point_at (pos + 4);
  c = m_geom_ptr->point_at (pos + 5);
  a->setZ (vals.x ());
  b->setZ (vals.z ());
  c->setZ (vals.w ());
  normal = QVector3D::normal (*a, *b, *c);
  *(m_geom_ptr->normal_at (pos + 3)) = normal;
  *(m_geom_ptr->normal_at (pos + 4)) = normal;
  *(m_geom_ptr->normal_at (pos + 5)) = normal;
}

surface::surface (const grid_data &grid, func2d &f): m_grid (grid)
{
  m_geom_ptr = new geometry_data ();

  double ratio;
  if (grid.n > RESOLUTION)
    {
      m_grid.n = grid.n;
    }
  else
    {
      ratio = (RESOLUTION * 1.) / grid.n;
      m_grid.n = grid.n * (int) (pow (2., ceil (log2 (ratio))));
    }

  if (grid.m > RESOLUTION)
    {
      m_grid.m = grid.m;
    }
  else
    {
      ratio = (RESOLUTION * 1.) / grid.m;
      m_grid.m = grid.m * (int) (pow (2., ceil (log2 (ratio))));
    }

  point_numb = get_matrix_size (m_grid.n, m_grid.m);
  //point_numb = (m_grid.m + 1) * (m_grid.n + 1) - (m_grid.n - 1 - 2 * m_grid.n / HOLE_SIZE) * (m_grid.m - 1 - 2 * m_grid.m / HOLE_SIZE);

  double r1 = std::max (fabs (grid.u.x()), fabs (grid.C.x()));
  double r2 = std::max (fabs (grid.v.y()), fabs (grid.C.y()));
  float dx_i = r1 / m_grid.n;
  float dy_i = 0. / m_grid.m;
  float dx_j = 0. / m_grid.n;
  float dy_j = r2 / m_grid.m;

  bool is_first = true;
  float f0, f1, f2, f3;
  int n = m_grid.n, m = m_grid.m;
  for (int i = 0; i < n; i++)
    {
      for (int j = 0; j < m; j++)
        {
          double new_x, new_y;

          float x = i * dx_i + j * dx_j;
          float y = i * dy_i + j * dy_j;

          translate_tetragon (x, y, new_x, new_y, m_grid.revJacobi);
          f0 = (float) f (new_x, new_y);
          translate_tetragon (x + dx_i, y + dy_i, new_x, new_y, m_grid.revJacobi);
          f1 = (float) f (new_x, new_y);
          translate_tetragon (x + dx_i + dx_j, y + dy_i + dy_j, new_x, new_y, m_grid.revJacobi);
          f2 = (float) f (new_x, new_y);
          translate_tetragon (x + dx_j, y + dy_j, new_x, new_y, m_grid.revJacobi);
          f3 = (float) f (new_x, new_y);

          //
          if (i > n / HOLE_SIZE && i < (n - n / HOLE_SIZE) && j > m / HOLE_SIZE && j < (m - m / HOLE_SIZE))
            {
              continue;
            }
          //

          if (is_first)
            {
              max_val = f0;
              min_val = f0;
              is_first = false;
            }

          for (float fi: {f0, f1, f2, f3})
            {
              if (fi > max_val)
                max_val = fi;
              if (fi < min_val)
                min_val = fi;
            }

          translate_tetragon (x, y, new_x, new_y, m_grid.revJacobi);
          QVector3D a (new_x, new_y, f0);
          translate_tetragon (x + dx_i, y + dy_i, new_x, new_y, m_grid.revJacobi);
          QVector3D b (new_x, new_y, f1);
          translate_tetragon (x + dx_i + dx_j, y + dy_i + dy_j, new_x, new_y, m_grid.revJacobi);
          QVector3D c (new_x, new_y, f2);
          translate_tetragon (x + dx_j, y + dy_j, new_x, new_y, m_grid.revJacobi);
          QVector3D d (new_x, new_y, f3);
          m_geom_ptr->add_triangle (a, b, c);
          m_geom_ptr->add_triangle (a, c, d);
        }
    }
}

surface::surface (const surface& obj)
{
  max_val = obj.max_val;
  min_val = obj.min_val;
  m_grid = obj.m_grid;
  m_geom_ptr = new geometry_data (*(obj.m_geom_ptr));
  point_numb = obj.point_numb;
}

surface::~surface ()
{
  if (m_geom_ptr)
    delete m_geom_ptr;
}

void surface::draw (bool is_resid)
{
  if (m_geom_ptr)
    m_geom_ptr->draw (is_resid);
}


