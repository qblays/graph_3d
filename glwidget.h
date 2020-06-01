#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QWidget>
//#include <QGLWidget>
#include <QMatrix4x4>
#include <QVector2D>
#include <QPointF>
#include <cstdlib>
#include <memory>
//#include <QOpenGLWindow>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include "window.h"

class grid_data;
class surface;

class glwidget: public QOpenGLWidget, protected QOpenGLFunctions
{
  Q_OBJECT



public:
  Window *parent;
  explicit glwidget (grid_data grid, QWidget *parent = 0);
  ~glwidget ();

  surface* get_buf_surface ()
  {
    return m_buf_surface.get ();
  }
  surface* get_buf_surface_resid ()
  {
    return m_buf_surface_resid.get ();
  }
  surface* get_buf_surface_orig ()
  {
    return m_buf_surface_orig.get ();
  }
  void swap_surfaces ();

protected:
  void initializeGL ();
  void resizeGL (int width, int height);
  void paintGL ();
  QSize minimumSizeHint () const;
  QSize sizeHint () const;
  void mousePressEvent (QMouseEvent *event);
  void mouseMoveEvent (QMouseEvent *event);
  void wheelEvent (QWheelEvent* pe);

public slots:
  void setXRotation (double angle);
  void setYRotation (double angle);
  void setZRotation (int angle);
  void change_curr_to_func ();
  void change_curr_to_resid ();

  void change_curr_to_orig();
private:
  void draw_axis (double c1, double c2, double c3);
  void set_gl_ortho ();
  void set_gl_ortho (double near, double far);

  double m_xRot;
  double m_yRot;
  int m_zRot;

  STATE m_state = FUNC;

  double m_scaleCoef;
  QPoint m_mousePosition;

  QVector<GLuint> m_faces;
  QVector<vector_3d> m_vertices;
  QVector<vector_3d> m_normals;

  surface *m_curr_surface;

  std::unique_ptr<surface> m_surface;
  std::unique_ptr<surface> m_buf_surface;


  std::unique_ptr<surface> m_surface_resid;
  std::unique_ptr<surface> m_buf_surface_resid;

  std::unique_ptr<surface> m_surface_orig;
  std::unique_ptr<surface> m_buf_surface_orig;
};

#endif // GLWIDGET_H
