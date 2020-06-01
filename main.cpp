#include "linear_map_2d.h"
#include "matrix_func.h"
#include "threading.h"
#include "window.h"
#include <QApplication>
#include <cstdlib>
#include <fenv.h>
#include <iostream>
#include <optional>
//#include <ieeefp.h>
#include <signal.h>
#include <thread>

// func2d Window::func = f;

int
read_file_info (std::optional<std::string> &fname, double &x, double &y,
                double &inner_coef)
{
  if (!fname)
    {
      return -1;
    }
  char *line = NULL;
  FILE *fp;
  size_t *len = 0;
  fp = fopen (fname->c_str (), "r");
  if (!fp)
    return -1;
  int success = 0;
  while (getline (&line, len, fp) != -1)
    {
      if (line[0] == '#')
        {
          continue;
        }
      if (!success)
        {
          int res = sscanf (line, "%lf %lf", &x, &y);
          if (res == 2)
            {
              success = 1;
              /*break*/;
            }
        }
      else
        {
          int res = sscanf (line, "%lf", &inner_coef);
          if (res == 1)
            {
              success = 2;
              break;
            }
        }
    }
  free (line);
  fclose (fp);
  if (success != 2)
    return -1;
  return 0;
}

void
show_fe_exceptions (void)
{
  printf ("current exceptions raised: ");
  if (fetestexcept (FE_DIVBYZERO))
    printf (" FE_DIVBYZERO");
  if (fetestexcept (FE_INEXACT))
    printf (" FE_INEXACT");
  if (fetestexcept (FE_INVALID))
    printf (" FE_INVALID");
  if (fetestexcept (FE_OVERFLOW))
    printf (" FE_OVERFLOW");
  if (fetestexcept (FE_UNDERFLOW))
    printf (" FE_UNDERFLOW");
  if (fetestexcept (FE_ALL_EXCEPT) == 0)
    printf (" none");
  printf ("\n");
}

int
main (int argc, char *argv[])
{
//  std::shared_ptr<void (int)> handler (
//      signal (SIGFPE, [] (int /*signum*/) { throw std::logic_error ("FPE"); }),
//      [] (__sighandler_t f) { signal (SIGFPE, f); });
//  show_fe_exceptions ();
//  fexcept_t excepts;

//  /* Setup a "current" set of exception flags. */
//  feraiseexcept (FE_INVALID);
//  show_fe_exceptions ();

//  /* Save current exception flags. */
//  fegetexceptflag (&excepts, FE_ALL_EXCEPT);

//  /* Temporarily raise two other exceptions. */
//  feclearexcept (FE_ALL_EXCEPT);
//  feraiseexcept (FE_OVERFLOW | FE_INEXACT | FE_DIVBYZERO | FE_UNDERFLOW |
//                 FE_INVALID);
//  show_fe_exceptions ();

  //  show_fe_exceptions ();
  //  fpsetmask(FP_X_INV|FP_X_OFL|FP_X_UFL|FP_X_DZ|FP_X_IMP);
  //  feenableexcept (FE_INVALID | FE_DIVBYZERO | FE_OVERFLOW | FE_UNDERFLOW);
//  volatile double aa = 1., ba = 0.;
//  auto ca = aa / ba;
//    double t = 1e-23;
//  printf ("%.30e\n", ca);
//  fflush (stdout);
  int n = 8, m = 8;
  std::optional<std::string> filename;
  std::pair<float, float> C = {1., 1.};
  float w = 0.7, h = 0.3;
  double eps = 1e-14;
  int thread_num = std::thread::hardware_concurrency ();
  int f_num = 5;
  char usage_hint[1024];
  //  sprintf (usage_hint, "Usage: %s <n> <m> <w> <h> <C.x> <C.y> <p>\n",
  //  argv[0]);
  sprintf (usage_hint, "Usage: %s <file> <nx> <ny> <k> <eps> <p>\n", argv[0]);

  ///---------- parse command line ----------///
  if (argc != 7)
    {
      printf ("%s", usage_hint);
    }
  else
    {
      if ((n = atoi (argv[2])) <= 0 || (m = atoi (argv[3])) <= 0 ||
          (thread_num = atoi (argv[6])) <= 0)
        {
          printf ("%s [n > 0, m > 0]\n", usage_hint);
          return -1;
        }
      filename = argv[1];
      f_num = atoi (argv[4]);
      eps = atof (argv[5]); /*
       w = atof (argv[3]);
       h = atof (argv[4]);
       C.first = atof (argv[5]);
       C.second = atof (argv[6]);
       if (w <= 0 || h <= 0)
         {
           printf ("Invalid points!\n");
           return -1;
         }*/
    }
  EPS_SOLVE = eps * eps;
  m = n;
  ///----------------------------------------///

  //  double x1 = 0, y1 = 0;
  //  double x2 = w, y2 = 0.3;
  //  //  double x3 = C.first, y3 = C.second;
  //  double x3 = 1, y3 = 1;
  //  double x4 = 0.3, y4 = w;
  //  double mat[4];
  //  double jacobi[4];

  double a = 1;
  double b = 0;
  double inner_coef = 0.5;
  if (read_file_info (filename, a, b, inner_coef) == -1)
    {
      printf ("cant read file\n");
    }
  else
    {
      printf ("(a, b, inner coef) = (%lf, %lf, %lf) from file\n", a, b,
              inner_coef);
      fflush (stdout);
    }

  auto p_to_q = Linear_map_2d::parallelogram_to_quad (a, b);
  auto q_to_p = Linear_map_2d::quad_to_parallelogram (a, b);
  A_real = 0.5 * (1 - inner_coef);
  B_real = 1 - A_real;
  //  double r1, r2;
  //  r1 = std::max (fabs (x2), fabs (x3));
  //  r2 = std::max (fabs (y3), fabs (y4));
  double new_x, new_y;
  p_to_q.map (a, b, new_x, new_y);
  QApplication app (argc, argv);
  QFont font = qApp->font ();
  font.setPointSize (10);
  qApp->setFont (font);
  setlocale (LC_ALL, "C");
  Window window ({n, m, w, h, C, p_to_q, q_to_p}, thread_num, f_num);
  window.set_func (f_num);
  setlocale (LC_ALL, "C");
  window.show ();
  int ret = app.exec ();
  return ret;
}
