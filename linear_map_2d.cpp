#include "linear_map_2d.h"

Linear_map_2d::Linear_map_2d (double m[4])
{
  this->matrix[0] = m[0];
  this->matrix[1] = m[1];
  this->matrix[2] = m[2];
  this->matrix[3] = m[3];
}

Linear_map_2d
Linear_map_2d::parallelogram_to_quad (double a, double b)
{
  double m[4];
  m[1] = (1. - a) / (b - a);
  m[3] = -a / (b - a);
  m[0] = 1 - m[1];
  m[2] = 1 - m[3];
  return Linear_map_2d (m);
}

Linear_map_2d
Linear_map_2d::quad_to_parallelogram (double a, double b)
{
  double m[4];
  Linear_map_2d x = parallelogram_to_quad (a, b);
  double *mat = x.matrix.data ();
  double det = mat[0] * mat[3] - mat[1] * mat[2];
  m[0] = mat[3];
  m[3] = mat[0];
  m[1] = -mat[1];
  m[2] = -mat[2];
  for (auto &x : m)
    {
      x /= det;
    }
  return Linear_map_2d (m);
}


void
Linear_map_2d::map (double x, double y, double &q, double &w)
{
  auto a = this->operator() (x, y);
  q = a[0];
  w = a[1];
}
