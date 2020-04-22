#include <QtOpenGL>

#include "geometry_data.h"
#include "window.h"
#include "glwidget.h"
#include "surface.h"

#define SMALL_PAINT_AREA_SIZE 700
#define BIG_PAINT_AREA_SIZE 800
#define SCALE_FACTOR 1.1
#define DEPTH_EPS 1e-16

static inline float max_of_3 (float a, float b, float c)
{
  float max = (a < b) ? b : a;
  return ((max < c) ? c : max);
}

glwidget::glwidget (grid_data grid, QWidget *parent)
  : QGLWidget (QGLFormat (QGL::SampleBuffers), parent)
{
  m_surface.reset (new surface (grid, Window::func));
  m_buf_surface.reset (new surface (*m_surface));
  func2d null_func = [] (double, double) {return 0;};
  m_surface_resid.reset (new surface (grid, null_func));
  m_buf_surface_resid.reset (new surface (*m_surface_resid));
  m_curr_surface = m_surface.get ();

  m_xRot = 0; m_yRot = 0; m_zRot = 0;
  m_scaleCoef = 1.0f;
}

glwidget::~glwidget()
{
}

void glwidget::swap_surfaces ()
{
  m_surface.swap (m_buf_surface);
  m_surface_resid.swap (m_buf_surface_resid);
  if (m_state == FUNC)
    {
      m_curr_surface = m_surface.get ();
    }
  else
    {
      m_curr_surface = m_surface_resid.get ();
    }
}

QSize glwidget::minimumSizeHint() const
{
  return QSize (SMALL_PAINT_AREA_SIZE, SMALL_PAINT_AREA_SIZE);
}

QSize glwidget::sizeHint() const
{
  return QSize (BIG_PAINT_AREA_SIZE, BIG_PAINT_AREA_SIZE);
}

void glwidget::change_curr_to_func ()
{
  m_curr_surface = m_surface.get ();
  //set_gl_ortho ();
  m_state = FUNC;
  updateGL ();
}

void glwidget::change_curr_to_resid ()
{
  m_curr_surface = m_surface_resid.get ();
  //set_gl_ortho ();
  m_state = RESID;
  updateGL ();
}

static inline void qNormalizeAngle (int &angle)
{
  while (angle < 0)
    angle += 360 * 16;
  while (angle > 360)
    angle -= 360 * 16;
}

void glwidget::setXRotation (int angle)
{
  qNormalizeAngle (angle);
  if (angle != m_xRot)
    {
      m_xRot = angle;
      updateGL();
    }
}

void glwidget::setYRotation (int angle)
{
  qNormalizeAngle (angle);
  if (angle != m_yRot)
    {
      m_yRot = angle;
      updateGL();
    }
}

void glwidget::setZRotation (int angle)
{
  qNormalizeAngle (angle);
  if (angle != m_zRot)
    {
      m_zRot = angle;
      updateGL();
    }
}

void glwidget::initializeGL()
{
  qglClearColor(Qt::white);
  glEnable (GL_DEPTH_TEST);
  glEnable (GL_CULL_FACE);
  glShadeModel (GL_SMOOTH);
  glEnable (GL_LIGHTING);
  glEnable (GL_LIGHT0);
  glEnable (GL_MULTISAMPLE);
  GLfloat lightPos[4] = {0, 0, m_curr_surface->get_max () + 1, 1.0};
  glLightfv (GL_LIGHT0, GL_POSITION, lightPos);
  set_gl_ortho();
}

void glwidget::paintGL()
{
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity ();

  float max_z = fabsf (m_curr_surface -> get_max ());
  if (fabsf (m_curr_surface->get_min ()) > max_z)
    max_z = fabsf (m_curr_surface->get_min ());
  grid_data *grid = m_surface->get_grid ();
  float max_w = max_of_3 (fabs (grid->u.x()),
                          fabs (grid->C.x()),
                          fabs (grid->v.x ()));
  float max_h = max_of_3 (fabs (grid->u.y()),
                          fabs (grid->C.y()),
                          fabs (grid->v.y ()));
  glRotatef (m_xRot / 16.0, 1.0, 0.0, 0.0);
  glRotatef (m_yRot / 16.0, 0.0, 1.0, 0.0);
  glScalef (m_scaleCoef / max_w, m_scaleCoef / max_h, m_scaleCoef / max_z);
  glEnableClientState (GL_VERTEX_ARRAY);
  glEnableClientState (GL_NORMAL_ARRAY);

  glColorMaterial (GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
  glEnable (GL_COLOR_MATERIAL);

  glDisable (GL_CULL_FACE);
  draw_axis ();
  qglColor (Qt::blue);
  m_curr_surface->draw ();

  glDisableClientState (GL_VERTEX_ARRAY);
  glDisableClientState (GL_NORMAL_ARRAY);
}

void glwidget::resizeGL (int w, int h)
{
  int side = qMin (w, h);
  glViewport ((w - side) / 2, (h - side) / 2, side, side);
  //set_gl_ortho ();
}

void glwidget::mousePressEvent (QMouseEvent *event)
{
  if (event->buttons() == Qt::LeftButton)
    {
      m_mousePosition = event->pos();
    }
}

void glwidget::mouseMoveEvent (QMouseEvent *event)
{
  QPoint diff = event->pos() - m_mousePosition;
  if (event->buttons() == Qt::LeftButton)
    {
      setXRotation (m_xRot + 8 * diff.y());
      setYRotation (m_yRot + 8 * diff.x());
    }
  m_mousePosition = event->pos();
}

void glwidget::wheelEvent (QWheelEvent *pe)
{
  if (pe->delta() > 0)
    m_scaleCoef *= SCALE_FACTOR;
  else if (pe->delta() < 0)
    m_scaleCoef /= SCALE_FACTOR;
  updateGL();
}

void glwidget::set_gl_ortho ()
{
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity();
#ifdef QT_OPENGL_ES_1
  glOrtho(-2, 2, -1, 1, -1, 1);
#else
  glOrtho(-2, 2, -2, 2, -2, 2);
#endif
  glMatrixMode (GL_MODELVIEW);
}

void glwidget::draw_axis ()
{
  glLineWidth (2.0f);
  qglColor (Qt::black);
  //x
  glBegin (GL_LINES);
  glVertex3f ( 100.0f,  0.0f,  0.0f);
  glVertex3f (-100.0f,  0.0f,  0.0f);
  glEnd ();
  //y
  glBegin (GL_LINES);
  glVertex3f (0.0f,  100.0f,  0.0f);
  glVertex3f (0.0f, -100.0f,  0.0f);
  glEnd ();
  //z
  glBegin (GL_LINES);
  glVertex3f ( 0.0f,  0.0f,  100.0f);
  glVertex3f ( 0.0f,  0.0f, -100.0f);
  glEnd ();
}

