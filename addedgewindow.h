#ifndef ADDEDGEWINDOW_H
#define ADDEDGEWINDOW_H

#include <QDialog>
#include <QGraphicsDropShadowEffect>
#include "metromapdata.h"
#include "addlinewindow.h"
#include "messagewindow.h"

namespace Ui {
class addEdgeWindow;
}

/**
 * @brief 添加站点直接连边对话框窗口
 */
class addEdgeWindow : public QDialog
{
    Q_OBJECT

public:
    explicit addEdgeWindow(QStringList _lines, QWidget *parent = nullptr);
    ~addEdgeWindow();

private:
    QStringList lines;
    Ui::addEdgeWindow *ui;
    void handleOk();
    void handleNewLineCreate();

signals:
    void newEdgeCreated(QString name1, QString name2, QString line);
    void newLineCreated(QString name, QColor color);
};

#endif // ADDEDGEWINDOW_H
