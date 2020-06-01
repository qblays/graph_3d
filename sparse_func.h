#ifndef MSR_MATRIX
#define MSR_MATRIX
#define MAX_NZ 6

#include "global_defines.h"

int get_links (int n, int m, int i, int j, int x[], int y[]);

int get_num_links (int n, int m, int i, int j);

int get_matrix_len (int n, int m);

int get_matrix_size (int n, int m);

int create_matrix_structure (int n, int m, int *res_I);

int create_matrix_values (int n, int m, int matrix_size, double *a, int *I, int p, int k);

int get_num_links (int n, int m);

//int get_index_by_i_j (int i, int j, int n, int m, int n_cut, int m_cut);

inline void get_i_j_by_index (int index, int &i, int &j, int /*n*/, int m)
{
  i = (index) / (m + 1);
  j = (index) - i * (m + 1);
}

int get_func_values_neighbourhood (int i, int j, int n, int m, func2d &func,
                                   double close_vals[], double edge_vals[], double vals[], grid_data grid);

int get_lin_func_value (double x, double y, int n, int m, double *vals, double &res);

void create_rhs (int n, int m, int matrix_size, func2d f, double *rhs,
                 grid_data grid, int p, int k);

inline double func_scalar_product_basis (double close_vals[], double edge_vals[], double vals[])
{
  double sum_close = 0.;
  double sum_edges = 0.;
  double sum = 0.;
  for (int i = 0; i < MAX_NZ; i++)
    {
      sum_close += close_vals[i];
      sum_edges += edge_vals[i];
      sum += vals[i];
    }
  return 0.25 * (3. / 2 * vals[MAX_NZ] + 5. / 6 * sum_close + 1. / 6 * sum_edges + 1. / 12 * sum);
}

void MSR_mtx_mult_vector (double *A, int *I, int n,
                         double *x, double *y, int p, int k);

void MSR_apply_Jacobi (double *a, int n, double *u, double *r, int p, int k);

int MSR_solve (double *a, int *I, int n, double *x, double *b, double *r, double *u,
               double *v, double eps, int maxit, int p, int k);

void lin_comb (double *x, double *y, double t, int n, int p, int k);

double scalar_prod (double *x, double *y, int n, int p, int k);

double MSR_residual (int n, int m, double *val, func2d f, double &max, grid_data grid, int p, int k);

#endif // MSR_MATRIX
