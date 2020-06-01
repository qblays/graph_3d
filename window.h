#ifndef WINDOW_H
#define WINDOW_H

#include <QMainWindow>
#include <QOpenGLWindow>

#include "global_defines.h"

class glwidget;
class QPushButton;
class QRadioButton;
class QLabel;
struct thread_info;

class polygon_widget : public QWidget
{
  Q_OBJECT
public:
  grid_data grid;
  polygon_widget (grid_data grid_) : QWidget () { grid = grid_; }

protected:
  void
  paintEvent (QPaintEvent * /*event*/);
};

class Window : public QWidget
{
  Q_OBJECT

public:
  static func2d func;
  static const int f_number_max = 8;
  static func2d f_pool[f_number_max];
  static std::string f_names[f_number_max];
  static int f_number;
  static double
  zero_func (double, double)
  {
    return 0;
  }

public:
  Window (grid_data grid, int threads_num, int func_n, QWidget *parent = 0);
  ~Window ();
  void
  closeEvent (QCloseEvent *event) override;
  void
  calculation_completed_emit ();

  void
  set_func (int k);
  void
  change_p_int (int dp);

public:
  // interface

  glwidget *widget;
  QPushButton *incr_button;
  QPushButton *decr_button;
  QPushButton *change_mode_button;
  QPushButton *incr_button_p;
  QPushButton *decr_button_p;
  QPushButton *change_func_button;
  QRadioButton *func_button;
  QRadioButton *resid_button;
  QRadioButton *orig_button;
  QLabel *nm_info;
  QLabel *max_info_data;
  QLabel *p_info;
  QLabel *func_info;
  STATE mode = FUNC;

  pthread_t *threads;
  thread_info *infos;
  func2d norm_func;

  // treads
  int p;
  int p_int = 0;
  pthread_cond_t *c_out_ptr;
  int *p_out;
  double max = 0;

  // calculations
  grid_data grid_for_calc;

  // function
public slots:
  void
  incr_button_handler ();
  void
  decr_button_handler ();
  void
  rebase ();
  void
  select_func ();
  void
  select_resid ();
  void
  change_func ();

  void
  change_func_button_handler ();
  void
  incr_button_handler_p ();
  void
  decr_button_handler_p ();
  void select_orig();
  void change_mode();
signals:
  void
  calculation_completed ();
  void
  func_selected ();
  void
  resid_selected ();
  void
  orig_selected ();
};

#endif // WINDOW_H
