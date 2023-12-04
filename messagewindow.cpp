#include "messagewindow.h"
#include "ui_messagewindow.h"

messageWindow::messageWindow(QString title, QString content, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::messageWindow)
{
    ui->setupUi(this);

    // 原窗口隐藏
    setWindowFlag(Qt::FramelessWindowHint);         // 无边框
    setAttribute(Qt::WA_TranslucentBackground);     // 透明背景

    // 窗口阴影绘制
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(20);
    shadow->setOffset(0);
    shadow->setColor(QColor(0, 0, 0, 100));
    ui->mainArea->setGraphicsEffect(shadow);

    ui->title->setText(title);
    ui->title->setAlignment(Qt::AlignHCenter | Qt::AlignCenter);
    ui->scrollContent->setText(content);

    ui->confirmButton->setCursor(Qt::PointingHandCursor);

    connect(ui->confirmButton, &QPushButton::clicked, this, &QDialog::close);
}

messageWindow::~messageWindow()
{
    delete ui;
}
