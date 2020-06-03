#include <QVector3D>
#include <QVector4D>
#include <cmath>

#include "matrix_func.h"
#include "geometry_data.h"
#include "sparse_func.h"
#include "surface.h"

#define RESOLUTION 1

static inline int
get_triangle_pos_in_buf_by_i_j (grid_data &grid, int i, int j)
{
  // int m = grid.m;
  // int pos = (i * m + j) * 6;
  // return pos;

  //  int n = grid.n;
  int m = grid.m;
  int pos = (i * m + j);
  //    int A = ::A+1;
  //    int B = ::B-1;
  if (i >= A && i < B)
    {
      pos -= (i - A) * (B - A);
      if (j >= A && j < B)
        abort ();
      pos -= j >= B ? B - A : 0;
    }
  if (i >= B)
    {
      pos -= (B - A) * (B - A);
    }
  //  printf ("for ij=(%d, %d) pos = %d\n", i, j, pos);
  //  fflush (stdout);
  return pos * 6;
}

void
surface::update_ij (int i, int j, vector_4d &vals)
{
  vector_3d *a, *b, *c;
  int pos = get_triangle_pos_in_buf_by_i_j (m_grid, i, j);
  a = m_geom_ptr->point_at (pos);
  b = m_geom_ptr->point_at (pos + 1);
  c = m_geom_ptr->point_at (pos + 2);
  a->setZ (vals.x);
  b->setZ (vals.y);
  c->setZ (vals.z);
  vector_3d normal = vector_3d::normal (*a, *b, *c);
  *(m_geom_ptr->normal_at (pos + 0)) = normal;
  *(m_geom_ptr->normal_at (pos + 1)) = normal;
  *(m_geom_ptr->normal_at (pos + 2)) = normal;

  a = m_geom_ptr->point_at (pos + 3);
  b = m_geom_ptr->point_at (pos + 4);
  c = m_geom_ptr->point_at (pos + 5);
  a->setZ (vals.x);
  b->setZ (vals.z);
  c->setZ (vals.w);
  normal = vector_3d::normal (*a, *b, *c);
  *(m_geom_ptr->normal_at (pos + 3)) = normal;
  *(m_geom_ptr->normal_at (pos + 4)) = normal;
  *(m_geom_ptr->normal_at (pos + 5)) = normal;
}

surface::surface (const grid_data &grid, func2d &f) : m_grid (grid)
{
  m_geom_ptr = new geometry_data ();


  m_grid.n = 64;
  m_grid.m = 64;
  update_stretch (m_grid.n);

  point_numb = get_matrix_size (m_grid.n, m_grid.m);

  double r1 = std::max (fabs (grid.u.x ()), fabs (grid.C.x ()));
  double r2 = std::max (fabs (grid.v.y ()), fabs (grid.C.y ()));
  double dx_i = r1 / m_grid.n;
  double dy_i = 0. / m_grid.m;
  double dx_j = 0. / m_grid.n;
  double dy_j = r2 / m_grid.m;

  bool is_first = true;
  double f0, f1, f2, f3;
  int n = m_grid.n, m = m_grid.m;
  for (int i = 0; i < n; i++)
    {
      for (int j = 0; j < m; j++)
        {
          dx_i = r1 / m_grid.n;
          dy_i = 0. / m_grid.m;
          dx_j = 0. / m_grid.n;
          dy_j = r2 / m_grid.m;
          double new_x, new_y;

          double x = i * dx_i + j * dx_j;
          double y = i * dy_i + j * dy_j;

          if (i == A - 1)
            {
              dx_i += A_s;
            }
          if (j == A - 1)
            {
              dy_j += A_s;
            }
          if (i == A)
            {
              dx_i -= A_s;
              x += A_s;
            }
          if (j == A)
            {
              dy_j -= A_s;
              y += A_s;
            }

          if (i == B - 1)
            {
              dx_i -= B_s;
            }
          if (j == B - 1)
            {
              dy_j -= B_s;
            }
          if (i == B)
            {
              dx_i += B_s;
              x -= B_s;
            }
          if (j == B)
            {
              dy_j += B_s;
              y -= B_s;
            }
          translate_tetragon (x, y, new_x, new_y, m_grid.revJacobi);
          f0 = f (new_x, new_y);
          translate_tetragon (x + dx_i, y + dy_i, new_x, new_y,
                              m_grid.revJacobi);
          f1 = f (new_x, new_y);
          translate_tetragon (x + dx_i + dx_j, y + dy_i + dy_j, new_x, new_y,
                              m_grid.revJacobi);
          f2 = f (new_x, new_y);
          translate_tetragon (x + dx_j, y + dy_j, new_x, new_y,
                              m_grid.revJacobi);
          f3 = f (new_x, new_y);

          //
          if (i >= A && i < B && j >= A && j < B)
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

          for (double fi : {f0, f1, f2, f3})
            {
              if (fi > max_val)
                max_val = fi;
              if (fi < min_val)
                min_val = fi;
            }

          translate_tetragon (x, y, new_x, new_y, m_grid.revJacobi);
          vector_3d a (new_x, new_y, f0);
          translate_tetragon (x + dx_i, y + dy_i, new_x, new_y,
                              m_grid.revJacobi);
          vector_3d b (new_x, new_y, f1);
          translate_tetragon (x + dx_i + dx_j, y + dy_i + dy_j, new_x, new_y,
                              m_grid.revJacobi);
          vector_3d c (new_x, new_y, f2);
          translate_tetragon (x + dx_j, y + dy_j, new_x, new_y,
                              m_grid.revJacobi);
          vector_3d d (new_x, new_y, f3);
          m_geom_ptr->add_triangle (a, b, c);
          m_geom_ptr->add_triangle (a, c, d);
        }
    }
}

surface::surface (const surface &obj)
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

void
surface::draw (bool is_resid, vector_3d scaling)
{
  if (m_geom_ptr)
    {

      m_geom_ptr->draw (is_resid, scaling);
//      m_geom_ptr->draw_lines (is_resid);
    }
}
