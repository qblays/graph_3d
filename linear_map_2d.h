#ifndef LINEAR_MAP_2D_H
#define LINEAR_MAP_2D_H
#include <array>
class Linear_map_2d
{
  std::array<double, 4> matrix;


public:
  Linear_map_2d (double[4]);
  Linear_map_2d () = default;
  static Linear_map_2d
  parallelogram_to_quad (double a, double b);
  static Linear_map_2d
  quad_to_parallelogram (double a, double b);
//  std::array<double, 2>
  std::array<double, 2>
  operator() (double x, double y)
  {
    double *m = this->matrix.data ();
    return {m[0] * x + m[1] * y, m[2] * x + m[3] * y};
  }

  void
  map (double a, double b, double &q, double &w);
};

#endif // LINEAR_MAP_2D_H
