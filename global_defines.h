#ifndef GLOBAL_DEFINES_H
#define GLOBAL_DEFINES_H

#include <functional>
#include <QPointF>
#include <cmath>
#include <utility>
#include "linear_map_2d.h"
#define MAIN_THREAD 0
#define HOLE_SIZE 5
#define I_MIN 1
inline int A = 2;
inline int B = 6;
inline double A_s = 0;
inline double A_real = 0.25;
inline double B_s = 0;
inline double B_real = 0.75;

typedef std::function<double (double, double)> func2d;

inline void
update_scretch (int n_surf)
{
  double dx = 1. / n_surf;
  int count = A_real / dx;
  A = count;
  A_s = A_real - count * dx;
  printf ("n = %d, A = %d, A_real = %lf, A_s = %lf, dx = %lf\n", n_surf, A, A_real,
          A_s, dx);
//  if (n_surf-A == A)
//    A--;
//  A--;


  B = n_surf - A;
  B_s = B_real - (B) * dx;
  B_s *=-1;
//    B_s = dx - B_s;
  printf ("n = %d, B = %d, B_real = %lf, B_s = %lf, dx = %lf\n", n_surf, B, B_real,
          B_s, dx);
  fflush (stdout);
}

/// function for approximation ///
static inline double f (double x, double y)
{
//  return 1;
//  return y;
//  return x*x + y*y;
  return exp((x - 0.5)*(x - 0.5) - (y - 0.5)*(y - 0.5));
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
  Linear_map_2d Jacobi; //tetragon --> square
  Linear_map_2d revJacobi; //square --> tetragon

  grid_data (int n_, int m_, float w, float h, std::pair<float, float> c_,
             Linear_map_2d jacobi, Linear_map_2d rev_jacobi)
    : u (w, 0), v (0., h), C (c_.first, c_.second), Jacobi(jacobi), revJacobi(rev_jacobi)
  {
    n = n_;
    m = m_;
    n_cut = n / 2;
    m_cut = m / 2;
  }
  grid_data () = default;
  grid_data (const grid_data &grid) = default;
  void increase ()
  {
    n *= 2;
    m *= 2;
    n_cut *= 2;
    m_cut *= 2;
  }
  void decrease ()
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
    B_s = B_real - (count) * dx;
//    B_s = dx - B_s;
    printf ("n = %d, B = %d, B_real = %lf, B_s = %lf, dx = %lf\n", n, B, B_real,
            B_s, dx);
    fflush (stdout);
  }
};

#endif // GLOBAL_DEFINES_H
