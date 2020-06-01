#ifndef GLOBAL_DEFINES_H
#define GLOBAL_DEFINES_H

#include "linear_map_2d.h"
#include <QPointF>
#include <cmath>
#include <functional>
#include <utility>
#include "vector4d_d.h"
#include "vector3d_d.h"
#define MAIN_THREAD 0
#define HOLE_SIZE 5
#define I_MIN 1
enum STATE { FUNC, RESID, ORIG };

inline double EPS_SOLVE = 1e-12;
inline bool SHOW_ORIG = 1;
inline int A = 2;
inline int B = 6;
inline double A_s = 0;
inline double A_real = 0.25;
inline double B_s = 0;
inline double B_real = 0.75;
inline int P = 0;
inline double MAX = 0;
typedef double (*func2d) (double, double);
typedef double (*func2d_n) (double, double, double, double);

inline void
update_stretch (int n_surf)
{
  double dx = 1. / n_surf;
  int count = A_real / dx;
  A = count;
  A_s = A_real - count * dx;
  //  printf ("n = %d, A = %d, A_real = %lf, A_s = %lf, dx = %lf\n", n_surf, A,
  //          A_real, A_s, dx);
  //  if (n_surf-A == A)
  //    A--;
  //  A--;

  B = n_surf - A;
  B_s = B_real - (B)*dx;
  B_s *= -1;
  //    B_s = dx - B_s;
  //  printf ("n = %d, B = %d, B_real = %lf, B_s = %lf, dx = %lf\n", n_surf, B,
  //          B_real, B_s, dx);
  //  fflush (stdout);
}

/// function for approximation ///
static inline double
f0 (double /*x*/, double /*y*/)
{
  return 1;
}
static inline double
f1 (double x, double /*y*/)
{
  return x;
}
static inline double
f2 (double /*x*/, double y)
{
  return y;
}

static inline double
f3 (double x, double y)
{
  return x + y;
}

static inline double
f4 (double x, double y)
{
  return sqrt (x * x + y * y);
}

static inline double
f5 (double x, double y)
{
  return (x * x + y * y);
}
static inline double
f6 (double x, double y)
{
  return exp (x * x - y * y);
}

static inline double
f7 (double x, double y)
{
  return 1 / (25 * (x * x + y * y) + 1);
}

inline double
double_eq (double a, double b)
{
  return fabs (a - b) < 1e-14;
}

#define FUNC_TO_STRING_FOR_GUI "z = x*x + y*y"

class grid_data
{
public:
  int n;
  int m;
  int n_cut;
  int m_cut;
  QPointF u, v, C;
  Linear_map_2d Jacobi;    // tetragon --> square
  Linear_map_2d revJacobi; // square --> tetragon

  grid_data (int n_, int m_, double w, double h, std::pair<double, double> c_,
             Linear_map_2d jacobi, Linear_map_2d rev_jacobi)
      : u (w, 0), v (0., h), C (c_.first, c_.second), Jacobi (jacobi),
        revJacobi (rev_jacobi)
  {
    n = n_;
    m = m_;
    n_cut = n / 2;
    m_cut = m / 2;
  }
  grid_data () = default;
  grid_data (const grid_data &grid) = default;
  void
  increase ()
  {
    n *= 2;
    m *= 2;
    n_cut *= 2;
    m_cut *= 2;
  }
  void
  decrease ()
  {
    n /= 2;
    m /= 2;
    n_cut /= 2;
    m_cut /= 2;
  }
};

#endif // GLOBAL_DEFINES_H

#ifndef GLOBAL_DEFINES_H
#define GLOBAL_DEFINES_H

#include "linear_map_2d.h"
#include <QPointF>
#include <cmath>
#include <functional>
#include <utility>
#define MAIN_THREAD 0
//#define HOLE_SIZE 1
//#define HOLE_SIZE1 1
inline int A = 2;
inline int B = 6;
inline double A_s = 0;
inline double A_real = 0.3;
inline double B_s = 0;
inline double B_real = 0.7;

typedef std::function<double (double, double)> func2d;

/// function for approximation ///
static inline double
f (double x, double y)
{
  //    return 1;
  //  return x;
  //  return x*x + y*y;
  return exp ((x - 0.5) * (x - 0.5) - (y - 0.5) * (y - 0.5));
}
#define FUNC_TO_STRING_FOR_GUI "z = x*x + y*y"

class grid_data
{
public:
  int n;
  int m;
  int n_cut;
  int m_cut;
  QPointF u, v, C;
  Linear_map_2d Jacobi;    // tetragon --> square
  Linear_map_2d revJacobi; // square --> tetragon

  grid_data (int n_, int m_, float w, float h, std::pair<float, float> c_,
             Linear_map_2d jacobi, Linear_map_2d rev_jacobi)
      : u (w, 0), v (0., h), C (c_.first, c_.second), Jacobi (jacobi),
        revJacobi (rev_jacobi)
  {
    n = n_;
    m = m_;
    n_cut = n / 2;
    m_cut = m / 2;
    update_scretch ();
  }
  grid_data () = default;
  grid_data (const grid_data &grid) = default;
  void
  increase ()
  {
    n *= 2;
    m *= 2;
    n_cut *= 2;
    m_cut *= 2;
    update_scretch ();
  }
  void
  decrease ()
  {
    n /= 2;
    m /= 2;
    n_cut /= 2;
    m_cut /= 2;
    update_scretch ();
  }
  void
  update_scretch ()
  {
    double dx = 1. / n;
    int count = A_real / dx;
    A = count + 1;
    A_s = A_real - count * dx;
    printf ("n = %d, A = %d, A_real = %lf, A_s = %lf, dx = %lf\n", n, A, A_real,
            A_s, dx);
    count = B_real / dx;
    B = count;
    B_s = B_real - (count)*dx;
    //    B_s = dx - B_s;
    printf ("n = %d, B = %d, B_real = %lf, B_s = %lf, dx = %lf\n", n, B, B_real,
            B_s, dx);
    fflush (stdout);
  }
};

#endif // GLOBAL_DEFINES_H
