#include "info_dialog.h"
#include "ui_info_dialog.h"

Info_dialog::Info_dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Info_dialog)
{
    ui->setupUi(this);
    read();
}

void Info_dialog::read()
{
    std::ifstream input;
    input.open("/proc/cpuinfo");
    if (input.is_open()) {
        char buf[10000];
        input.read(buf, 10000);
        ui->textEdit->setText(buf);
        input.close();
    } else {
        qDebug() << "Cannot open /proc/cpuinfo";
    }
}

Info_dialog::~Info_dialog()
{
    delete ui;
}
