#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QWidget>
#include <QGLWidget>
#include <QMatrix4x4>
#include <QVector2D>
#include <QPointF>
#include <cstdlib>
#include <memory>

class grid_data;
class surface;

class glwidget: public QGLWidget
{
  Q_OBJECT

  enum STATE { FUNC, RESID };

public:
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
  void setXRotation (int angle);
  void setYRotation (int angle);
  void setZRotation (int angle);
  void change_curr_to_func ();
  void change_curr_to_resid ();

private:
  void draw_axis ();
  void set_gl_ortho ();

  int m_xRot;
  int m_yRot;
  int m_zRot;

  STATE m_state = FUNC;

  float m_scaleCoef;
  QPoint m_mousePosition;

  QVector<GLuint> m_faces;
  QVector<QVector3D> m_vertices;
  QVector<QVector3D> m_normals;

  surface *m_curr_surface;

  std::unique_ptr<surface> m_surface;
  std::unique_ptr<surface> m_buf_surface;

  std::unique_ptr<surface> m_surface_resid;
  std::unique_ptr<surface> m_buf_surface_resid;
};

#endif // GLWIDGET_H
