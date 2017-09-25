#ifndef INFO_DIALOG_H
#define INFO_DIALOG_H

#include <QDialog>
#include <fstream>
#include <string>
#include <string.h>
#include "tools/tools.h"
#include <QDebug>

namespace Ui {
class Info_dialog;
}

class Info_dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Info_dialog(QWidget *parent = 0);
    ~Info_dialog();

private:
    Ui::Info_dialog *ui;

    void read();
};

#endif // INFO_DIALOG_H
