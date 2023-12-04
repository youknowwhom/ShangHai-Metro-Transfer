#ifndef ADDSTATIONWINDOW_H
#define ADDSTATIONWINDOW_H

#include <QDialog>
#include <QPushButton>
#include <QGraphicsDropShadowEffect>
#include "metromapdata.h"
#include "stationType.h"

namespace Ui {
class addStationWindow;
}

/**
 * @brief 添加站点的对话框窗口
 */
class addStationWindow : public QDialog
{
    Q_OBJECT

public:
    explicit addStationWindow(QWidget *parent = nullptr);
    ~addStationWindow();

private:
    Ui::addStationWindow *ui;
    void handleOk();

signals:
    void newStationCreated(metroStationData* newStation);
};

#endif // ADDSTATIONWINDOW_H
