#ifndef GLOBAL_DEFINES_H
#define GLOBAL_DEFINES_H

#include <functional>
#include <QPointF>
#include <cmath>
#include <utility>

#define MAIN_THREAD 0
#define HOLE_SIZE 4

typedef std::function<double (double, double)> func2d;

/// function for approximation ///
static inline double f (double x, double y)
{
  return y;
  return x*x + y*y;
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
  double *Jacobi; //tetragon --> square
  double *revJacobi; //square --> tetragon

  grid_data (int n_, int m_, float w = 1., float h = 1., std::pair<float, float> c_ = {2., 2.},
             double *jacobi = 0, double *rev_jacobi = 0)
    : u (w, 0), v (0., h), C (c_.first, c_.second)
  {
    Jacobi = jacobi;
    revJacobi = rev_jacobi;
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