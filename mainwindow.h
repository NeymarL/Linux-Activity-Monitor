#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QTime>
#include <fstream>
#include <string>
#include <QPushButton>
#include <dirent.h>
#include "tools/tools.h"
#include <QStandardItemModel>
#include <string.h>
#include <map>
#include <sys/types.h>
#include <signal.h>
#include <QMessageBox>
#include <vector>
#include <QSortFilterProxyModel>
#include <QDialogButtonBox>
#include "info_dialog.h"

#define CPU 0
#define MEM 1
#define CLK_TCK 100

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    void get_cpu_stat();

    void get_process_info(int);

    void init_table();

    void refocus();

    void get_mem_stat();

private slots:
    void update_data();

    void on_cpu_button_clicked();

    void on_mem_button_clicked();

    void on_tableView_clicked(const QModelIndex &index);

    void on_pushButton_3_clicked();

    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;
    QTimer* timer;
    int total_cpu_time;
    int total_user_time;
    int total_sys_time;
    int total_idle_time;
    int state;
    QStandardItemModel* model;
    QStandardItemModel* cpu_model;
    QStandardItemModel* mem_model;
    std::map<int, int>* process_map;
    std::map<int, std::string>* username_dict;
    int num_processes;
    int last_pid;
    int mem_size;
};

#endif // MAINWINDOW_H
