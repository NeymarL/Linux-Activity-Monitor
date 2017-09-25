#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->timer = new QTimer(this);
    this->cpu_model = new QStandardItemModel();
    this->mem_model = new QStandardItemModel();
    this->state = CPU;
    this->model = this->cpu_model;
    this->init_table();
    ui->frame_center->add_data(0.1);
    process_map = new std::map<int, int>();
    username_dict = get_username_dict();
    timer->start(2000);
    this->total_cpu_time = 0;
    this->total_idle_time = 0;
    this->last_pid = -1;
    this->num_processes = 0;
    get_cpu_stat();
    get_process_info(0);
    connect(timer, SIGNAL(timeout()), this, SLOT(update_data()));
}

MainWindow::~MainWindow()
{
    if (timer->isActive()) {
        timer->stop();
    }
    delete timer;
    delete process_map;
    delete username_dict;
    delete cpu_model;
    delete mem_model;
    delete ui;
}

void MainWindow::update_data()
{
    switch (state) {
    case CPU:
        get_cpu_stat();
        break;

    case MEM:
        get_mem_stat();
        break;

    default:
        break;
    }
    refocus();
}

void MainWindow::get_cpu_stat()
{
    std::ifstream input;
    input.open("/proc/stat", std::ios_base::in);
    int total_diff = 0;
    if (input.is_open()) {
        std::string cpu;
        int user, nice, system, idle, iowait, irq, softirq, stealstolen, guest;
        input >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> stealstolen >> guest;
        input.close();
        int total_cpu_time = user + nice + system + idle + irq + softirq + guest + iowait + stealstolen;
        if (this->total_cpu_time != 0) {
            total_diff = total_cpu_time - this->total_cpu_time;
            int idle_diff = idle - this->total_idle_time;
            int user_diff = user - this->total_user_time;
            int sys_diff = system - this->total_sys_time;
            float total_rate = (total_diff - idle_diff) * 1.0 / total_diff;
            float idle_rate = idle_diff * 1.0 / total_diff;
            float user_rate = user_diff * 1.0 / total_diff;
            float sys_rate = sys_diff * 1.0 / total_diff;
            ui->left_1->setText(tr("系统:\t          %1%").arg(QString::number(sys_rate * 100, 'f',2 )));
            ui->left_2->setText(tr("用户:\t          %1%").arg(QString::number(user_rate * 100, 'f', 2)));
            ui->left_3->setText(tr("空闲:\t          %1%").arg(QString::number(idle_rate * 100, 'f', 2)));
            ui->right_1->setText(tr("总利用率:           %1%").arg(QString::number(total_rate * 100, 'f', 1)));
            ui->right_1->setStyleSheet("QLabel {\n	color:rgb(242, 21, 21)\n}");
            ui->frame_center->add_data(total_rate);
        }
        this->total_cpu_time = total_cpu_time;
        this->total_idle_time = idle;
        this->total_sys_time = system;
        this->total_user_time = user;
    }
    else {
        qDebug() << "Cannot open file" << endl;
    }
    get_process_info(total_diff);
}

void MainWindow::get_mem_stat()
{
    std::ifstream input;
    input.open("/proc/meminfo", std::ios_base::in);
    if (input.is_open()) {
        int mem_total, mem_free, mem_used, mem_cached;
        int swap_total, swap_free, swap_used;
        std::string tmp;
        char buffer[100];
        int values[45];
        int i = 0;
        while (i < 45) {
            input >> tmp >> values[i];
            input.getline(buffer, 100);
            i++;
        }
        mem_total = values[0];
        mem_free = values[1];
        mem_used = mem_total - mem_free;
        mem_cached = values[4];
        swap_total = values[14];
        swap_free = values[15];
        swap_used = swap_total - swap_free;
        float rate = mem_used * 100.0 / mem_total;
        this->mem_size = mem_total;
        ui->left_1->setText(tr("物理内存:      %1").arg(get_proper_unit(mem_total).c_str()));
        ui->left_2->setText(tr("已使用内存:  %1").arg(get_proper_unit(mem_used).c_str()));
        ui->left_3->setText(tr("可用内存:    %1").arg(get_proper_unit(mem_free).c_str()));
        ui->right_1->setText(tr("总使用率:          %1%").arg(QString::number(rate, 'f', 2)));
        ui->right_2->setText(tr("已缓存文件: %1").arg(get_proper_unit(mem_cached).c_str()));
        ui->right_3->setText(tr("已使用的交换: %1 KB").arg(swap_used));
        ui->frame_center->add_data(rate / 100);
        input.close();
    }
    get_process_info(0);
}

void MainWindow::get_process_info(int total_diff)
{
    struct dirent *ptr;
    DIR *dir;
    std::ifstream input;
    dir = opendir("/proc");
    int i = 0;
    int total_threads = 0;
    int cnt = 0;
    while((ptr = readdir(dir)) != NULL) {
        if (is_digit(ptr->d_name)) {
            // qDebug() << ptr->d_name << endl;
            model->setItem(i, 5, new QStandardItem(ptr->d_name));
            model->item(i, 5)->setTextAlignment(Qt::AlignCenter);
            char filename[20] = "/proc/";
            strcat(filename, ptr->d_name);
            input.open(strcat(filename, "/stat"));
            if (input.is_open()) {
                cnt++;
                std::string name;   // process name
                int pid;            // process id
                char state;         // process state : "RSDZTW"
                int ppid, pgid, sid, tty_nr, tty_pgrp, flags, min_flt;
                int cmin_flt, maj_flt, cmaj_flt, priority, nice;
                long utime, stime, cutime, cstime;
                int num_threads, zero;
                unsigned long long start_time;
                input >> pid >> name >> state >> ppid >> pgid >> sid >> tty_nr \
                        >> tty_pgrp >> flags >> min_flt >> cmin_flt >> maj_flt \
                        >> cmaj_flt >> utime >> stime >> cutime >> cstime >> priority \
                        >> nice >> num_threads >> zero >> start_time;
                // discard the parentheses
                char* tmp = (char*)name.c_str();
                int len = strlen(tmp);
                tmp[len - 1] = '\0';
                model->setItem(i, 0, new QStandardItem(tmp+1));
                // num threads
                if (this->state == CPU) {
                    model->setItem(i, 3, new QStandardItem(QString::number(num_threads)));
                    model->item(i, 3)->setTextAlignment(Qt::AlignCenter);
                } else {
                    model->setItem(i, 4, new QStandardItem(QString::number(num_threads)));
                    model->item(i, 4)->setTextAlignment(Qt::AlignCenter);
                }
                total_threads += num_threads;
                if (this->state == CPU) {
                    // display process state
                    switch (state) {
                    case 'R':
                        model->setItem(i, 4, new QStandardItem("运行"));
                        break;
                    case 'S':
                        model->setItem(i, 4, new QStandardItem("休眠"));
                        break;
                    case 'D':
                        model->setItem(i, 4, new QStandardItem("TASK_UNINTERRUPTIBLE"));
                        break;
                    case 'T':
                        model->setItem(i, 4, new QStandardItem("停止"));
                        break;
                    case 'Z':
                        model->setItem(i, 4, new QStandardItem("Zombie"));
                        break;
                    case 'W':
                        model->setItem(i, 4, new QStandardItem("Paging"));
                        break;
                    default:
                        break;
                    }
                    model->item(i, 4)->setTextAlignment(Qt::AlignCenter);

                    // compute processes' CPU rate
                    if (total_diff == 0) {
                        model->setItem(i, 1, new QStandardItem("00.00%"));
                    } else {
                        int total_process = utime + stime + cutime + cstime;
                        if (process_map->find(pid) != process_map->end()) {
                            int total_pro_diff = total_process - (*process_map)[pid];
                            float rate = total_pro_diff * 8.0 / total_diff;
                            QString temp = tr("%1%").arg(QString::number(rate * 100, 'f', 2));
                            if (temp.length() < 6) {
                                temp = tr("0%1").arg(temp);
                            }
                            model->setItem(i, 1, new QStandardItem(temp));

                        } else {
                            model->setItem(i, 1, new QStandardItem("00.00%"));
                        }
                        (*process_map)[pid] = total_process;
                    }
                    model->item(i, 1)->setTextAlignment(Qt::AlignCenter);
                    // process runing time
                    std::ifstream uptime;
                    uptime.open("/proc/uptime");
                    if (uptime.is_open()) {
                        float total_time;
                        uptime >> total_time;
                        start_time = (int)(total_time - start_time * 1.0 / CLK_TCK);
                        // qDebug() << pid << "\t" << start_time << endl;
                        QDateTime time = QDateTime::fromTime_t(start_time, Qt::UTC, -8 * 60 * 60);
                        QString s_time = time.toString("hh:mm:ss");
                        model->setItem(i, 2, new QStandardItem(s_time));
                        uptime.close();
                        model->item(i, 2)->setTextAlignment(Qt::AlignCenter);
                    }
                }
                input.close();
                // username
                char filename2[20] = "/proc/";
                strcat(filename2, ptr->d_name);
                std::ifstream input2;
                input2.open(strcat(filename2, "/status"));
                if (input2.is_open()) {
                    int j = 0;
                    char buffer[100];
                    while (j < 7) {
                        j++;
                        input2.getline(buffer, 100);
                    }
                    std::string s_uid;
                    int uid;
                    input2 >> s_uid >> uid;
                    if (username_dict->find(uid) != username_dict->end()) {
                        std::string user = (*username_dict)[uid];
                        //qDebug() << pid << "\t" << user.c_str();
                        model->setItem(i, 6, new QStandardItem(user.c_str()));
                    }
                    else {
                        model->setItem(i, 6, new QStandardItem("nobody"));
                    }
                    model->item(i, 6)->setTextAlignment(Qt::AlignLeft);
                    if (this->state == MEM) {
                        j = 0;
                        while (j < 9) {
                            j++;
                            input2.getline(buffer, 100);
                        }
                        std::string tmp;
                        int vmsize, vmrss;
                        input2 >> tmp >> vmsize;
                        input2.getline(buffer, 100);
                        input2.getline(buffer, 100);
                        input2.getline(buffer, 100);
                        input2.getline(buffer, 100);
                        input2 >> tmp >> vmrss;
                        float rate = vmrss * 100.0 / this->mem_size;
                        QString temp = tr("%1%").arg(QString::number(rate, 'f', 2));
                        if (temp.length() < 6) {
                            temp = tr("0%1").arg(temp);
                        }
                        model->setItem(i, 1, new QStandardItem(temp));
                        model->setItem(i, 2, new QStandardItem(get_proper_unit(vmsize).c_str()));
                        model->setItem(i, 3, new QStandardItem(get_proper_unit(vmrss).c_str()));
                        model->item(i, 1)->setTextAlignment(Qt::AlignCenter);
                        //model->sort(1, Qt::DescendingOrder);
                    }
                    input2.close();
                } else {
                    qDebug() << tr("Cannot open status: %1!").arg(ptr->d_name) << endl;
                }
            } else {
                qDebug() << tr("Cannot open %1!").arg(ptr->d_name) << endl;
                i--;
            }
            i++;
        }
    }
    if (this->state == CPU) {
        ui->right_2->setText(tr("进程\t            %1").arg(i));
        ui->right_3->setText(tr("线程\t            %1").arg(total_threads));
    }
    model->sort(1, Qt::DescendingOrder);
    // qDebug() << "now: " << cnt << "last:" << this->num_processes << "rows:" << model->rowCount();
    if (cnt < this->num_processes) {
        int diff= this->num_processes - cnt;
        model->removeRows(model->rowCount() - diff, diff);
    }
    this->num_processes = cnt;
}

void MainWindow::on_cpu_button_clicked()
{
    ui->cpu_button->setEnabled(false);
    ui->mem_button->setEnabled(true);
    ui->mem_button->setStyleSheet("background-color:rgb(243,243,243);");
    ui->cpu_button->setStyleSheet("background-color:rgb(108,107,108);color:rgb(255,254,250);");
    this->state = CPU;
    init_table();
    ui->left_1->setText(tr("系统:\t          0.00%"));
    ui->left_2->setText(tr("用户:\t          0.00%"));
    ui->left_3->setText(tr("空闲:\t          0.00%"));
    ui->right_1->setText(tr("总利用率:           0.0%"));
    ui->right_1->setStyleSheet("QLabel {\n	color:rgb(242, 21, 21)\n}");
    ui->frame_center->change_queue();
    get_cpu_stat();
    refocus();
}

void MainWindow::on_mem_button_clicked()
{
    ui->mem_button->setEnabled(false);
    ui->cpu_button->setStyleSheet("background-color:rgb(243,243,243);");
    ui->cpu_button->setEnabled(true);
    ui->mem_button->setStyleSheet("background-color:rgb(108,107,108);color:rgb(255,254,250);");
    this->state = MEM;
    init_table();
    ui->left_1->setText(tr("物理内存:      00.00 GB"));
    ui->left_2->setText(tr("已使用内存:  00.00 GB"));
    ui->left_3->setText(tr("可用内存:      00.00 GB"));
    ui->right_1->setText(tr("总使用率:          0.0%"));
    ui->right_2->setText(tr("已缓存文件:  00.0 MB"));
    ui->right_3->setText(tr("已使用的交换: 00 KB"));
    ui->right_1->setStyleSheet("QLabel {\n	color:rgb(242, 21, 21)\n}");
    ui->frame_center->change_queue();
    get_mem_stat();
    refocus();
}

void MainWindow::init_table()
{
    // ui->tableView->setSortingEnabled(true);
    switch(state) {
    case CPU:
        model = this->cpu_model;
        ui->tableView->setModel(model);
        // ui->tableView->setModel(this->proxy_model);
        // ui->tableView->sortByColumn(1, Qt::DescendingOrder);
        model->setColumnCount(7);
        model->setHeaderData(0, Qt::Horizontal, "进程名称");
        model->setHeaderData(1, Qt::Horizontal, "  % CPU");
        model->setHeaderData(2, Qt::Horizontal, " CPU时间");
        model->setHeaderData(3, Qt::Horizontal, "    线程");
        model->setHeaderData(4, Qt::Horizontal, "闲置唤醒");
        model->setHeaderData(5, Qt::Horizontal, "      PID");
        model->setHeaderData(6, Qt::Horizontal, "用户");
        ui->tableView->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
        model->sort(1, Qt::DescendingOrder);
        break;
    case MEM:
        model = this->mem_model;
        ui->tableView->setModel(model);
        model->setColumnCount(7);
        model->setHeaderData(0, Qt::Horizontal, "进程名称");
        model->setHeaderData(1, Qt::Horizontal, "  % MEM");
        model->setHeaderData(2, Qt::Horizontal, "虚拟内存");
        model->setHeaderData(3, Qt::Horizontal, "物理内存");
        model->setHeaderData(4, Qt::Horizontal, "    线程");
        model->setHeaderData(5, Qt::Horizontal, "      PID");
        model->setHeaderData(6, Qt::Horizontal, "用户");
        ui->tableView->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
        model->sort(1, Qt::DescendingOrder);
        break;

    default:
        break;
    }
    ui->tableView->verticalHeader()->hide();
    ui->tableView->setColumnWidth(0, 130);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);
}



void MainWindow::on_tableView_clicked(const QModelIndex &index)
{
    int row = index.row();
    this->last_pid = model->item(row, 5)->text().toInt();
    ui->pushButton_3->setEnabled(true);
}

void MainWindow::refocus()
{
    if (last_pid == -1) {
        return;
    }
    for (int i = 0; i < model->rowCount(); i++) {
        if (model->item(i, 5)->text().toInt() == last_pid) {
            ui->tableView->setCurrentIndex(model->index(i, 0));
        }
    }
}

void MainWindow::on_pushButton_3_clicked()
{
    QMessageBox::StandardButton rb = QMessageBox::question(this, tr("温馨提示"),
                  tr("您确定要退出此进程吗"), QMessageBox::Yes | QMessageBox::No);
    if (rb == QMessageBox::Yes && this->last_pid != -1) {
        kill(last_pid, SIGTERM);
        ui->pushButton_3->setEnabled(false);
        last_pid = -1;
    }
}

void MainWindow::on_pushButton_clicked()
{
    Info_dialog info;
    info.exec();
}
