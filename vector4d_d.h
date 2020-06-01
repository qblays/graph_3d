#ifndef VECTOR4D_D_H
#define VECTOR4D_D_H
#include <math.h>
struct vector_4d
{
  double x;
  double y;
  double z;
  double w;
  vector_4d (double _x = 0, double _y = 0, double _z = 0, double _w = 0)
      : x (_x), y (_y), z (_z), w (_w)
  {
  }
  static vector_4d
  subtract (vector_4d a, vector_4d b)
  {
    return {a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w};
  }
  //  static vector_4d
  //  normal (vector_4d a, vector_4d b, vector_4d c)
  //  {
  //    auto x = subtract (b, a);
  //    auto y = subtract (c, a);
  //    vector_4d normal = {x.y * y.z - x.z * y.y, -x.x * y.z + x.z * y.x,
  //                        x.x * y.y - x.y * y.x};
  //    double len = normal.length ();
  //    return {normal.x / len, normal.y / len, normal.z / len};
  //  }
  double
  length ()
  {
    return sqrt (x * x + y * y + z * z + w * w);
  }
  void
  setZ (double z)
  {
    this->z = z;
  }
  void
  setX (double x)
  {
    this->x = x;
  }
  void
  setY (double y)
  {
    this->y = y;
  }
  void
  setW (double w)
  {
    this->w = w;
  }
};
#endif // VECTOR4D_D_H
