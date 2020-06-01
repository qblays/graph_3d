#ifndef threading
#define threading

#include "global_defines.h"


class surface;
class Window;

struct thread_info
{
  int p;
  int k;
  int error;
  Window *main_window;
  pthread_cond_t *c_out;
  bool calculate;
  int *p_out;

  grid_data grid;

  int *structure;
  double *matrix;
  double *rhs;

  double *x;
  double *r;
  double *u;
  double *v;
  double max;

  double r1, r2;

  int matr_size, matr_len;

  func2d f;
  func2d_n f_n;
  surface *buf_surface;
  surface *buf_surface_resid;
  surface *buf_surface_orig;
};

void
fill_surface_with_vals (surface *buf_surface, grid_data &grid_calc, int p,
                        int k, double *vals, double &max_argfunc2d,
                        func2d func = 0, bool show_orig = 0);

void *
thread_func (void *arg);

void
reduce_sum (int p, int *a = 0, int n = 0);

void
reduce_sum (int p, double *a, int n);

void
reduce_max (int p, double *a, int n);

void
reduce_for_GUI (int p, Window *main_window, pthread_cond_t &c_out, int &t_out);

#endif // threading
