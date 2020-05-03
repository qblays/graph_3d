#include "basic_matrix_tools.h"
#include "linear_map_2d.h"
#include "window.h"
#include <QApplication>

func2d Window::func = f;

int
main (int argc, char *argv[])
{
  int n = 8, m = 8;
  std::pair<float, float> C = {1., 1.};
  float w = 0.7, h = 0.3;
  int thread_num = 1;
  char usage_hint[1024];
  sprintf (usage_hint, "Usage: %s <n> <m> <w> <h> <C.x> <C.y> <p>\n", argv[0]);

  ///---------- parse command line ----------///
  if (argc != 8)
    {
      printf ("%s", usage_hint);
    }
  else
    {
      if ((n = atoi (argv[1])) <= 0 || (m = atoi (argv[2])) <= 0 ||
          (thread_num = atoi (argv[7])) <= 0)
        {
          printf ("%s [n > 0, m > 0]\n", usage_hint);
          return -1;
        }
      w = atof (argv[3]);
      h = atof (argv[4]);
      C.first = atof (argv[5]);
      C.second = atof (argv[6]);
      if (w <= 0 || h <= 0)
        {
          printf ("Invalid points!\n");
          return -1;
        }
    }
  ///----------------------------------------///

  double x1 = 0, y1 = 0;
  double x2 = w, y2 = 0.3;
  //  double x3 = C.first, y3 = C.second;
  double x3 = 1, y3 = 1;
  double x4 = 0.3, y4 = w;
  double mat[4];
  double jacobi[4];
  double a = 0.7;
  double b = 0.4;

  auto p_to_q = Linear_map_2d::parallelogram_to_quad (a, b);
  auto q_to_p = Linear_map_2d::quad_to_parallelogram (a, b);

  double r1, r2;
  r1 = std::max (fabs (x2), fabs (x3));
  r2 = std::max (fabs (y3), fabs (y4));
//  build_homography_to_rect (x1, y1, x2, y2, x3, y3, x4, y4, Jacobi, r1, r2);
  //  build_homography_from_quad (x1, y1, x2, y2, x3, y3, x4, y4, Mat);

  // print_mtx_3x3 (Mat);
//  mtx_reverse (Jacobi, Mat, 3);
//  build_homography_to_rect (x1, y1, x2, y2, x3, y3, x4, y4, Jacobi, r1, r2);
  //  build_homography_from_quad (x1, y1, x2, y2, x3, y3, x4, y4, Mat);

//  print_mtx_3x3 (Jacobi);

  double new_x, new_y;
  p_to_q.map(a, b, new_x, new_y);
//  translate_tetragon (x1, y1, new_x, new_y, Jacobi);
  printf ("new A: (%.3f, %.3f)\n", new_x, new_y);
//  translate_tetragon (x2, y2, new_x, new_y, Jacobi);
//  printf ("new B: (%.3f, %.3f)\n", new_x, new_y);
//  translate_tetragon (x3, y3, new_x, new_y, Jacobi);
//  printf ("new C: (%.3f, %.3f)\n", new_x, new_y);
//  translate_tetragon (x4, y4, new_x, new_y, Jacobi);
//  printf ("new D: (%.3f, %.3f)\n", new_x, new_y);

  QApplication app (argc, argv);
  setlocale (LC_ALL, "C");
  Window window ({n, m, w, h, C, p_to_q, q_to_p}, thread_num);
  setlocale (LC_ALL, "C");
  window.show ();
  int ret = app.exec ();
  return ret;
}
