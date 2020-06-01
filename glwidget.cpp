#include "glwidget.h"
#include "geometry_data.h"
#include "surface.h"
#include "vector3d_d.h"
#include "window.h"
#include <QtOpenGL>

#define SMALL_PAINT_AREA_SIZE 800
#define BIG_PAINT_AREA_SIZE 800
#define SCALE_FACTOR 1.1
#define DEPTH_EPS 1e-14

static inline double
max_of_3 (double a, double b, double c)
{
  double max = (a < b) ? b : a;
  return ((max < c) ? c : max);
}

glwidget::glwidget (grid_data grid, QWidget *parent) : QOpenGLWidget (parent)
{
  m_surface.reset (new surface (grid, Window::func));
  m_buf_surface.reset (new surface (*m_surface));
  func2d null_func = Window::zero_func;
  m_surface_resid.reset (new surface (grid, null_func));
  m_buf_surface_resid.reset (new surface (*m_surface_resid));
  m_surface_orig.reset (new surface (grid, Window::func));
  m_buf_surface_orig.reset (new surface (*m_surface_orig));

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
  m_surface_orig.swap (m_buf_surface_orig);
  if (m_state == FUNC)
    {
      m_curr_surface = m_surface.get ();
    }
  else if (m_state == RESID)
    {
      m_curr_surface = m_surface_resid.get ();
    }
  else
    {
      m_curr_surface = m_surface_orig.get ();
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
  glEnable (GL_LIGHTING);
  update ();
}

void
glwidget::change_curr_to_resid ()
{
  char info_buf[1024];
  sprintf (info_buf, "%e", MAX);
  parent->max_info_data->setText (QString (info_buf));
  m_curr_surface = m_surface_resid.get ();
  // set_gl_ortho ();
  m_state = RESID;
  glDisable (GL_LIGHTING);

  update ();
}

void
glwidget::change_curr_to_orig ()
{
  m_curr_surface = m_surface_orig.get ();
  // set_gl_ortho ();
  m_state = ORIG;
  glEnable (GL_LIGHTING);
  update ();
}

static inline void
qNormalizeAngle (int & /*angle*/)
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
  //  glEnable (GL_DOUBL)
  //  glEnable (GL_CULL_FACE);
  //  glShadeModel (GL_SMOOTH);
  glEnable (GL_LIGHTING);
  glEnable (GL_LIGHT2);
  //  glEnable (GL_MULTISAMPLE);
  //  GLfloat lightPos[4] = {0, 0, m_curr_surface->get_max () + 1, 1.0};
  GLfloat lightParams[4] = {1., 1., 1., 1};
  glLightfv (GL_LIGHT2, GL_DIFFUSE, lightParams);
  set_gl_ortho ();
}

void
glwidget::paintGL ()
{
  if (m_state == RESID)
    {
      glDisable (GL_LIGHTING);
    }
  else
    {
      glEnable (GL_LIGHTING);
    }
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  int side = qMax (width (), height ());
  int wside = side;

  //  printf ("w = %d, h = %d, wside = %d, side = %d\n", width (), height (),
  //  wside,
  //          side);
  //  fflush (stdout);

  glViewport ((width () - wside) / 2, (height () - side) / 2, wside, side);
  glMatrixMode (GL_MODELVIEW_MATRIX);
  GLdouble init_mat[16] = {1., 0, 0,  0,
                           0, 1.0, 0, 0,
                           0,  0, 1., 0,
                           0, 0,   0, 1.0};
  glLoadMatrixd (init_mat);
//  glLoadIdentity ();

  float max_z = std::max (fabs (m_curr_surface->get_max ()),
                           fabs (m_curr_surface->get_min ()));
  MAX = max_z;
  char info_buf[1024];
  sprintf (info_buf, "%.2e", MAX);
  parent->max_info_data->setText (QString (info_buf));
  parent->update ();
  if (max_z < 1e-10)
    {
      max_z = 1e-45;
    }
  //  printf ("max_z = %e\n", max_z);
  //  fflush (stdout);

  grid_data *grid = m_surface->get_grid ();
  double max_w =
      max_of_3 (fabs (grid->u.x ()), fabs (grid->C.x ()), fabs (grid->v.x ()));
  double max_h =
      max_of_3 (fabs (grid->u.y ()), fabs (grid->C.y ()), fabs (grid->v.y ()));
  if (m_xRot > 90.)
    {
      m_xRot = 90.;
    }
  if (m_xRot < -90.)
    {
      m_xRot = -90.;
    }
  double scale_coef = m_scaleCoef * double (SMALL_PAINT_AREA_SIZE) / wside;
  glRotated (m_xRot, 1.0, 0.0, 0.0);
  glRotated (m_yRot, 0.0, 1.0, 0.0);
  glTranslated (-0.5 * scale_coef / max_w, -1 * max_z * scale_coef,
                0.5 * scale_coef / max_h);
  glRotatef (-90, 1, 0, 0);
  //  glTranslated (0, 0, -max_z / 20);
//  printf ("%e %e\n", scale_coef / max_z, max_z * scale_coef);
  fflush (stdout);
  draw_axis (scale_coef / max_w, scale_coef / max_h, scale_coef / max_z);
  glScaled (scale_coef / max_w, scale_coef / max_h, scale_coef / max_z);

  glEnableClientState (GL_VERTEX_ARRAY);
  glEnableClientState (GL_NORMAL_ARRAY);

  glColorMaterial (GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
  glEnable (GL_COLOR_MATERIAL);

  //  glDisable (GL_CULL_FACE);

  glColor4ub (145, 17, 20, 1);

  m_curr_surface->draw (m_state == RESID);

  glDisableClientState (GL_VERTEX_ARRAY);
  glDisableClientState (GL_NORMAL_ARRAY);
}

void
glwidget::resizeGL (int /*w*/, int /*h*/)
{
  //  int side = qMin (w, h);
  //  glViewport ((w - side) / 2, (h - side) / 2, side, side);
  //  set_gl_ortho ();
}

void
glwidget::mousePressEvent (QMouseEvent *event)
{
  if (event->buttons () == Qt::RightButton)
    {
      m_mousePosition = event->pos ();
    }
}

void
glwidget::mouseMoveEvent (QMouseEvent *event)
{
  QPoint diff = event->pos () - m_mousePosition;
  if (event->buttons () == Qt::RightButton)
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
  GLdouble init_mat[16] = {1., 0, 0,  0,
                           0, 1.0, 0, 0,
                           0,  0, 1., 0,
                           0, 0,   0, 1.0};
  glLoadMatrixd (init_mat);
  glOrtho (-1, 1, -1, 1, -10, 10);
  //  glFrustum(-2, 2, -2, 2, -2000, 2000);
  glMatrixMode (GL_MODELVIEW);
}

void
glwidget::set_gl_ortho (double near, double far)
{
  //  glMatrixMode (GL_PROJECTION);
  glMatrixMode (GL_MODELVIEW);
  GLdouble init_mat[16] = {1., 0, 0,  0,
                           0, 1.0, 0, 0,
                           0,  0, 1., 0,
                           0, 0,   0, 1.0};
  glLoadMatrixd (init_mat);
  glOrtho (-2, 2, -2, 2, near, far);
  //  glMatrixMode (GL_MODELVIEW);
}

void
glwidget::draw_axis (double c1, double c2, double /*c3*/)
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
  glVertex3d (6, 0, 0);
  glVertex3d (-6, 0, 0);
  //  glEnd ();
  // y
  //  glBegin (GL_LINES);
  glVertex3d (0, 6, 0);
  glVertex3d (0, -6, 0);

  //  if (m_state == RESID)
  //    {
  //      glVertex3d (0, 0, 1.e-2);
  //      glVertex3d (0, 0, -1.e-2);
  //    }
  //  else
  //    {
  glVertex3d (0, 0, 6);
  glVertex3d (0, 0, -6);
  //    }

  glVertex3d (0.5 * c1, 0.5 * c2, 6);
  glVertex3d (0.5 * c1, 0.5 * c2, -6);
  glEnd ();
  // z
}
