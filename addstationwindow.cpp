#include "addstationwindow.h"
#include "ui_addstationwindow.h"

addStationWindow::addStationWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::addStationWindow)
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

    ui->title->setAlignment(Qt::AlignHCenter | Qt::AlignCenter);

    ui->stationType->addItem("常规");
    ui->stationType->addItem("垂直");
    ui->stationType->addItem("水平");
    ui->stationType->addItem("对角线");
    ui->stationType->addItem("反对角");

    ui->tagPosition->addItem("上方");
    ui->tagPosition->addItem("下方");
    ui->tagPosition->addItem("左侧");
    ui->tagPosition->addItem("右侧");
    ui->tagPosition->addItem("左上");
    ui->tagPosition->addItem("右上");
    ui->tagPosition->addItem("左下");
    ui->tagPosition->addItem("右下");

    ui->longitude->setMinimum(120.87);
    ui->longitude->setMaximum(122.2);
    ui->longitude->setValue(121);

    ui->latitude->setMinimum(30.67);
    ui->latitude->setMaximum(31.88);
    ui->latitude->setValue(31);


    QPushButton *okButton = ui->buttonBox->button(QDialogButtonBox::Ok);
    QPushButton *cancelButton = ui->buttonBox->button(QDialogButtonBox::Cancel);

    okButton->setText("确认");
    okButton->setCursor(Qt::PointingHandCursor);
    cancelButton->setText("取消");
    cancelButton->setCursor(Qt::PointingHandCursor);

    connect(okButton, &QPushButton::clicked, this, &addStationWindow::handleOk);
    connect(cancelButton, &QPushButton::clicked, this, &addStationWindow::close);
}

void addStationWindow::handleOk(){
    metroStationData* station = new metroStationData(ui->name->text(),
                QPointF(ui->longitude->value(), ui->latitude->value()),
                STATION_TYPE(ui->stationType->currentIndex()), NAMETAG_POSITION(ui->tagPosition->currentIndex()));

    // 发送信号并关闭窗口
    emit newStationCreated(station);
    close();
}

addStationWindow::~addStationWindow()
{
    delete ui;
}
