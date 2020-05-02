#include <QtOpenGL>
#include <QHBoxLayout>
#include <QRadioButton>
#include <QPainter>
#include <cstring>
#include <atomic>
#include <algorithm>

#include "window.h"
#include "glwidget.h"
#include "thread_tools.h"
#include "sparse_tools.h"
#include "basic_matrix_tools.h"

std::atomic_bool is_busy (true);

static inline float max_of_3 (float a, float b, float c)
{
  float max = (a < b) ? b : a;
  return ((max < c) ? c : max);
}

void polygon_widget::paintEvent (QPaintEvent */*event*/)
{
  int center_x = width () / 2;
  int center_y = height () / 2;
  QPainter ptr (this);
  QPen pen;
  pen.setWidth (1);
  pen.setColor ("lightgrey");
  ptr.setPen (pen);
  ptr.drawRect (1, 2, width() - 3, height() - 4);
  pen.setColor ("grey");
  ptr.setPen (pen);
  ptr.drawLine (center_x, 4, center_x, height() - 4);
  ptr.drawLine (4, center_y, width() - 4, center_y);
  ptr.drawText (width () - 12, center_y - 4, "x");
  ptr.drawText (center_x - 10, 12, "y");

  QPointF p1 = grid.u;
  QPointF p2 = grid.v;
  QPointF pC = grid.C;
  QPointF pT1 = {0.,0.};
  QPointF pT2 = {0.,0.};
  QPointF pTC = {0.,0.};
  translate_tetragon (p1, pT1, grid.Jacobi);
  translate_tetragon (p2, pT2, grid.Jacobi);
  translate_tetragon (pC, pTC, grid.Jacobi);

  double range = max_of_3 (std::max (fabs (pC.x()), fabs (pC.y())),
                           fabs (p1.x()),
                           fabs (p2.y()));
  range *= 2.;

  double related_p1_x = center_x * p1.x() / range;
  double related_p1_y = center_y * p1.y() / range;
  double related_p2_x = center_x * p2.x() / range;
  double related_p2_y = center_y * p2.y() / range;
  double related_pC_x = center_x * pC.x() / range;
  double related_pC_y = center_y * pC.y() / range;
  double related_pTC_x = center_x * pTC.x() / range;
  double related_pTC_y = center_y * pTC.y() / range;
  double related_pT1_x = center_x * pT1.x() / range;
  double related_pT1_y = center_y * pT1.y() / range;
  double related_pT2_x = center_x * pT2.x() / range;
  double related_pT2_y = center_y * pT2.y() / range;
  p1.setX(related_p1_x + center_x);
  p1.setY(-related_p1_y + center_y);
  p2.setX(related_p2_x + center_x);
  p2.setY(-related_p2_y + center_y);
  pC.setX(related_pC_x + center_x);
  pC.setY(-related_pC_y + center_y);
  pTC.setX(related_pTC_x + center_x);
  pTC.setY(-related_pTC_y + center_y);
  pT1.setX(related_pT1_x + center_x);
  pT1.setY(-related_pT1_y + center_y);
  pT2.setX(related_pT2_x + center_x);
  pT2.setY(-related_pT2_y + center_y);
  QPointF center (center_x, center_y);

  //-----original polygon--------//
  pen.setColor ("red");
  pen.setWidth (2);
  ptr.setPen (pen);
  ptr.drawLine (center, p1);
  ptr.drawLine (center, p2);
  ptr.drawLine (p1, pC);
  ptr.drawLine (p2, pC);
  //-----------------------------//

  pen.setColor ("black");
  pen.setWidth (1);
  ptr.setPen (pen);
  ptr.drawText (pC.x () + 3, pC.y () - 3, "C");
  ptr.drawText (center.x () - 10, center.y() + 12, "0");
  ptr.drawText (5, 18, "Polygon:");
  pen.setWidth (3);
  ptr.setPen (pen);
  ptr.drawPoint (pC);
  ptr.drawPoint (p1);
  ptr.drawPoint (p2);
  ptr.drawPoint (center);

  //---translated polygon--------//
  pen.setColor ("green");
  pen.setWidth (1);
  ptr.setPen (pen);
  ptr.drawLine (center, pT1);
  ptr.drawLine (center, pT2);
  ptr.drawLine (pT1, pTC);
  ptr.drawLine (pT2, pTC);
  //-----------------------------//
}

int init_data_for_calc (grid_data &grid, thread_info *infos, surface *buf_surface,
                        surface *buf_surface_resid, int p)
{
  int n = grid.n;
  int m = grid.m;

  double *matrix;
  double *rhs;
  int *structure;

  double *x;
  double *r;
  double *u;
  double *v;

  int matr_size = get_matrix_size (n, m);
  int matr_len = get_matrix_len (n, m);

  printf ("Data initializing...\n");

  if (!(matrix = new double[matr_len])||
      !(structure = new int[matr_len])||
      !(x = new double[matr_size])||
      !(r = new double[matr_size])||
      !(u = new double[matr_size])||
      !(v = new double[matr_size])||
      !(rhs = new double[matr_size]))
    {
      return -1;
    }

  printf ("System size: %d\n", matr_size);
  memset (x, 0, matr_size * sizeof (double));
  memset (r, 0, matr_size * sizeof (double));
  memset (u, 0, matr_size * sizeof (double));
  memset (v, 0, matr_size * sizeof (double));

  for (int i = 0; i < p; i++)
    {
      infos[i].x = x;
      infos[i].r = r;
      infos[i].u = u;
      infos[i].v = v;

      infos[i].matrix = matrix;
      infos[i].rhs = rhs;
      infos[i].structure = structure;
      infos[i].grid = grid;

      infos[i].matr_len = matr_len;
      infos[i].matr_size = matr_size;
      infos[i].error = 0;
      infos[i].buf_surface = buf_surface;
      infos[i].buf_surface_resid = buf_surface_resid;
    }
  return 0;
}

void del_data_after_calc (thread_info *infos)
{
  delete [] infos[0].x;
  delete [] infos[0].r;
  delete [] infos[0].u;
  delete [] infos[0].v;
  delete [] infos[0].matrix;
  delete [] infos[0].rhs;
  delete [] infos[0].structure;
}

Window::Window (grid_data grid, int threads_num, QWidget *parent)
  : QWidget (parent), grid_for_calc (grid)
{
  widget = new glwidget (grid, this);

  QGroupBox *infobox = new QGroupBox (tr("Information:"));
  QGridLayout *vbox = new QGridLayout;
  QLabel *func_info = new QLabel (this);
  QLabel *gr_info = new QLabel (this);
  QLabel *f_info = new QLabel (this);
  QLabel *th_info = new QLabel (this);
  QLabel *th_info_data = new QLabel (this);
  QLabel *A_info = new QLabel (this);
  QLabel *A_info_data = new QLabel (this);
  QLabel *extra_info = new QLabel (this);
  QLabel *extra_info_data = new QLabel (this);
  nm_info = new QLabel (this);
  func_info->setText (FUNC_TO_STRING_FOR_GUI);
  gr_info->setText ("Grid Size:");
  f_info->setText ("Function:");
  th_info->setText ("Threads:");
  A_info->setText ("Point C:");
  extra_info->setText ("Points on axis:");
  th_info_data->setText (std::to_string(threads_num).c_str());

  char info_buf[1024];
  sprintf (info_buf, "n = %d, m = %d", grid_for_calc.n, grid_for_calc.m);
  nm_info->setText (info_buf);

  sprintf (info_buf, "(%.2f, %.2f)", grid_for_calc.C.x(), grid_for_calc.C.y());
  A_info_data->setText (info_buf);

  sprintf (info_buf, "(%.2f, 0.00), (0.00, %.2f)", grid_for_calc.u.x(), grid_for_calc.v.y());
  extra_info_data->setText (info_buf);

  vbox->addWidget (th_info, 0, 0);
  vbox->addWidget (th_info_data, 0, 1);
  vbox->addWidget (f_info, 1, 0);
  vbox->addWidget (func_info, 1, 1);
  vbox->addWidget (gr_info, 2, 0);
  vbox->addWidget (nm_info, 2, 1);

  vbox->addWidget (A_info, 3, 0);
  vbox->addWidget (A_info_data, 3, 1);
  vbox->addWidget (extra_info, 4, 0);
  vbox->addWidget (extra_info_data, 4, 1);
  infobox->setLayout (vbox);

  QGroupBox *settings = new QGroupBox (tr("Settings:"));
  QGridLayout *grid_buttons = new QGridLayout;
  incr_button = new QPushButton ("Increase [n]");
  decr_button = new QPushButton ("Decrease [n]");
  func_button = new QRadioButton ("Function");
  resid_button = new QRadioButton ("Residual");
  grid_buttons->addWidget (incr_button, 0, 0);
  grid_buttons->addWidget (decr_button, 0, 1);
  grid_buttons->addWidget (func_button, 1, 0);
  grid_buttons->addWidget (resid_button, 1, 1);
  settings->setLayout (grid_buttons);

  polygon_widget *grid_viz = new polygon_widget (grid);

  QVBoxLayout *v_layout = new QVBoxLayout (this);
  QHBoxLayout *h_layout = new QHBoxLayout (this);
  h_layout->addWidget (settings);
  h_layout->addWidget (infobox);
  h_layout->addWidget (grid_viz);
  v_layout->addLayout (h_layout);
  v_layout->addWidget (widget);
  this->setLayout (v_layout);

  connect (incr_button, SIGNAL (pressed ()), this, SLOT (incr_button_handler ()));
  connect (decr_button, SIGNAL (pressed ()), this, SLOT (decr_button_handler ()));
  connect (this, SIGNAL (calculation_completed ()), this, SLOT (rebase ()));

  connect (func_button, SIGNAL (clicked ()), this, SLOT (select_func ()));
  connect (resid_button, SIGNAL (clicked ()), this, SLOT (select_resid ()));

  connect (this, SIGNAL (func_selected ()), widget, SLOT (change_curr_to_func ()));
  connect (this, SIGNAL (resid_selected ()), widget, SLOT (change_curr_to_resid ()));

  setWindowTitle ("Graph 2D");

  p = threads_num;
  infos = new thread_info [p];
  threads = 0;
  c_out_ptr = new pthread_cond_t [1];
  c_out_ptr[0] = PTHREAD_COND_INITIALIZER;
  p_out = new int [1];
  *p_out = 0;

  norm_func = new func2d ([grid] (double u, double v)
    {
      double r1 = std::max (fabs (grid.u.x()), fabs (grid.C.x()));
      double r2 = std::max (fabs (grid.v.y()), fabs (grid.C.y()));
      return func (u * r1, v * r2);
    });

  for (int i = 0; i < p; i++)
    {
      infos[i].k = i;
      infos[i].p = p;
      infos[i].main_window = this;
      infos[i].c_out = c_out_ptr;
      infos[i].calculate = true;
      infos[i].p_out = p_out;
      infos[i].f = norm_func;
    }

  incr_button->setEnabled (false);
  decr_button->setEnabled (false);
  func_button->setChecked (true);

  if (init_data_for_calc (grid, infos, widget->get_buf_surface (),
                          widget->get_buf_surface_resid(), p) < 0)
    {
      printf ("Cannot build initial interpolation!\n");
      is_busy = false;
    }
  else
    {
      threads = new pthread_t [p];
      for (int i = 0; i < p; i++)
        {
          pthread_create (threads + i, 0, &thread_func, infos + i);
        }
    }
}

void Window::incr_button_handler ()
{
  if (!is_busy)
    {
      incr_button->setDisabled (true);
      decr_button->setDisabled (true);
      grid_for_calc.increase();
      init_data_for_calc (grid_for_calc, infos, widget->get_buf_surface(),
                          widget->get_buf_surface_resid (), p);
      is_busy = true;
      (*p_out)++;
      pthread_cond_broadcast (c_out_ptr);
    }
  char info_buf[1024];
  sprintf (info_buf, "n = %d, m = %d", grid_for_calc.n, grid_for_calc.m);
  nm_info->setText (info_buf);
}

void Window::decr_button_handler ()
{
  if (!is_busy)
    {
      if (grid_for_calc.m >= 4 && grid_for_calc.n >= 4)
        {
          incr_button->setDisabled (true);
          decr_button->setDisabled (true);
          grid_for_calc.decrease();
          init_data_for_calc (grid_for_calc, infos, widget->get_buf_surface(),
                              widget->get_buf_surface_resid (), p);
          is_busy = true;
          (*p_out)++;
          pthread_cond_broadcast (c_out_ptr);
        }
    }
  char info_buf[1024];
  sprintf (info_buf, "n = %d, m = %d", grid_for_calc.n, grid_for_calc.m);
  nm_info->setText (info_buf);
}

void Window::select_func ()
{
  emit func_selected ();
}

void Window::select_resid ()
{
  emit resid_selected ();
}

void Window::rebase ()
{
  incr_button->setEnabled (true);
  decr_button->setEnabled (true);
  del_data_after_calc (infos);
  if (infos[0].error >= 0)
    {
      widget->swap_surfaces ();
    }
  widget->update ();
  is_busy = false;
}

void Window::closeEvent (QCloseEvent *event)
{
  if (is_busy)
      event->ignore ();
  else
    {
      for (int i = 0; i < p; i++)
        {
          infos[i].calculate = false;
        }
      (*p_out)++;
      pthread_cond_broadcast (c_out_ptr);
      event->accept ();
    }
}

Window::~Window()
{
  if (threads)
    {
     for (int i = 0; i < p; i++)
       {
          pthread_join (threads[i], 0);
       }
     delete [] threads;
    }
  delete [] infos;
  delete [] c_out_ptr;
  delete [] p_out;
}

void Window::calculation_completed_emit ()
{
  emit calculation_completed ();
}
