#ifndef matrix_func_H
#define matrix_func_H
#include "linear_map_2d.h"
#include <QPointF>

void
build_homography_from_quad (double x1, double y1, double x2, double y2,
                            double x3, double y3, double x4, double y4,
                            double *Mat);

void
build_homography_to_rect (double x1, double y1, double x2, double y2, double x3,
                          double y3, double x4, double y4, double *Mat,
                          double r1, double r2);

int
mtx_reverse (double *mtx, double *buf, int n, double eps = 1e-12);

double
det_3x3 (double *Mat);

void
translate_tetragon (double x, double y, double &new_x, double &new_y,
                    double *Mat);

void
translate_tetragon (QPointF &p, QPointF &new_p, double *Mat);

inline void
translate_tetragon (double x, double y, double &new_x, double &new_y,
                    Linear_map_2d &map)
{
  new_x = map (x, y)[0];
  new_y = map (x, y)[1];
}

void
translate_tetragon (QPointF &p, QPointF &new_p, Linear_map_2d &map);

void
print_mtx_3x3 (double *Mat);

#endif // matrix_func_H
