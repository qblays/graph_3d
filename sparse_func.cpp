#include <cmath>

#include "matrix_func.h"
#include "sparse_func.h"
#include "threading.h"
#include <cstdio>

#define OUTPUT 16
#define MAX_NZ 6
//#define EPS 1e-14

static inline void
split_threads (int n, int p, int k, int &i1, int &i2)
{
  i1 = n * k;
  i1 /= p;
  i2 = n * (k + 1);
  i2 /= p;
  i2--;
}

inline double
f_by_ij (double i, double j, int n, int m, func2d &f, grid_data grid)
{
  // double r1 = std::max (fabs (grid.u.x()), fabs (grid.C.x()));
  // double r2 = std::max (fabs (grid.v.y()), fabs (grid.C.y()));
  double hx = 1. / n;
  double hy = 1. / m;
  double x = hx * i;
  double y = hy * j;
  double new_x, new_y;
  translate_tetragon (x, y, new_x, new_y, grid.revJacobi);
  double addition = 0.1 * (double_eq (i, 0.)) * double_eq (j, n / 2) * P;
  return f (new_x, new_y) + addition;
}

static inline int
get_index_by_i_j (int i, int j, int, int m)
{
  return i * (m + 1) + j;
}

static inline void
vector_product (double v[], double u[], double r[])
{
  r[0] = u[1] * v[2] - u[2] * v[1];
  r[1] = u[2] * v[0] - u[0] * v[2];
  r[2] = u[0] * v[1] - u[1] * v[0];
}

int
get_links (int n, int m, int i, int j, int x[], int y[])
{
  if ((0 < j && j < m && 0 < i && i < n))
    {
      x[0] = i;
      y[0] = j + 1;
      x[1] = i + 1;
      y[1] = j + 1;
      x[2] = i + 1;
      y[2] = j;
      x[3] = i;
      y[3] = j - 1;
      x[4] = i - 1;
      y[4] = j - 1;
      x[5] = i - 1;
      y[5] = j;
      return 6;
    }
  if (i == 0 && 0 < j && j < m)
    {
      x[0] = i;
      y[0] = j + 1;
      x[1] = i + 1;
      y[1] = j + 1;
      x[2] = i + 1;
      y[2] = j;
      x[3] = i;
      y[3] = j - 1;
      return 4;
    }
  if (0 < i && i < n && j == 0)
    {
      x[0] = i;
      y[0] = j + 1;
      x[1] = i + 1;
      y[1] = j + 1;
      x[2] = i + 1;
      y[2] = j;
      x[3] = i - 1;
      y[3] = j;
      return 4;
    }
  if (0 < i && i < n && j == m)
    {
      x[0] = i + 1;
      y[0] = j;
      x[1] = i;
      y[1] = j - 1;
      x[2] = i - 1;
      y[2] = j - 1;
      x[3] = i - 1;
      y[3] = j;
      return 4;
    }
  if (i == n && 0 < j && j < m)
    {
      x[0] = i;
      y[0] = j + 1;
      x[1] = i;
      y[1] = j - 1;
      x[2] = i - 1;
      y[2] = j - 1;
      x[3] = i - 1;
      y[3] = j;
      return 4;
    }
  if (i == 0 && j == 0)
    {
      x[0] = i;
      y[0] = j + 1;
      x[1] = i + 1;
      y[1] = j + 1;
      x[2] = i + 1;
      y[2] = j;
      return 3;
    }
  if (i == n && j == m)
    {
      x[0] = i;
      y[0] = j - 1;
      x[1] = i - 1;
      y[1] = j - 1;
      x[2] = i - 1;
      y[2] = j;
      return 3;
    }

  if (i == 0 && j == m)
    {
      x[0] = i + 1;
      y[0] = j;
      x[1] = i;
      y[1] = j - 1;
      return 2;
    }

  if (j == 0 && i == n)
    {
      x[0] = i;
      y[0] = j + 1;
      x[1] = i - 1;
      y[1] = j;
      return 2;
    }
  //  fprintf (stderr, "error\n");
  abort ();
  return -1000;
}

int
get_num_links (int n, int m)
{
  int sum = 0;
  for (int i = 0; i <= n; i++)
    {
      for (int j = 0; j <= m; j++)
        sum += get_num_links (n, m, i, j);
    }
  return sum;
}

int
get_num_links (int n, int m, int i, int j)
{
  if ((i == 0 && 0 < j && j < m) || (j == m && 0 < i && i < n) ||
      (i == n && 0 < j && j < m) || (j == 0 && 0 < i && i < n))
    return 4;
  if ((j == 0 && i == 0) || (i == n && j == m))
    return 3;
  if ((i == 0 && j == m) || (j == 0 && i == n))
    return 2;
  if ((0 < j && j < m && 0 < i && i < n) || (0 < i && i < n && 0 < j && j < m))
    return 6;
  //  fprintf (stderr, "error\n");
  abort ();
  return -1000;
}

int
get_matrix_size (int n, int m)
{
  return (m + 1) * (n + 1);
}

int
get_matrix_len (int n, int m)
{
  int diag = get_matrix_size (n, m);
  int offdiag = get_num_links (n, m);
  return diag + offdiag + 1;
}

int
create_matrix_structure (int n, int m, int *res_I)
{
  int N = get_matrix_size (n, m);
  int N_l = get_num_links (n, m);
  int len = N + 1 + N_l;

  int x[MAX_NZ], y[MAX_NZ];
  int pos = N + 1;
  int k = 0;
  for (int i = 0; i <= n; i++)
    {
      for (int j = 0; j <= m; j++)
        {
          res_I[k] = pos;
          int l = get_links (n, m, i, j, x, y);
          for (int q = 0; q < l; q++)
            {
              int index = get_index_by_i_j (x[q], y[q], n, m);
              res_I[pos + q] = index;
            }
          k++;
          pos += l;
        }
    }
  res_I[N] = len;
  if (pos != len)
    {
      fprintf (stderr, "FATAL ERROR IN create_matrix_structure\n");
      abort ();
      return -1000;
    }
  return 0;
}

int
create_matrix_values (int n, int m, int matrix_size, double *a, int *I, int p,
                      int k)
{
  int l1, l2;
  split_threads (matrix_size, p, k, l1, l2);

  double err = 0;
  for (int l = l1; l <= l2; l++)
    {
      int i, j;
      get_i_j_by_index (l, i, j, n, m);
      a[l] = 1.;
      int adj_size = get_num_links (n, m, i, j);
      int pos = I[l];
      for (int q = 0; q < adj_size; q++)
        {
          a[pos + q] = 1. / 6;
        }
    }
  if (err < 0)
    return -1;
  return 0;
}

int
get_func_values_neighbourhood (int i, int j, int n, int m, func2d &func,
                               double close_vals[], double edge_vals[],
                               double vals[], grid_data grid)
{
  if (0 < j && j < m && 0 < i && i < n)
    {
      vals[0] = f_by_ij (i, j + 1, n, m, func, grid);
      vals[1] = f_by_ij (i + 1, j + 1, n, m, func, grid);
      vals[2] = f_by_ij (i + 1, j, n, m, func, grid);
      vals[3] = f_by_ij (i, j - 1, n, m, func, grid);
      vals[4] = f_by_ij (i - 1, j - 1, n, m, func, grid);
      vals[5] = f_by_ij (i - 1, j, n, m, func, grid);
      vals[6] = f_by_ij (i, j, n, m, func, grid);

      close_vals[0] = f_by_ij (i, j + 0.5, n, m, func, grid);
      close_vals[1] = f_by_ij (i + 0.5, j + 0.5, n, m, func, grid);
      close_vals[2] = f_by_ij (i + 0.5, j, n, m, func, grid);
      close_vals[3] = f_by_ij (i, j - 0.5, n, m, func, grid);
      close_vals[4] = f_by_ij (i - 0.5, j - 0.5, n, m, func, grid);
      close_vals[5] = f_by_ij (i - 0.5, j, n, m, func, grid);

      edge_vals[0] = f_by_ij (i + 0.5, j + 1, n, m, func, grid);
      edge_vals[1] = f_by_ij (i + 1, j + 0.5, n, m, func, grid);
      edge_vals[2] = f_by_ij (i + 0.5, j - 0.5, n, m, func, grid);
      edge_vals[3] = f_by_ij (i - 0.5, j - 1, n, m, func, grid);
      edge_vals[4] = f_by_ij (i - 1, j - 0.5, n, m, func, grid);
      edge_vals[5] = f_by_ij (i - 0.5, j + 0.5, n, m, func, grid);
      return 6;
    }
  if (i == 0 && 0 < j && j < m)
    {
      vals[0] = f_by_ij (i, j + 1, n, m, func, grid);
      vals[1] = f_by_ij (i + 1, j + 1, n, m, func, grid);
      vals[2] = f_by_ij (i + 1, j, n, m, func, grid);
      vals[3] = f_by_ij (i, j - 1, n, m, func, grid);
      vals[6] = f_by_ij (i, j, n, m, func, grid);

      vals[5] = 0.; // extrapolate_func (vals[1], vals[0], vals[6]) * 0;
      vals[4] = 0.; // extrapolate_func (vals[2], vals[6], vals[3]) * 0;

      close_vals[0] = f_by_ij (i, j + 0.5, n, m, func, grid);
      close_vals[1] = f_by_ij (i + 0.5, j + 0.5, n, m, func, grid);
      close_vals[2] = f_by_ij (i + 0.5, j, n, m, func, grid);
      close_vals[3] = f_by_ij (i, j - 0.5, n, m, func, grid);

      close_vals[4] = (vals[4] + vals[6]) / 2.;
      close_vals[5] = (vals[5] + vals[6]) / 2.;

      edge_vals[0] = f_by_ij (i + 0.5, j + 1, n, m, func, grid);
      edge_vals[1] = f_by_ij (i + 1, j + 0.5, n, m, func, grid);
      edge_vals[2] = f_by_ij (i + 0.5, j - 0.5, n, m, func, grid);

      edge_vals[3] = (vals[3] + vals[4]) / 2.;
      edge_vals[4] = (vals[4] + vals[5]) / 2.;
      edge_vals[5] = (vals[5] + vals[0]) / 2.;
      return 4;
    }
  if (0 < i && i < n && j == 0)
    {
      vals[0] = f_by_ij (i, j + 1, n, m, func, grid);
      vals[1] = f_by_ij (i + 1, j + 1, n, m, func, grid);
      vals[2] = f_by_ij (i + 1, j, n, m, func, grid);
      vals[5] = f_by_ij (i - 1, j, n, m, func, grid);
      vals[6] = f_by_ij (i, j, n, m, func, grid);

      vals[3] = 0; // extrapolate_func (vals[1], vals[2], vals[6]) * 0;
      vals[4] = 0; // extrapolate_func (vals[0], vals[6], vals[5]) * 0;

      close_vals[0] = f_by_ij (i, j + 0.5, n, m, func, grid);
      close_vals[1] = f_by_ij (i + 0.5, j + 0.5, n, m, func, grid);
      close_vals[2] = f_by_ij (i + 0.5, j, n, m, func, grid);
      close_vals[5] = f_by_ij (i - 0.5, j, n, m, func, grid);

      close_vals[3] = (vals[6] + vals[3]) / 2.;
      close_vals[4] = (vals[6] + vals[4]) / 2.;

      edge_vals[0] = f_by_ij (i + 0.5, j + 1, n, m, func, grid);
      edge_vals[1] = f_by_ij (i + 1, j + 0.5, n, m, func, grid);
      edge_vals[5] = f_by_ij (i - 0.5, j + 0.5, n, m, func, grid);

      edge_vals[2] = (vals[2] + vals[3]) / 2.;
      edge_vals[3] = (vals[3] + vals[4]) / 2.;
      edge_vals[4] = (vals[4] + vals[5]) / 2.;
      return 4;
    }
  if (0 < i && i < n && j == m)
    {
      vals[2] = f_by_ij (i + 1, j, n, m, func, grid);
      vals[3] = f_by_ij (i, j - 1, n, m, func, grid);
      vals[4] = f_by_ij (i - 1, j - 1, n, m, func, grid);
      vals[5] = f_by_ij (i - 1, j, n, m, func, grid);
      vals[6] = f_by_ij (i, j, n, m, func, grid);

      vals[0] = 0.; // extrapolate_func (vals[4], vals[5], vals[6]) * 0;
      vals[1] = 0.; // extrapolate_func (vals[3], vals[6], vals[2]) * 0;

      close_vals[2] = f_by_ij (i + 0.5, j, n, m, func, grid);
      close_vals[3] = f_by_ij (i, j - 0.5, n, m, func, grid);
      close_vals[4] = f_by_ij (i - 0.5, j - 0.5, n, m, func, grid);
      close_vals[5] = f_by_ij (i - 0.5, j, n, m, func, grid);

      close_vals[0] = (vals[6] + vals[0]) / 2.;
      close_vals[1] = (vals[6] + vals[1]) / 2.;

      edge_vals[2] = f_by_ij (i + 0.5, j - 0.5, n, m, func, grid);
      edge_vals[3] = f_by_ij (i - 0.5, j - 1, n, m, func, grid);
      edge_vals[4] = f_by_ij (i - 1, j - 0.5, n, m, func, grid);

      edge_vals[0] = (vals[0] + vals[1]) / 2.;
      edge_vals[1] = (vals[1] + vals[2]) / 2.;
      edge_vals[5] = (vals[5] + vals[0]) / 2.;
      return 4;
    }
  if (i == n && 0 < j && j < m)
    {
      vals[0] = f_by_ij (i, j + 1, n, m, func, grid);
      vals[3] = f_by_ij (i, j - 1, n, m, func, grid);
      vals[4] = f_by_ij (i - 1, j - 1, n, m, func, grid);
      vals[5] = f_by_ij (i - 1, j, n, m, func, grid);
      vals[6] = f_by_ij (i, j, n, m, func, grid);

      vals[1] = 0.; // extrapolate_func (vals[5], vals[6], vals[0]) * 0;
      vals[2] = 0.; // extrapolate_func (vals[4], vals[3], vals[6]) * 0;

      close_vals[0] = f_by_ij (i, j + 0.5, n, m, func, grid);
      close_vals[3] = f_by_ij (i, j - 0.5, n, m, func, grid);
      close_vals[4] = f_by_ij (i - 0.5, j - 0.5, n, m, func, grid);
      close_vals[5] = f_by_ij (i - 0.5, j, n, m, func, grid);

      close_vals[1] = (vals[6] + vals[1]) / 2.;
      close_vals[2] = (vals[6] + vals[2]) / 2.;

      edge_vals[3] = f_by_ij (i - 0.5, j - 1, n, m, func, grid);
      edge_vals[4] = f_by_ij (i - 1, j - 0.5, n, m, func, grid);
      edge_vals[5] = f_by_ij (i - 0.5, j + 0.5, n, m, func, grid);

      edge_vals[0] = (vals[0] + vals[1]) / 2.;
      edge_vals[1] = (vals[1] + vals[2]) / 2.;
      edge_vals[2] = (vals[2] + vals[3]) / 2.;
      return 4;
    }
  if (i == 0 && j == 0)
    {
      vals[0] = f_by_ij (i, j + 1, n, m, func, grid);
      vals[1] = f_by_ij (i + 1, j + 1, n, m, func, grid);
      vals[2] = f_by_ij (i + 1, j, n, m, func, grid);
      vals[6] = f_by_ij (i, j, n, m, func, grid);

      vals[3] = 0.; // extrapolate_func (vals[1], vals[2], vals[6]) * 0;
      vals[5] = 0.; // extrapolate_func (vals[1], vals[0], vals[6]) * 0;
      vals[4] = 0.; //(2 * vals[6] - vals[1]) * 0;

      close_vals[0] = f_by_ij (i, j + 0.5, n, m, func, grid);
      close_vals[1] = f_by_ij (i + 0.5, j + 0.5, n, m, func, grid);
      close_vals[2] = f_by_ij (i + 0.5, j, n, m, func, grid);

      close_vals[3] = (vals[6] + vals[3]) / 2.;
      close_vals[4] = (vals[6] + vals[4]) / 2.;
      close_vals[5] = (vals[6] + vals[5]) / 2.;

      edge_vals[0] = f_by_ij (i + 0.5, j + 1, n, m, func, grid);
      edge_vals[1] = f_by_ij (i + 1, j + 0.5, n, m, func, grid);

      edge_vals[2] = (vals[2] + vals[3]) / 2.;
      edge_vals[3] = (vals[3] + vals[4]) / 2.;
      edge_vals[4] = (vals[4] + vals[5]) / 2.;
      edge_vals[5] = (vals[5] + vals[0]) / 2.;
      return 3;
    }
  if (i == n && j == m)
    {
      vals[3] = f_by_ij (i, j - 1, n, m, func, grid);
      vals[4] = f_by_ij (i - 1, j - 1, n, m, func, grid);
      vals[5] = f_by_ij (i - 1, j, n, m, func, grid);
      vals[6] = f_by_ij (i, j, n, m, func, grid);

      vals[0] = 0.; // extrapolate_func (vals[4], vals[5], vals[6]) * 0;
      vals[2] = 0.; // extrapolate_func (vals[4], vals[3], vals[6]) * 0;
      vals[1] = 0.; //(2 * vals[6] - vals[4]) * 0;

      close_vals[3] = f_by_ij (i, j - 0.5, n, m, func, grid);
      close_vals[4] = f_by_ij (i - 0.5, j - 0.5, n, m, func, grid);
      close_vals[5] = f_by_ij (i - 0.5, j, n, m, func, grid);

      close_vals[0] = (vals[6] + vals[0]) / 2.;
      close_vals[1] = (vals[6] + vals[1]) / 2.;
      close_vals[2] = (vals[6] + vals[2]) / 2.;

      edge_vals[3] = f_by_ij (i - 0.5, j - 1, n, m, func, grid);
      edge_vals[4] = f_by_ij (i - 1, j - 0.5, n, m, func, grid);

      edge_vals[0] = (vals[0] + vals[1]) / 2.;
      edge_vals[1] = (vals[1] + vals[2]) / 2.;
      edge_vals[2] = (vals[2] + vals[3]) / 2.;
      edge_vals[5] = (vals[5] + vals[0]) / 2.;
      return 3;
    }
  if (i == 0 && j == m)
    {
      vals[2] = f_by_ij (i + 1, j, n, m, func, grid);
      vals[3] = f_by_ij (i, j - 1, n, m, func, grid);
      vals[6] = f_by_ij (i, j, n, m, func, grid);

      vals[1] = 0.; // extrapolate_func (vals[3], vals[6], vals[2]) * 0;
      vals[4] = 0.; // extrapolate_func (vals[2], vals[6], vals[3]) * 0;
      vals[0] = 0.; // vals[2] * 0;
      vals[5] = 0.; // vals[3] * 0;

      close_vals[2] = f_by_ij (i + 0.5, j, n, m, func, grid);
      close_vals[3] = f_by_ij (i, j - 0.5, n, m, func, grid);

      close_vals[0] = (vals[6] + vals[0]) / 2.;
      close_vals[1] = (vals[6] + vals[1]) / 2.;
      close_vals[4] = (vals[6] + vals[4]) / 2.;
      close_vals[5] = (vals[6] + vals[5]) / 2.;

      edge_vals[2] = f_by_ij (i + 0.5, j - 0.5, n, m, func, grid);

      edge_vals[0] = (vals[0] + vals[1]) / 2.;
      edge_vals[1] = (vals[1] + vals[2]) / 2.;
      edge_vals[3] = (vals[3] + vals[4]) / 2.;
      edge_vals[4] = (vals[4] + vals[5]) / 2.;
      edge_vals[5] = (vals[5] + vals[0]) / 2.;
      return 2;
    }
  if (j == 0 && i == n)
    {
      vals[0] = f_by_ij (i, j + 1, n, m, func, grid);
      vals[5] = f_by_ij (i - 1, j, n, m, func, grid);
      vals[6] = f_by_ij (i, j, n, m, func, grid);

      vals[1] = 0.; // extrapolate_func (vals[5], vals[6], vals [0]) * 0;
      vals[4] = 0.; // extrapolate_func (vals[0], vals[6], vals [5]) * 0;
      vals[2] = 0.; // vals[0] * 0;
      vals[3] = 0.; // vals[5] * 0;

      close_vals[0] = f_by_ij (i, j + 0.5, n, m, func, grid);
      close_vals[5] = f_by_ij (i - 0.5, j, n, m, func, grid);

      close_vals[1] = (vals[6] + vals[1]) / 2.;
      close_vals[2] = (vals[6] + vals[2]) / 2.;
      close_vals[3] = (vals[6] + vals[3]) / 2.;
      close_vals[4] = (vals[6] + vals[4]) / 2.;

      edge_vals[5] = f_by_ij (i - 0.5, j + 0.5, n, m, func, grid);
      edge_vals[0] = (vals[0] + vals[1]) / 2.;
      edge_vals[1] = (vals[1] + vals[2]) / 2.;
      edge_vals[2] = (vals[2] + vals[3]) / 2.;
      edge_vals[3] = (vals[3] + vals[4]) / 2.;
      edge_vals[4] = (vals[4] + vals[5]) / 2.;
      return 2;
    }
  return -1000;
}

// double func_scalar_product_basis (double close_vals[], double edge_vals[],
// double vals[])
//{
//  double sum_close = 0.;
//  double sum_edges = 0.;
//  double sum = 0.;
//  for (int i = 0; i < MAX_NZ; i++)
//    {
//      sum_close += close_vals[i];
//      sum_edges += edge_vals[i];
//      sum += vals[i];
//    }
//  return 0.25 * (3. / 2 * vals[MAX_NZ] + 5. / 6 * sum_close + 1. / 6 *
//  sum_edges + 1. / 12 * sum);
//}

void
create_rhs (int n, int m, int matrix_size, func2d func, double *rhs,
            grid_data grid, int p, int k)
{
  double vals[MAX_NZ + 1];
  double close_vals[MAX_NZ];
  double edge_vals[MAX_NZ];

  int l1, l2;
  split_threads (matrix_size, p, k, l1, l2);

  for (int l = l1; l <= l2; l++)
    {
      int i, j;
      get_i_j_by_index (l, i, j, n, m);
      get_func_values_neighbourhood (i, j, n, m, func, close_vals, edge_vals,
                                     vals, grid);
      rhs[l] = func_scalar_product_basis (close_vals, edge_vals, vals);
    }
  reduce_sum (p);
}

int
get_lin_func_value (double x, double y, int n, int m, double *vals, double &res)
{
  int i_l = (int)(floor (x * n));
  int j_d = (int)(floor (y * m));
  if (i_l == n)
    {
      i_l = n - 1;
    }
  if (j_d == m)
    {
      j_d = m - 1;
    }
  if (i_l >= n || j_d >= m || (i_l < 0 && j_d < 0))
    {
      return -1;
    }

  double hx = 1. / n;
  double hy = 1. / m;
  double delt_x = (x - i_l * 1. / n);
  double delt_y = (y - j_d * 1. / m);

  double a[3];
  double b[3];
  double c[3];

  int i = i_l, j = j_d;
  int index = get_index_by_i_j (i, j, n, m);
  if (delt_y > delt_x) // upper triangle
    {
      a[0] = 0;
      a[1] = 0;
      a[2] = vals[index];

      j++;
      index++;
      b[0] = 0;
      b[1] = hy;
      b[2] = vals[index];

      i++;
      index = get_index_by_i_j (i, j, n, m);
      c[0] = hx;
      c[1] = hy;
      c[2] = vals[index];
    }
  else
    {
      a[0] = 0;
      a[1] = 0;
      a[2] = vals[index];

      i++;
      j++;
      index = get_index_by_i_j (i, j, n, m);
      b[0] = hx;
      b[1] = hy;
      b[2] = vals[index];

      j--;
      index--;
      c[0] = hx;
      c[1] = 0;
      c[2] = vals[index];
    }
  b[0] -= a[0];
  b[1] -= a[1];
  b[2] -= a[2];

  c[0] -= a[0];
  c[1] -= a[1];
  c[2] -= a[2];

  double prod[3];
  vector_product (b, c, prod);

  res = 1. / prod[2] * (prod[0] * (a[0] - delt_x) + prod[1] * (a[1] - delt_y)) +
        a[2];
  return 0;
}

void
MSR_mtx_mult_vector (double *A, int *I, int n, double *x, double *y, int p,
                     int k)
{
  int i1, i2;
  split_threads (n, p, k, i1, i2);

  for (int i = i1; i <= i2; i++)
    {
      double s = A[i] * x[i];
      int len = I[i + 1] - I[i];
      int offset = I[i];
      for (int j = 0; j < len; j++)
        {
          s += A[offset + j] * x[I[offset + j]];
        }
      y[i] = s;
    }
  reduce_sum (p);
}

void
MSR_apply_Jacobi (double *a, int n, double *u, double *r, int p, int k)
{
  int i1, i2;
  split_threads (n, p, k, i1, i2);

  for (int i = i1; i <= i2; i++)
    {
      u[i] = r[i] / a[i];
    }
  reduce_sum (p);
}

void
lin_comb (double *x, double *y, double t, int n, int p, int k)
{
  int i1, i2;
  split_threads (n, p, k, i1, i2);

  //  for (int i = i1; i <= i2; i++)
  //    {
  //      x[i] -= y[i] * t;
  //    }
  int i;
  for (i = i1; i <= i2 - 7; i += 8)
    {
      x[i] -= y[i] * t;
      x[i + 1] -= y[i + 1] * t;
      x[i + 2] -= y[i + 2] * t;
      x[i + 3] -= y[i + 3] * t;
      x[i + 4] -= y[i + 4] * t;
      x[i + 5] -= y[i + 5] * t;
      x[i + 6] -= y[i + 6] * t;
      x[i + 7] -= y[i + 7] * t;
    }
  for (; i <= i2; i++)
    {
      x[i] -= y[i] * t;
    }
  reduce_sum (p);
}

double
scalar_prod (double *x, double *y, int n, int p, int k)
{
  int i1, i2;
  split_threads (n, p, k, i1, i2);

  double s = 0.;
  double s1 = 0.;
  double s2 = 0.;
  double s3 = 0.;
  double s4 = 0.;
  double s5 = 0.;
  double s6 = 0.;
  double s7 = 0.;
  //  double s8 = 0.;
  //  for (int i = i1; i <= i2; i++)
  //    {
  //      s += x[i] * y[i];
  //    }
  int i = i1;
  for (i = i1; i <= i2 - 7; i += 8)
    {
      s += x[i] * y[i];
      s1 += x[i + 1] * y[i + 1];
      s2 += x[i + 2] * y[i + 2];
      s3 += x[i + 3] * y[i + 3];
      s4 += x[i + 4] * y[i + 4];
      s5 += x[i + 5] * y[i + 5];
      s6 += x[i + 6] * y[i + 6];
      s7 += x[i + 7] * y[i + 7];
    }
  for (; i <= i2; i++)
    {
      s += x[i] * y[i];
    }
  s += s1 + s2 + s3 + s4 + s5 + s6 + s7;
  reduce_sum (p, &s, 1);
  double ret = s;
  return ret;
}

int
MSR_solve (double *a, int *I, int n, double *x, double *b, double *r, double *u,
           double *v, double eps, int maxit, int p, int k)
{
  // u = Ax
  int it = 0;
  int finished = 0;
  int restarts = 0;
  while (it < maxit)
    {
      MSR_mtx_mult_vector (a, I, n, x, r, p, k);
      // r -= b
      lin_comb (r, b, 1, n, p, k);
      double res = scalar_prod (r, r, n, p, k);
      if (res < eps)
        return 0;

      double c1, c2;
      if (k == MAIN_THREAD)
        {
          //      printf ("-----------------------\n");
          //      printf ("Calculation started...\n");
        }
      int save_it = it;
      for (; it < 20 + save_it; it++)
        {
          // Mu = r
          MSR_apply_Jacobi (a, n, u, r, p, k);
          // v = Au
          MSR_mtx_mult_vector (a, I, n, u, v, p, k);
          c1 = scalar_prod (v, r, n, p, k);
          if (c1 < eps)
            {
              finished = 1;
              break;
            }

          c2 = scalar_prod (v, v, n, p, k);
          if (c2 < eps)
            {
              finished = 1;
              break;
            }
          double t = c1 / c2;
          // x -= tu;
          lin_comb (x, u, t, n, p, k);
          // r -= tv
          lin_comb (r, v, t, n, p, k);
          if (k == MAIN_THREAD)
            {
              //          printf (" iter %3.1d: c1 = %e c2 = %e\n", it, c1, c2);
            }
        }
      if (finished)
        {
          break;
        }
      restarts++;
    }

  if (k == MAIN_THREAD)
    {
      printf ("Finished at iter: %d, restarts = %d\n", it, restarts);
      //      printf ("-----------------------\n");
    }
  if (it >= maxit)
    return -1;
  return 0;
}

double
MSR_residual (int n, int m, double *vals, func2d f, double &max, grid_data grid,
              int p, int k)
{
  int matrix_size = get_matrix_size (n, m);

  int l1, l2;
  split_threads (matrix_size, p, k, l1, l2);

  int i, j;
  double x, y;
  // double r1 = std::max (fabs (grid.u.x()), fabs (grid.C.x()));
  // double r2 = std::max (fabs (grid.v.y()), fabs (grid.C.y()));
  double dx = 1. / n;
  double dy = 1. / m;
  max = 0;

  double x_c, y_c;
  for (int l = l1; l <= l2; l++)
    {
      get_i_j_by_index (l, i, j, n, m);

      if (i == n || j == m)
        {
          continue;
        }
      x = i * dx;
      y = j * dy;

      double approx_val;
      double new_x, new_y;
      double my_res = 0.;
      // translate_tetragon (x, y, new_x, new_y, grid.revJacobi);
      // x = new_x; y = new_y;

      x_c = x;
      y_c = y + dy / 2.;
      translate_tetragon (x_c, y_c, new_x, new_y, grid.revJacobi);
      get_lin_func_value (x_c, y_c, n, m, vals, approx_val);
      my_res = fabs (approx_val - f (new_x, new_y));
      max = (my_res > max) ? my_res : max;

      x_c = x + dx / 2.;
      y_c = y + dy;
      translate_tetragon (x_c, y_c, new_x, new_y, grid.revJacobi);
      get_lin_func_value (x_c, y_c, n, m, vals, approx_val);
      my_res = fabs (approx_val - f (new_x, new_y));
      max = (my_res > max) ? my_res : max;

      x_c = x + dx / 2.;
      y_c = y + dy / 2.;
      translate_tetragon (x_c, y_c, new_x, new_y, grid.revJacobi);
      get_lin_func_value (x_c, y_c, n, m, vals, approx_val);
      my_res = fabs (approx_val - f (new_x, new_y));
      max = (my_res > max) ? my_res : max;

      /// if ((i < 0 && j == 0) || (i >= 0 && j == 0))
      ///  {
      ///    x_c = x + dx / 2.;
      ///    y_c = y;
      ///    translate_tetragon (x_c, y_c, new_x, new_y, grid.revJacobi);
      ///    get_lin_func_value (x_c, y_c, n, m, 0, 0, vals, approx_val);
      ///    my_res = fabs (approx_val - f (new_x, new_y));
      ///    max = (my_res > max)? my_res : max;
      ///  }
      ///
      /// if (i == n - 1)
      ///  {
      ///    x_c = x + dx;
      ///    y_c = y + dy / 2.;
      ///    translate_tetragon (x_c, y_c, new_x, new_y, grid.revJacobi);
      ///    get_lin_func_value (x_c, y_c, n, m, 0, 0, vals, approx_val);
      ///    my_res = fabs (approx_val - f (new_x, new_y));
      ///    max = (my_res > max)? my_res : max;
      ///  }
    }
  reduce_max (p, &max, 1);
  return max;
}
