#include "drawer.h"
#include "ui_drawer.h"

Drawer::Drawer(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::Drawer)
{
    ui->setupUi(this);
    cpu_queue = new std::vector<Point>;
    mem_queue = new std::vector<Point>;
    data_queue = cpu_queue;
}

void Drawer::paintEvent(QPaintEvent *event)
{
    QPainterPath path;
    int n = data_queue->size();

    path.moveTo(width - 1, height - 1);

    for (int i = 0; i < n; i++) {
        path.lineTo((*data_queue)[i].x, (*data_queue)[i].y);
    }
    if (n != 0) {
        path.lineTo((*data_queue)[n - 1].x, height - 1);
    }

    QPainter painter(this);
    painter.setPen(QPen(QColor(250, 50, 38), 1, Qt::SolidLine,
                        Qt::FlatCap, Qt::MiterJoin));
    painter.setBrush(QColor(255, 183, 177));
    painter.drawPath(path);
}

void Drawer::change_queue()
{
    if (data_queue == cpu_queue) {
        data_queue = mem_queue;
    } else {
        data_queue = cpu_queue;
    }
}

/**
 * @brief Drawer::add_data
 * @param data  0 <= data <= 1
 */
void Drawer::add_data(float data)
{
    int y = (int)(this->height * data);
    int n = data_queue->size();
    int x = 0;
    if (n >= this->size) {
        // delete the last one
        data_queue->pop_back();
        n--;
    }
    // move forward
    for (int i = 0; i < n; i++) {
        (*data_queue)[i].x -= 2;
    }
    Point point(x, y);
    // push front
    data_queue->insert(data_queue->begin(), point);
    // update canvas
    update();
}

Drawer::~Drawer()
{
    delete cpu_queue;
    delete mem_queue;
    delete ui;
}
