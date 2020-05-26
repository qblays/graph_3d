#include <QtOpenGL>

#include "geometry_data.h"
#include "glwidget.h"
#include "surface.h"
#include "window.h"

#define SMALL_PAINT_AREA_SIZE 800
#define BIG_PAINT_AREA_SIZE 800
#define SCALE_FACTOR 1.1
#define DEPTH_EPS 1e-16

static inline float
max_of_3 (float a, float b, float c)
{
  float max = (a < b) ? b : a;
  return ((max < c) ? c : max);
}

glwidget::glwidget (grid_data grid, QWidget *parent) : QOpenGLWidget (parent)
{
  m_surface.reset (new surface (grid, Window::func));
  m_buf_surface.reset (new surface (*m_surface));
  func2d null_func = [] (double, double) { return 0; };
  m_surface_resid.reset (new surface (grid, null_func));
  m_buf_surface_resid.reset (new surface (*m_surface_resid));
  m_curr_surface = m_surface.get ();

  m_xRot = 0;
  m_yRot = 0;
  m_zRot = 0;
  m_scaleCoef = 1.0f;
}

glwidget::~glwidget () {}

void
glwidget::swap_surfaces ()
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

QSize
glwidget::minimumSizeHint () const
{
  return QSize (SMALL_PAINT_AREA_SIZE, SMALL_PAINT_AREA_SIZE);
}

QSize
glwidget::sizeHint () const
{
  return QSize (BIG_PAINT_AREA_SIZE, BIG_PAINT_AREA_SIZE);
}

void
glwidget::change_curr_to_func ()
{
  m_curr_surface = m_surface.get ();
  // set_gl_ortho ();
  m_state = FUNC;
  update ();
}

void
glwidget::change_curr_to_resid ()
{
  m_curr_surface = m_surface_resid.get ();
  // set_gl_ortho ();
  m_state = RESID;
  update ();
}

static inline void
qNormalizeAngle (int &angle)
{
  //  while (angle < 0)
  //    angle += 360 * 16;
  //  while (angle > 360)
  //    angle -= 360 * 16;
  //  angle %= 360*16;
}

void
glwidget::setXRotation (double angle)
{
  //  qNormalizeAngle (angle);
  if (std::abs (angle - m_xRot) > 1.e-2)
    {
      m_xRot = angle;
      update ();
    }
}

void
glwidget::setYRotation (double angle)
{
  //  qNormalizeAngle (angle);
  if (std::abs (angle - m_yRot) > 1.e-2)
    {
      m_yRot = angle;
      update ();
    }
}

void
glwidget::setZRotation (int angle)
{
  qNormalizeAngle (angle);
  if (angle != m_zRot)
    {
      m_zRot = angle;
      update ();
    }
}

void
glwidget::initializeGL ()
{
  initializeOpenGLFunctions ();
  glClearColor (0, 0, 0, 1);
  glEnable (GL_DEPTH_TEST);
  //  glEnable (GL_CULL_FACE);
  glShadeModel (GL_SMOOTH);
  //  glEnable (GL_LIGHTING);
  //  glEnable (GL_LIGHT0);
  glEnable (GL_MULTISAMPLE);
  //  GLfloat lightPos[4] = {0, 0, m_curr_surface->get_max () + 1, 1.0};
  //  glLightfv (GL_LIGHT0, GL_POSITION, lightPos);
  set_gl_ortho ();
}

void
glwidget::paintGL ()
{
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  int side = qMax (width (), height ());
  int wside = side;

  printf ("w = %d, h = %d, wside = %d, side = %d\n", width (), height (), wside,
          side);
  fflush (stdout);
  glViewport ((width () - wside) / 2, (height () - side) / 2, wside, side);
  glMatrixMode (GL_MODELVIEW_MATRIX);
  glLoadIdentity ();

  float max_z = std::max (fabsf (m_curr_surface->get_max ()),
                          fabsf (m_curr_surface->get_min ()));
  printf ("max_z = %e\n", max_z);
  fflush (stdout);

  grid_data *grid = m_surface->get_grid ();
  float max_w =
      max_of_3 (fabs (grid->u.x ()), fabs (grid->C.x ()), fabs (grid->v.x ()));
  float max_h =
      max_of_3 (fabs (grid->u.y ()), fabs (grid->C.y ()), fabs (grid->v.y ()));
  if (m_xRot > 90.)
    {
      m_xRot = 90.;
    }
  if (m_xRot < -90.)
    {
      m_xRot = -90.;
    }
  glRotatef (m_xRot, 1.0, 0.0, 0.0);
  glRotatef (m_yRot, 0.0, 1.0, 0.0);

  double scale_coef = m_scaleCoef * double (SMALL_PAINT_AREA_SIZE) / wside;
  draw_axis ();
  glScaled (scale_coef / max_w, scale_coef / max_h, scale_coef / max_z);

  glEnableClientState (GL_VERTEX_ARRAY);
  glEnableClientState (GL_NORMAL_ARRAY);

  //  glColorMaterial (GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
  //  glEnable (GL_COLOR_MATERIAL);

  //  glDisable (GL_CULL_FACE);

  glColor4ub (145, 17, 20, 1);

  m_curr_surface->draw (m_state == RESID);
  if (m_state == RESID)
    {
      printf ("resid = %e\n", max_z);
      fflush (stdout);
    }

  glDisableClientState (GL_VERTEX_ARRAY);
  glDisableClientState (GL_NORMAL_ARRAY);
}

void
glwidget::resizeGL (int w, int h)
{
  int side = qMin (w, h);
  //  glViewport ((w - side) / 2, (h - side) / 2, side, side);
  //  set_gl_ortho ();
}

void
glwidget::mousePressEvent (QMouseEvent *event)
{
  if (event->buttons () == Qt::LeftButton)
    {
      m_mousePosition = event->pos ();
    }
}

void
glwidget::mouseMoveEvent (QMouseEvent *event)
{
  QPoint diff = event->pos () - m_mousePosition;
  if (event->buttons () == Qt::LeftButton)
    {
      setXRotation (m_xRot + diff.y () / 2.);
      setYRotation (m_yRot + diff.x () / 2.);
    }
  m_mousePosition = event->pos ();
}

void
glwidget::wheelEvent (QWheelEvent *pe)
{
  if (pe->delta () > 0)
    m_scaleCoef *= SCALE_FACTOR;
  else if (pe->delta () < 0)
    m_scaleCoef /= SCALE_FACTOR;
  update ();
}

void
glwidget::set_gl_ortho ()
{
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  glOrtho (-1, 1, -1, 1, -10, 10);
  //  glFrustum(-2, 2, -2, 2, -2000, 2000);
  glMatrixMode (GL_MODELVIEW);
}

void
glwidget::set_gl_ortho (double near, double far)
{
  //  glMatrixMode (GL_PROJECTION);
  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity ();
  glOrtho (-2, 2, -2, 2, near, far);
  //  glMatrixMode (GL_MODELVIEW);
}

void
glwidget::draw_axis ()
{
  //  glLineWidth (2.0f);
  //  qglColor (Qt::white);
  // x
  glColor4ub (0, 50, 50, 1);
  //  glColor4
  glBegin (GL_LINES);
  //  glVertex3i (0, 0, 100);
  //  glVertex3i (0, 0, -100);
  //  glEnd ();
  //  glBegin (GL_LINES);
  glVertex3i (10000, 0, 0);
  glVertex3i (-10000, 0, 0);
  //  glEnd ();
  // y
  //  glBegin (GL_LINES);
  glVertex3i (0, 10000, 0);
  glVertex3i (0, -10000, 0);

  //  if (m_state == RESID)
  //    {
  //      glVertex3d (0, 0, 1.e-2);
  //      glVertex3d (0, 0, -1.e-2);
  //    }
  //  else
  //    {
  glVertex3i (0, 0, 10000);
  glVertex3i (0, 0, -10000);
  //    }

  glEnd ();
  // z
}
