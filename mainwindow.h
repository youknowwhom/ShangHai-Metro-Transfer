#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPaintEvent>
#include <QPainter>
#include <QColor>
#include <QLabel>
#include <QGraphicsDropShadowEffect>
#include <QColorDialog>
#include <QPushButton>
#include <QLineEdit>
#include <QCompleter>
#include <QComboBox>
#include <QDebug>
#include <QListView>
#include "graphview.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() Q_DECL_OVERRIDE;

    void handlePickStart(QString name);
    void handlePickEnd(QString name);
    void handleGuidanceGenerated(QString content);
    void handleErrorMessage(QString msg);
    void handleCueMessage(QString msg);

protected:
    void mousePressEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *e) Q_DECL_OVERRIDE;

private:
    Ui::MainWindow *ui;

    // 拖拽窗口
    bool isValidDragging = false;   // 是否有效区域内在拖拽窗口
    QPoint mouseOffset;             // 鼠标按下时距离窗口左上角的偏移

    // 输入起止站
    QCompleter* stationCompleter = nullptr;
    QLineEdit* startLineEdit;
    QLineEdit* endLineEdit;

};

#endif // MAINWINDOW_H
