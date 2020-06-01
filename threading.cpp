#include <QVector4D>
#include <cstdio>
#include <unistd.h>

#include "matrix_func.h"
#include "sparse_func.h"
#include "surface.h"
#include "threading.h"
#include "window.h"

#define MAXIT 100

static inline void
split_threads (int n, int p, int k, int &i1, int &i2)
{
  i1 = n * k;
  i1 /= p;
  i2 = n * (k + 1);
  i2 /= p;
  i2--;
}

void
fill_surface_with_vals (surface *buf_surface, grid_data &grid_calc, int p,
                        int k, double *vals, double &max_arg, func2d func,
                        bool show_orig)
{

  int N = buf_surface->get_point_numb ();

  int i1, i2;
  split_threads (N, p, k, i1, i2);

  grid_data *grid = buf_surface->get_grid ();
  int n = grid->n;
  int m = grid->m;
  update_stretch (n);
  int n_calc = grid_calc.n;
  int m_calc = grid_calc.m;

  double dx = 1. / n;
  double dy = 1. / m;
  vector_4d vec;
  double max = -1e-31;
  for (int l = i1; l <= i2; l++)
    {
      dx = 1. / n;
      dy = 1. / m;
      int i, j;
      get_i_j_by_index (l, i, j, n, m);
      if (j >= m || i >= n)
        continue;
      double x = i * dx;
      double y = j * dy;
      if (i == A - 1)
        {
          dx += A_s;
        }
      if (j == A - 1)
        {
          dy += A_s;
        }
      if (i == A)
        {
          dx -= A_s;
          x += A_s;
        }
      if (j == A)
        {
          dy -= A_s;
          y += A_s;
        }

      if (i == B - 1)
        {
          dx -= B_s;
        }
      if (j == B - 1)
        {
          dy -= B_s;
        }
      if (i == B)
        {
          dx += B_s;
          x -= B_s;
        }
      if (j == B)
        {
          dy += B_s;
          y -= B_s;
        }
      double f0, f1, f2, f3;

      if (show_orig == 1)
        {
          double new_x, new_y;
          translate_tetragon (x, y, new_x, new_y, grid_calc.revJacobi);
          vec.setX (fabs (((*func) (new_x, new_y))));
          translate_tetragon (x + dx, y, new_x, new_y, grid_calc.revJacobi);
          vec.setY (fabs (((*func) (new_x, new_y))));
          translate_tetragon (x + dx, y + dy, new_x, new_y,
                              grid_calc.revJacobi);
          vec.setZ (fabs (((*func) (new_x, new_y))));
          translate_tetragon (x, y + dy, new_x, new_y, grid_calc.revJacobi);
          vec.setW (fabs (((*func) (new_x, new_y))));

          if (i >= A && i < B && j >= A && j < B)
            {
              continue;
            }
        }
      else
        {
          get_lin_func_value (x, y, n_calc, m_calc, vals, f0);
          get_lin_func_value (x + dx, y, n_calc, m_calc, vals, f1);
          get_lin_func_value (x + dx, y + dy, n_calc, m_calc, vals, f2);
          get_lin_func_value (x, y + dy, n_calc, m_calc, vals, f3);
          if (!func)
            {
              if (i >= A && i < B && j >= A && j < B)
                {
                  continue;
                }
              vec.setX (f0);
              vec.setY (f1);
              vec.setZ (f2);
              vec.setW (f3);
            }
          else
            {
              double new_x, new_y;
              translate_tetragon (x, y, new_x, new_y, grid_calc.revJacobi);
              vec.setX (fabs ((f0 - (*func) (new_x, new_y))));
              translate_tetragon (x + dx, y, new_x, new_y, grid_calc.revJacobi);
              vec.setY (fabs ((f1 - (*func) (new_x, new_y))));
              translate_tetragon (x + dx, y + dy, new_x, new_y,
                                  grid_calc.revJacobi);
              vec.setZ (fabs ((f2 - (*func) (new_x, new_y))));
              translate_tetragon (x, y + dy, new_x, new_y, grid_calc.revJacobi);
              vec.setW (fabs ((f3 - (*func) (new_x, new_y))));

              if (i >= A && i < B && j >= A && j < B)
                {
                  continue;
                }
            }
        }
      double l_max = std::max ({vec.x, vec.y, vec.z, vec.w});
      if (l_max > max)
        {
          max = l_max;
        }

      buf_surface->update_ij (i, j, vec);
    }
  max_arg = max;
  reduce_max (p, &max, 1);
}

void *
thread_func (void *arg)
{
  thread_info *my_arg = static_cast<thread_info *> (arg);
  //  printf ("Thread # %d of %d started.\n", my_arg->k, my_arg->p);
  while (my_arg->calculate)
    {
      int *I = my_arg->structure;
      double *A = my_arg->matrix;
      double *rhs = my_arg->rhs;

      double *x = my_arg->x;
      double *r = my_arg->r;
      double *u = my_arg->u;
      double *v = my_arg->v;

      double *max = &(my_arg->max);

      int n = my_arg->grid.n;
      int m = my_arg->grid.m;
      int matrix_size = my_arg->matr_size;
      func2d f = my_arg->f;
      f = Window::f_pool[Window::f_number];

      int thread_id = my_arg->k;
      int p = my_arg->p;
      int *error = &(my_arg->error);
      surface *buf_surface = my_arg->buf_surface;
      surface *buf_surface_resid = my_arg->buf_surface_resid;
      surface *buf_surface_orig = my_arg->buf_surface_orig;

      if (thread_id == MAIN_THREAD)
        {
          *error = create_matrix_structure (n, m, I);
        }
      if (thread_id == MAIN_THREAD)
        {
          printf ("create matrix struct\n");
          fflush (stdout);
        }

      reduce_sum (p, error, 1);
      if (*error >= 0)
        {
          *error = create_matrix_values (n, m, matrix_size, A, I, p, thread_id);
          reduce_sum (p, error, 1);
          if (thread_id == MAIN_THREAD)
            {
              printf ("create matrix vals\n");
              fflush (stdout);
            }
          if (*error >= 0)
            {
              create_rhs (n, m, matrix_size, f, rhs, my_arg->grid, p,
                          thread_id);
              if (thread_id == MAIN_THREAD)
                {
                  printf ("rhs\n");
                  fflush (stdout);
                }
              *error = MSR_solve (A, I, matrix_size, x, rhs, r, u, v, EPS_SOLVE,
                                  MAXIT, p, thread_id);
              if (thread_id == MAIN_THREAD)
                {
                  printf ("msr solve\n");
                  fflush (stdout);
                }
              if (*error >= 0)
                {
                  double max2 = 0;

                  fill_surface_with_vals (buf_surface_resid, (my_arg->grid), p,
                                          thread_id, x, max2, f);
                  fill_surface_with_vals (buf_surface_orig, (my_arg->grid), p,
                                          thread_id, x, max2, f, 1);
                  fill_surface_with_vals (buf_surface, (my_arg->grid), p,
                                          thread_id, x, max2);
                  if (thread_id == MAIN_THREAD)
                    {
                      printf ("fill with vals\n");
                      fflush (stdout);
                    }
                  double residual = MSR_residual (n, m, x, f, *max,
                                                  my_arg->grid, p, thread_id);
                  buf_surface_resid->set_max (residual);
                  buf_surface_resid->set_min (0.0);
                  //                  buf_surface->set_max (max2);
                  if (thread_id == MAIN_THREAD)
                    {
                      //                      printf ("[ Residual: %e ]\n\n",
                      //                      residual);
                      fflush (stdout);
                    }
                }
              else
                {
                  if (thread_id == MAIN_THREAD)
                    printf ("Cannot solve system!\n");
                }
            }
        }
      reduce_for_GUI (my_arg->p, my_arg->main_window, *(my_arg->c_out),
                      *(my_arg->p_out));
    }
  return 0;
}

void
reduce_sum (int p, int *a, int n)
{
  static pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
  static pthread_cond_t c_in = PTHREAD_COND_INITIALIZER;
  static pthread_cond_t c_out = PTHREAD_COND_INITIALIZER;

  static int t_in = 0;
  static int t_out = 0;
  static int *p_a;
  int i = 0;

  if (p <= 1)
    return;
  pthread_mutex_lock (&m);

  if (!p_a)
    p_a = a;
  else if (a)
    {
      for (i = 0; i < n; i++)
        p_a[i] += a[i];
    }
  t_in++;
  if (t_in >= p)
    {
      t_out = 0;
      pthread_cond_broadcast (&c_in);
    }
  else
    while (t_in < p)
      pthread_cond_wait (&c_in, &m);

  if (p_a && p_a != a)
    {
      for (int i = 0; i < n; i++)
        {
          a[i] = p_a[i];
        }
    }
  t_out++;
  if (t_out >= p)
    {
      p_a = 0;
      t_in = 0;
      pthread_cond_broadcast (&c_out);
    }
  else
    {
      while (t_out < p)
        pthread_cond_wait (&c_out, &m);
    }
  pthread_mutex_unlock (&m);
}

void
reduce_for_GUI (int p, Window *main_window, pthread_cond_t &c_out, int &t_out)
{
  static pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
  static pthread_cond_t c_in = PTHREAD_COND_INITIALIZER;
  static int t_in = 0;

  pthread_mutex_lock (&m);

  t_in++;
  if (t_in >= p)
    {
      t_out = 0;
      pthread_cond_broadcast (&c_in);
    }
  else
    while (t_in < p)
      pthread_cond_wait (&c_in, &m);
  t_out++;
  if (t_out >= p)
    {
      t_in = 0;
      main_window->calculation_completed_emit ();
    }

  while (t_out <= p)
    pthread_cond_wait (&c_out, &m);

  pthread_mutex_unlock (&m);
}

void
reduce_sum (int p, double *a, int n)
{
  static pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
  static pthread_cond_t c_in = PTHREAD_COND_INITIALIZER;
  static pthread_cond_t c_out = PTHREAD_COND_INITIALIZER;

  static int t_in = 0;
  static int t_out = 0;
  static double *p_a;
  int i = 0;

  if (p <= 1)
    return;
  pthread_mutex_lock (&m);

  if (!p_a)
    p_a = a;
  else if (a)
    {
      for (i = 0; i < n; i++)
        p_a[i] += a[i];
    }
  t_in++;
  if (t_in >= p)
    {
      t_out = 0;
      pthread_cond_broadcast (&c_in);
    }
  else
    while (t_in < p)
      pthread_cond_wait (&c_in, &m);

  if (p_a && p_a != a)
    {
      for (int i = 0; i < n; i++)
        {
          a[i] = p_a[i];
        }
    }
  t_out++;
  if (t_out >= p)
    {
      p_a = 0;
      t_in = 0;
      pthread_cond_broadcast (&c_out);
    }
  else
    {
      while (t_out < p)
        pthread_cond_wait (&c_out, &m);
    }
  pthread_mutex_unlock (&m);
}

void
reduce_max (int p, double *a, int n)
{
  static pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
  static pthread_cond_t c_in = PTHREAD_COND_INITIALIZER;
  static pthread_cond_t c_out = PTHREAD_COND_INITIALIZER;

  static int t_in = 0;
  static int t_out = 0;
  static double *p_a;
  int i = 0;

  if (p <= 1)
    return;
  pthread_mutex_lock (&m);

  if (!p_a)
    p_a = a;
  else if (a)
    {
      for (i = 0; i < n; i++)
        if (p_a[i] < a[i])
          p_a[i] = a[i];
    }
  t_in++;
  if (t_in >= p)
    {
      t_out = 0;
      pthread_cond_broadcast (&c_in);
    }
  else
    while (t_in < p)
      pthread_cond_wait (&c_in, &m);

  if (p_a && p_a != a)
    {
      for (int i = 0; i < n; i++)
        {
          a[i] = p_a[i];
        }
    }
  t_out++;
  if (t_out >= p)
    {
      p_a = 0;
      t_in = 0;
      pthread_cond_broadcast (&c_out);
    }
  else
    {
      while (t_out < p)
        pthread_cond_wait (&c_out, &m);
    }
  pthread_mutex_unlock (&m);
}
