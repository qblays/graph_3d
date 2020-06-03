#ifndef VECTOR3D_D_H
#define VECTOR3D_D_H
#include <initializer_list>

#include <math.h>
struct vector_3d
{
  double x;
  double y;
  double z;
  vector_3d (double _x, double _y, double _z) : x (_x), y (_y), z (_z) {}
  static vector_3d
  subtract (vector_3d a, vector_3d b)
  {
    return {a.x - b.x, a.y - b.y, a.z - b.z};
  }
  static vector_3d
  normal (vector_3d a, vector_3d b, vector_3d c)
  {
    auto x = subtract (b, a);
    auto y = subtract (c, a);
    vector_3d normal = {x.y * y.z - x.z * y.y, -x.x * y.z + x.z * y.x,
                        x.x * y.y - x.y * y.x};
    double len = normal.length ();
    return {normal.x / len, normal.y / len, normal.z / len};
  }
  double
  length ()
  {
    return sqrt (x * x + y * y + z * z);
  }
  void
  mult (vector_3d &a)
  {
    x *= a.x;
    y *= a.y;
    z *= a.z;
  }
  static vector_3d
  mult (vector_3d &a, vector_3d &b)
  {
    return {a.x * b.x, a.y * b.y, a.z * b.z};
  }
  void
  setZ (double z)
  {
    this->z = z;
  }
};

#endif // VECTOR3D_D_H
