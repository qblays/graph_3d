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

class polygon_widget: public QWidget
{
  Q_OBJECT
public:
  grid_data grid;
  polygon_widget (grid_data grid_) : QWidget ()
  {
    grid = grid_;
  }
protected:
  void paintEvent (QPaintEvent */*event*/);
};

class Window: public QWidget
{
  Q_OBJECT

public:
  static func2d func;

public:
  Window (grid_data grid, int threads_num, QWidget *parent = 0);
  ~Window();
  void closeEvent (QCloseEvent *event) override;
  void calculation_completed_emit ();

private:
  //interface

  glwidget *widget;
  QPushButton *incr_button;
  QPushButton *decr_button;
  QRadioButton *func_button;
  QRadioButton *resid_button;
  QLabel *nm_info;

  pthread_t *threads;
  thread_info *infos;
  func2d *norm_func;

  //treads
  int p;
  pthread_cond_t *c_out_ptr;
  int *p_out;

  //calculations
  grid_data grid_for_calc;

  //function
public slots:
  void incr_button_handler ();
  void decr_button_handler ();
  void rebase ();
  void select_func ();
  void select_resid ();

signals:
  void calculation_completed ();
  void func_selected ();
  void resid_selected ();
};

#endif // WINDOW_H
