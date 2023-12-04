#ifndef ADDLINEWINDOW_H
#define ADDLINEWINDOW_H

#include <QDialog>
#include <QGraphicsDropShadowEffect>
#include <QPushButton>
#include <QColorDialog>
#include <QDebug>

namespace Ui {
class addLineWindow;
}

/**
 * @brief 添加新线路的对话框窗口
 */
class addLineWindow : public QDialog
{
    Q_OBJECT

public:
    explicit addLineWindow(QWidget *parent = nullptr);
    ~addLineWindow();

    QColor lineColor = Qt::black;
    QString lineName;

private:
    Ui::addLineWindow *ui;
};

#endif // ADDLINEWINDOW_H
