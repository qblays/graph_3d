#ifndef SURFACE_H
#define SURFACE_H

#include "global_defines.h"

class QVector4D;
class geometry_data;

class surface
{
public:
  surface (const grid_data &grid, func2d &f);
  surface (const surface& obj);
  ~surface();

  int get_point_numb ()
  {
    return point_numb;
  }
  grid_data *get_grid ()
  {
    return &m_grid;
  }
  geometry_data *get_geom ()
  {
    return m_geom_ptr;
  }
  double get_max ()
  {
    return max_val;
  }
  double get_min ()
  {
    return min_val;
  }

  void set_max (double max)
  {
    max_val = max;
  }
  void set_min (double min)
  {
    min_val = min;
  }

  void update_ij (int i, int j, vector_4d &vals);

  void draw (bool is_resid, vector_3d scaling);

private:
  geometry_data *m_geom_ptr;
  grid_data m_grid;

  double max_val;
  double min_val;
  int point_numb;
};

#endif // SURFACE_H
