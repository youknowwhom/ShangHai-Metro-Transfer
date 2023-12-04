#ifndef MESSAGEWINDOW_H
#define MESSAGEWINDOW_H

#include <QDialog>
#include <QGraphicsDropShadowEffect>

namespace Ui {
class messageWindow;
}

class messageWindow : public QDialog
{
    Q_OBJECT

public:
    explicit messageWindow(QString title, QString content, QWidget *parent = nullptr);
    ~messageWindow();

private:
    Ui::messageWindow *ui;
};

#endif // MESSAGEWINDOW_H
