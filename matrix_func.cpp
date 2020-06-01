#include "matrix_func.h"
#include <QPointF>
#include <algorithm>
#include <cmath>
#include <cstdio>

static inline void
add_str (double *a, int n, int j, int i, int l, double p)
{
  int s;
  for (s = l; s + 9 < n; s += 9)
    {
      a[j * n + s + 0] += p * a[i * n + s + 0];
      a[j * n + s + 1] += p * a[i * n + s + 1];
      a[j * n + s + 2] += p * a[i * n + s + 2];
      a[j * n + s + 3] += p * a[i * n + s + 3];
      a[j * n + s + 4] += p * a[i * n + s + 4];
      a[j * n + s + 5] += p * a[i * n + s + 5];
      a[j * n + s + 6] += p * a[i * n + s + 6];
      a[j * n + s + 7] += p * a[i * n + s + 7];
      a[j * n + s + 8] += p * a[i * n + s + 8];
    }
  for (int k = s; k < n; k++)
    a[j * n + k] += p * a[i * n + k];
}

void
build_homography_from_quad (double x1, double y1, double x2, double y2,
                            double x3, double y3, double x4, double y4,
                            double *Mat)
{
  Mat[2] = (y4 - y3) * (x1 + x3 - x2 - x4) - (x4 - x3) * (y1 + y3 - y2 - y4);
  Mat[2] /= (y4 - y3) * (x2 - x3) - (x4 - x3) * (y2 - y3);
  Mat[5] = (x1 + x3 - x2 - x4) - (x2 - x3) * Mat[2];
  Mat[5] /= x4 - x3;
  Mat[8] = 1.0;
  Mat[0] = x2 * Mat[2] + x2 - x1;
  Mat[3] = x4 * Mat[5] + x4 - x1;
  Mat[6] = x1;
  Mat[1] = y2 * Mat[2] + y2 - y1;
  Mat[4] = y4 * Mat[5] + y4 - y1;
  Mat[7] = y1;
}

void
build_homography_to_rect (double x1, double y1, double x2, double y2, double x3,
                          double y3, double x4, double y4, double *Mat,
                          double r1, double r2)
{
  (void)x1;
  (void)y1;
  (void)y2;
  (void)x4;
  Mat[2] = y3 - y4;
  Mat[2] /= x3 * y4;
  Mat[5] = x3 - x2;
  Mat[5] /= x2 * y3;
  Mat[8] = 1.0;
  Mat[0] = (x2 * Mat[2] + 1) * r1 / x2;
  Mat[3] = 0.;
  Mat[6] = 0.;
  Mat[1] = 0.;
  Mat[4] = (y4 * Mat[5] + 1) * r2 / y4;
  Mat[7] = 0.;
}

int
mtx_reverse (double *mtx, double *buf, int n, double eps)
{
  double x = 0.;
  for (int i = 0; i < n; i++)
    {
      for (int j = 0; j < n; j++)
        buf[i * n + j] = 0.;
      buf[i * n + i] = 1.;
    }
  for (int i = 0; i < n; i++)
    {
      double max = 0.;
      int m = i;
      for (int k = i; k < n; k++)
        {
          x = fabs (mtx[k * n + i]);
          if (x > max)
            {
              max = x;
              m = k;
            }
        }
      if (m != i)
        {
          for (int k = 0; k < n; k++)
            {
              double tmp1 = mtx[i * n + k];
              double tmp2 = buf[i * n + k];
              mtx[i * n + k] = mtx[m * n + k];
              buf[i * n + k] = buf[m * n + k];
              mtx[m * n + k] = tmp1;
              buf[m * n + k] = tmp2;
            }
        }
      if (fabs (mtx[i * n + i]) < eps)
        {
          return -1;
        }
      x = 1. / mtx[i * n + i];
      mtx[i * n + i] = 1.;
      for (int j = i + 1; j < n; j++)
        mtx[i * n + j] *= x;
      for (int j = 0; j < n; j++)
        buf[i * n + j] *= x;
      for (int j = i + 1; j < n; j++)
        {
          x = -mtx[j * n + i];
          add_str (mtx, n, j, i, i + 1, x);
          mtx[j * n + i] = 0.;
          add_str (buf, n, j, i, 0, x);
        }
    }
  for (int i = n - 1; i > 0; i--)
    {
      for (int j = i - 1; j >= 0; j--)
        {
          x = -mtx[j * n + i];
          add_str (buf, n, j, i, 0, x);
          mtx[j * n + i] = 0.;
        }
    }
  return 0;
}

double
det_3x3 (double *Mat)
{
  double det = 0.;
  det += Mat[0] * Mat[4] * Mat[8];
  det += Mat[3] * Mat[7] * Mat[2];
  det += Mat[1] * Mat[5] * Mat[6];
  det -= Mat[6] * Mat[4] * Mat[2];
  det -= Mat[0] * Mat[5] * Mat[7];
  det -= Mat[1] * Mat[3] * Mat[8];
  return det;
}

void
translate_tetragon (double x, double y, double &new_x, double &new_y,
                    double *Mat)
{
  double div = Mat[2] * x + Mat[5] * y + Mat[8];
  new_x = (Mat[0] * x + Mat[3] * y + Mat[6]) / div;
  new_y = (Mat[1] * x + Mat[4] * y + Mat[7]) / div;
}



void
translate_tetragon (QPointF &p, QPointF &new_p, double *Mat)
{
  double x = p.x (), y = p.y ();

  double div = Mat[2] * x + Mat[5] * y + Mat[8];
  new_p.setX ((Mat[0] * x + Mat[3] * y + Mat[6]) / div);
  new_p.setY ((Mat[1] * x + Mat[4] * y + Mat[7]) / div);
}

void
translate_tetragon (QPointF &p, QPointF &new_p, Linear_map_2d &map)
{
  new_p.setX (map (p.x (), p.y ())[0]);
  new_p.setY (map (p.x (), p.y ())[1]);
}

void
print_mtx_3x3 (double *Mat)
{
  printf ("---------\n");
  printf ("%.3f %.3f %.3f\n", Mat[0], Mat[1], Mat[2]);
  printf ("%.3f %.3f %.3f\n", Mat[3], Mat[4], Mat[5]);
  printf ("%.3f %.3f %.3f\n", Mat[6], Mat[7], Mat[8]);
  printf ("---------\n");
}
