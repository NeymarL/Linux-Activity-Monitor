#ifndef DRAWER_H
#define DRAWER_H

#include <QWidget>
#include <QPainter>
#include <QFrame>
#include <QPainterPath>
#include <vector>
#include <QDebug>

namespace Ui {
class Drawer;
}

struct Point {
    int x;
    int y;
    Point(int x_point, int y_point) {
        x = 159 - x_point;
        y = 100 - y_point;
    }
};

class Drawer : public QFrame
{
    Q_OBJECT

public:
    explicit Drawer(QWidget *parent = 0);
    ~Drawer();
    int X(int x);   // trans coordinate
    int Y(float y);
    void add_data(float data);

    void change_queue();

private:
    void paintEvent(QPaintEvent*);

    Ui::Drawer *ui;
    int width = 160;
    int height = 101;
    int dist = 2;
    std::vector<Point>* data_queue;
    std::vector<Point>* cpu_queue;
    std::vector<Point>* mem_queue;
    int size = width / dist;
};

#endif // DRAWER_H
