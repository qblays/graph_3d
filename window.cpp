#include <QHBoxLayout>
#include <QPainter>
#include <QRadioButton>
#include <QtOpenGL>
#include <algorithm>
#include <atomic>
#include <cstring>

#include "glwidget.h"
#include "matrix_func.h"
#include "sparse_func.h"
#include "threading.h"
#include "window.h"

const int Window::f_number_max;
func2d Window::f_pool[];
std::string Window::f_names[];
func2d Window::func = f0;
int Window::f_number = 0;
std::atomic_bool is_busy (true);

static inline double
max_of_3 (double a, double b, double c)
{
  double max = (a < b) ? b : a;
  return ((max < c) ? c : max);
}

void
polygon_widget::paintEvent (QPaintEvent * /*event*/)
{
  int center_x = width () / 2;
  int center_y = height () / 2;
  QPainter ptr (this);
  QPen pen;
  pen.setWidth (1);
  pen.setColor ("lightgrey");
  ptr.setPen (pen);
  ptr.drawRect (1, 2, width () - 3, height () - 4);
  pen.setColor ("grey");
  ptr.setPen (pen);
  ptr.drawLine (center_x, 4, center_x, height () - 4);
  ptr.drawLine (4, center_y, width () - 4, center_y);
  ptr.drawText (width () - 12, center_y - 4, "x");
  ptr.drawText (center_x - 10, 12, "y");

  QPointF p1 = grid.u;
  QPointF p2 = grid.v;
  QPointF pC = grid.C;
  QPointF pT1 = {0., 0.};
  QPointF pT2 = {0., 0.};
  QPointF pTC = {0., 0.};
  translate_tetragon (p1, pT1, grid.Jacobi);
  translate_tetragon (p2, pT2, grid.Jacobi);
  translate_tetragon (pC, pTC, grid.Jacobi);

  double range = max_of_3 (std::max (fabs (pC.x ()), fabs (pC.y ())),
                           fabs (p1.x ()), fabs (p2.y ()));
  range *= 2.;

  double related_p1_x = center_x * p1.x () / range;
  double related_p1_y = center_y * p1.y () / range;
  double related_p2_x = center_x * p2.x () / range;
  double related_p2_y = center_y * p2.y () / range;
  double related_pC_x = center_x * pC.x () / range;
  double related_pC_y = center_y * pC.y () / range;
  double related_pTC_x = center_x * pTC.x () / range;
  double related_pTC_y = center_y * pTC.y () / range;
  double related_pT1_x = center_x * pT1.x () / range;
  double related_pT1_y = center_y * pT1.y () / range;
  double related_pT2_x = center_x * pT2.x () / range;
  double related_pT2_y = center_y * pT2.y () / range;
  p1.setX (related_p1_x + center_x);
  p1.setY (-related_p1_y + center_y);
  p2.setX (related_p2_x + center_x);
  p2.setY (-related_p2_y + center_y);
  pC.setX (related_pC_x + center_x);
  pC.setY (-related_pC_y + center_y);
  pTC.setX (related_pTC_x + center_x);
  pTC.setY (-related_pTC_y + center_y);
  pT1.setX (related_pT1_x + center_x);
  pT1.setY (-related_pT1_y + center_y);
  pT2.setX (related_pT2_x + center_x);
  pT2.setY (-related_pT2_y + center_y);
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
  ptr.drawText (center.x () - 10, center.y () + 12, "0");
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

int
init_data_for_calc (grid_data &grid, thread_info *infos, surface *buf_surface,
                    surface *buf_surface_resid, surface *buf_surface_orig,
                    int p)
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

  if (!(matrix = new double[matr_len]) || !(structure = new int[matr_len]) ||
      !(x = new double[matr_size]) || !(r = new double[matr_size]) ||
      !(u = new double[matr_size]) || !(v = new double[matr_size]) ||
      !(rhs = new double[matr_size]))
    {
      return -1;
    }

  printf ("System size: %d\n", matr_size);
  std::fill_n (x, matr_size, 1);
  //  memset (x, 0, matr_size * sizeof (double));
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
      infos[i].buf_surface_orig = buf_surface_orig;
    }
  return 0;
}

void
del_data_after_calc (thread_info *infos)
{
  delete[] infos[0].x;
  delete[] infos[0].r;
  delete[] infos[0].u;
  delete[] infos[0].v;
  delete[] infos[0].matrix;
  delete[] infos[0].rhs;
  delete[] infos[0].structure;
}

Window::Window (grid_data grid, int threads_num, int f_num, QWidget *parent)
    : QWidget (parent), grid_for_calc (grid)
{
  widget = new glwidget (grid, this);
  widget->parent = this;
  f_number = f_num;

  QGroupBox *infobox = new QGroupBox (tr ("Information:"));
  QGridLayout *vbox = new QGridLayout;
  func_info = new QLabel (this);
  QLabel *gr_info = new QLabel (this);
  QLabel *max_info = new QLabel (this);
  QLabel *f_info = new QLabel (this);
  QLabel *th_info = new QLabel (this);
  QLabel *th_info_data = new QLabel (this);
  QLabel *A_info = new QLabel (this);
  //  QLabel *A_info_data = new QLabel (this);
  QLabel *extra_info = new QLabel (this);
  QLabel *extra_info_data = new QLabel (this);
  nm_info = new QLabel (this);
  p_info = new QLabel (this);
  max_info_data = new QLabel (this);
  func_info->setText (f_names[f_num].c_str ());
  gr_info->setText ("n, m:");
  f_info->setText ("function:");
  th_info->setText ("threads:");
  A_info->setText ("p: ");
  extra_info->setText ("Points on axis:");
  th_info_data->setText (std::to_string (threads_num).c_str ());

  char info_buf[1024];
  sprintf (info_buf, "%d", grid_for_calc.n);
  nm_info->setText (info_buf);

  sprintf (info_buf, "%d", p_int);
  p_info->setText (info_buf);
  sprintf (info_buf, "max val:");
  max_info->setText (info_buf);
  sprintf (info_buf, "(%.2f, 0.00), (0.00, %.2f)", grid_for_calc.u.x (),
           grid_for_calc.v.y ());
  extra_info_data->setText (info_buf);

  //  vbox->addWidget (th_info, 0, 0);
  //  vbox->addWidget (th_info_data, 0, 1);
  //  vbox->addWidget (f_info, 0, 2);
  //  vbox->addWidget (func_info, 0, 3);
  //  vbox->addWidget (gr_info, 2, 0);
  //  vbox->addWidget (nm_info, 2, 1);
  //  vbox->addWidget (max_info, 2, 2);
  //  vbox->addWidget (max_info_data, 2, 3);

  //  vbox->addWidget (A_info, 3, 0);
  //  vbox->addWidget (p_info, 3, 1);
  vbox->setColumnMinimumWidth (1, 160);
  vbox->addWidget (th_info, 0, 0);
  vbox->addWidget (th_info_data, 0, 1);
  vbox->addWidget (f_info, 1, 0);
  vbox->addWidget (func_info, 1, 1);
  vbox->addWidget (gr_info, 2, 0);
  vbox->addWidget (nm_info, 2, 1);
  vbox->addWidget (max_info, 3, 0);
  vbox->addWidget (max_info_data, 3, 1);

  vbox->addWidget (A_info, 4, 0);
  vbox->addWidget (p_info, 4, 1);
  //  vbox->addWidget (extra_info, 4, 0);
  //  vbox->addWidget (extra_info_data, 4, 1);
  infobox->setLayout (vbox);

  QGroupBox *settings = new QGroupBox (tr ("Settings:"));
  QGridLayout *grid_buttons = new QGridLayout;
  incr_button = new QPushButton ("Increase n");
  decr_button = new QPushButton ("Decrease n");
  change_mode_button = new QPushButton ("Change mode");
  incr_button_p = new QPushButton ("Increase p");
  decr_button_p = new QPushButton ("Decrease p");
  change_func_button = new QPushButton ("Change f");
  func_button = new QRadioButton ("Approximation");
  resid_button = new QRadioButton ("Residual");
  orig_button = new QRadioButton ("Original");
  //  grid_buttons->addWidget (incr_button, 0, 0);
  //  grid_buttons->addWidget (decr_button, 0, 1);
  //  grid_buttons->addWidget (change_func_button, 0, 2);
  //  grid_buttons->addWidget (func_button, 1, 0);
  //  grid_buttons->addWidget (resid_button, 1, 1);
  //  grid_buttons->addWidget (incr_button_p, 1, 2);
  //  grid_buttons->addWidget (decr_button_p, 1, 3);

  grid_buttons->addWidget (incr_button, 0, 0);
  grid_buttons->addWidget (decr_button, 1, 0);
  grid_buttons->addWidget (change_func_button, 2, 0);
  grid_buttons->addWidget (func_button, 3, 0);
  grid_buttons->addWidget (resid_button, 4, 0);
  grid_buttons->addWidget (orig_button, 5, 0);
  grid_buttons->addWidget (incr_button_p, 6, 0);
  grid_buttons->addWidget (decr_button_p, 7, 0);
  grid_buttons->addWidget (change_mode_button, 8, 0);
  settings->setLayout (grid_buttons);

  //  polygon_widget *grid_viz = new polygon_widget (grid);

  auto *v_layout = new QHBoxLayout (this);

  QVBoxLayout *h_layout = new QVBoxLayout (nullptr);
  settings->setMaximumWidth (230);
  infobox->setMaximumWidth (230);
  h_layout->addWidget (settings);
  h_layout->addWidget (infobox);
  //  h_layout->addWidget (grid_viz);
  v_layout->addLayout (h_layout);
  v_layout->addWidget (widget);
  //  this->setLayout (v_layout);

  connect (change_mode_button, SIGNAL (pressed ()), this,
           SLOT (change_mode ()));
  change_mode_button->setShortcut (QString ("1"));
  connect (incr_button, SIGNAL (pressed ()), this,
           SLOT (incr_button_handler ()));
  incr_button->setShortcut (QString ("4"));
  connect (decr_button, SIGNAL (pressed ()), this,
           SLOT (decr_button_handler ()));
  decr_button->setShortcut (QString ("5"));
  connect (incr_button_p, SIGNAL (pressed ()), this,
           SLOT (incr_button_handler_p ()));
  incr_button_p->setShortcut (QString ("6"));
  connect (decr_button_p, SIGNAL (pressed ()), this,
           SLOT (decr_button_handler_p ()));
  decr_button_p->setShortcut (QString ("7"));
  connect (change_func_button, SIGNAL (pressed ()), this,
           SLOT (change_func_button_handler ()));
  change_func_button->setShortcut (QString ("0"));
  connect (this, SIGNAL (calculation_completed ()), this, SLOT (rebase ()));

  connect (func_button, SIGNAL (clicked ()), this, SLOT (select_func ()));
  connect (resid_button, SIGNAL (clicked ()), this, SLOT (select_resid ()));
  connect (orig_button, SIGNAL (clicked ()), this, SLOT (select_orig ()));

  connect (this, SIGNAL (func_selected ()), widget,
           SLOT (change_curr_to_func ()));
  connect (this, SIGNAL (resid_selected ()), widget,
           SLOT (change_curr_to_resid ()));
  connect (this, SIGNAL (orig_selected ()), widget,
           SLOT (change_curr_to_orig ()));

  setWindowTitle ("Graph 2D");
  this->f_pool[0] = f0;
  this->f_pool[1] = f1;
  this->f_pool[2] = f2;
  this->f_pool[3] = f3;
  this->f_pool[4] = f4;
  this->f_pool[5] = f5;
  this->f_pool[6] = f6;
  this->f_pool[7] = f7;
  this->f_names[0] = "1";
  this->f_names[1] = "x";
  this->f_names[2] = "y";
  this->f_names[3] = "x+y";
  this->f_names[4] = "(x^2+y^2)^.5";
  this->f_names[5] = "x^2+y^2";
  this->f_names[6] = "exp (x^2-y^2)";
  this->f_names[7] = "1/(25(x^2+y^2)+1)";

  p = threads_num;
  infos = new thread_info[p];
  threads = 0;
  c_out_ptr = new pthread_cond_t[1];
  c_out_ptr[0] = PTHREAD_COND_INITIALIZER;
  p_out = new int[1];
  *p_out = 0;

  //  norm_func = new func2d ([grid] (double u, double v)
  //    {
  //      double r1 = std::max (fabs (grid.u.x()), fabs (grid.C.x()));
  //      double r2 = std::max (fabs (grid.v.y()), fabs (grid.C.y()));
  //      return func (u * r1, v * r2);
  //    });

  double r1_l = std::max (fabs (grid.u.x ()), fabs (grid.C.x ()));
  double r2_l = std::max (fabs (grid.v.y ()), fabs (grid.C.y ()));
  for (int i = 0; i < p; i++)
    {
      infos[i].k = i;
      infos[i].p = p;
      infos[i].main_window = this;
      infos[i].c_out = c_out_ptr;
      infos[i].calculate = true;
      infos[i].p_out = p_out;
      infos[i].f = func; // norm_func;
      infos[i].r1 = r1_l;
      infos[i].r2 = r2_l;
    }

  incr_button->setEnabled (false);
  decr_button->setEnabled (false);
  //  change_func_button->setEnabled(false);

  func_button->setChecked (true);

  if (init_data_for_calc (grid, infos, widget->get_buf_surface (),
                          widget->get_buf_surface_resid (),
                          widget->get_buf_surface_orig (), p) < 0)
    {
      printf ("Cannot build initial interpolation!\n");
      is_busy = false;
    }
  else
    {
      threads = new pthread_t[p];
      for (int i = 0; i < p; i++)
        {
          pthread_create (threads + i, 0, &thread_func, infos + i);
        }
    }
}

void
Window::incr_button_handler ()
{
  if (!is_busy)
    {
      incr_button->setDisabled (true);
      decr_button->setDisabled (true);
      incr_button_p->setDisabled (true);
      decr_button_p->setDisabled (true);
      change_func_button->setDisabled (true);
      grid_for_calc.increase ();
      init_data_for_calc (grid_for_calc, infos, widget->get_buf_surface (),
                          widget->get_buf_surface_resid (),
                          widget->get_buf_surface_orig (), p);
      is_busy = true;
      (*p_out)++;
      pthread_cond_broadcast (c_out_ptr);
    }
  char info_buf[1024];
  sprintf (info_buf, "%d", grid_for_calc.n);
  nm_info->setText (info_buf);
}

void
Window::decr_button_handler ()
{
  if (!is_busy)
    {
      if (grid_for_calc.m >= 4 && grid_for_calc.n >= 4)
        {
          incr_button->setDisabled (true);
          decr_button->setDisabled (true);
          incr_button_p->setDisabled (true);
          decr_button_p->setDisabled (true);
          change_func_button->setDisabled (true);
          grid_for_calc.decrease ();
          init_data_for_calc (grid_for_calc, infos, widget->get_buf_surface (),
                              widget->get_buf_surface_resid (),
                              widget->get_buf_surface_orig (), p);
          is_busy = true;
          (*p_out)++;
          pthread_cond_broadcast (c_out_ptr);
        }
    }
  char info_buf[1024];
  sprintf (info_buf, "%d", grid_for_calc.n);
  nm_info->setText (info_buf);
}
void
Window::incr_button_handler_p ()
{
  if (!is_busy)
    {
      incr_button->setDisabled (true);
      decr_button->setDisabled (true);
      incr_button_p->setDisabled (true);
      decr_button_p->setDisabled (true);
      change_func_button->setDisabled (true);
      change_p_int (1);
      init_data_for_calc (grid_for_calc, infos, widget->get_buf_surface (),
                          widget->get_buf_surface_resid (),
                          widget->get_buf_surface_orig (), p);
      is_busy = true;
      (*p_out)++;
      pthread_cond_broadcast (c_out_ptr);
    }
  char info_buf[1024];
  sprintf (info_buf, "%d", p_int);
  p_info->setText (info_buf);
}

void
Window::change_mode ()
{
  if (!is_busy)
    {

      int m = mode;

      mode = (STATE) ((m + 1) % 3);
      if (mode == FUNC)
        {
          func_selected ();
          func_button->setChecked(1);
        }
      else if (mode == RESID)
        {
          resid_selected ();
          resid_button->setChecked(1);
        }
      else if (mode == ORIG)
        {
          orig_selected ();
          orig_button->setChecked(1);
        }
    }
  char info_buf[1024];
  sprintf (info_buf, "%d", p_int);
  p_info->setText (info_buf);
}

void
Window::decr_button_handler_p ()
{
  if (!is_busy)
    {
      if (grid_for_calc.m >= 4 && grid_for_calc.n >= 4)
        {
          incr_button->setDisabled (true);
          decr_button->setDisabled (true);
          incr_button_p->setDisabled (true);
          decr_button_p->setDisabled (true);
          change_func_button->setDisabled (true);
          change_p_int (-1);
          init_data_for_calc (grid_for_calc, infos, widget->get_buf_surface (),
                              widget->get_buf_surface_resid (),
                              widget->get_buf_surface_orig (), p);
          is_busy = true;
          (*p_out)++;
          pthread_cond_broadcast (c_out_ptr);
        }
    }
  char info_buf[1024];
  sprintf (info_buf, "%d", p_int);
  p_info->setText (info_buf);
}

void
Window::change_func_button_handler ()
{
  if (!is_busy)
    {
      if (grid_for_calc.m >= 4 && grid_for_calc.n >= 4)
        {
          incr_button->setDisabled (true);
          decr_button->setDisabled (true);
          incr_button_p->setDisabled (true);
          decr_button_p->setDisabled (true);
          change_func_button->setDisabled (true);
          change_func ();
          init_data_for_calc (grid_for_calc, infos, widget->get_buf_surface (),
                              widget->get_buf_surface_resid (),
                              widget->get_buf_surface_orig (), p);
          is_busy = true;
          (*p_out)++;
          pthread_cond_broadcast (c_out_ptr);
        }
    }
  //  char info_buf[1024];
  //  sprintf (info_buf, "n = %d, m = %d", grid_for_calc.n, grid_for_calc.m);
  //  nm_info->setText (info_buf);
  func_info->setText (f_names[f_number].c_str ());
}

void
Window::set_func (int k)
{
  if (k < f_number_max)
    {
      f_number = k;
      func_info->setText (f_names[f_number].c_str ());
      func = f_pool[k];
    }
}

void
Window::select_func ()
{
  char info_buf[1024];
  sprintf (info_buf, "%e", MAX);
  max_info_data->setText (QString (info_buf));
  emit func_selected ();
}

void
Window::select_resid ()
{
  char info_buf[1024];
  sprintf (info_buf, "%e", MAX);
  max_info_data->setText (QString (info_buf));
  emit resid_selected ();
}

void
Window::select_orig ()
{
  char info_buf[1024];
  sprintf (info_buf, "%e", MAX);
  max_info_data->setText (QString (info_buf));
  emit orig_selected ();
}

void
Window::change_func ()
{
  f_number++;
  f_number %= f_number_max;
  Window::func = Window::f_pool[f_number];
}

void
Window::change_p_int (int dp)
{
  p_int += dp;
  P = p_int;
}

void
Window::rebase ()
{
  char info_buf[1024];
  sprintf (info_buf, "%e", MAX);
  max_info_data->setText (QString (info_buf));
  incr_button->setEnabled (true);
  decr_button->setEnabled (true);
  incr_button_p->setEnabled (true);
  decr_button_p->setEnabled (true);
  change_func_button->setEnabled (true);
  del_data_after_calc (infos);
  if (infos[0].error >= 0)
    {
      widget->swap_surfaces ();
    }
  widget->update ();
  is_busy = false;
}

void
Window::closeEvent (QCloseEvent *event)
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

Window::~Window ()
{
  if (threads)
    {
      for (int i = 0; i < p; i++)
        {
          pthread_join (threads[i], 0);
        }
      delete[] threads;
    }
  delete[] infos;
  delete[] c_out_ptr;
  delete[] p_out;
}

void
Window::calculation_completed_emit ()
{
  char info_buf[1024];
  sprintf (info_buf, "%e", MAX);
  max_info_data->setText (QString (info_buf));
  emit calculation_completed ();
}
