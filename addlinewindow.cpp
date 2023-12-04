#include "addlinewindow.h"
#include "ui_addlinewindow.h"

addLineWindow::addLineWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::addLineWindow)
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

    QPushButton *okButton = ui->buttonBox->button(QDialogButtonBox::Ok);
    QPushButton *cancelButton = ui->buttonBox->button(QDialogButtonBox::Cancel);

    okButton->setText("确认");
    okButton->setCursor(Qt::PointingHandCursor);
    cancelButton->setText("取消");
    cancelButton->setCursor(Qt::PointingHandCursor);

    ui->colorChooser->setCursor(Qt::PointingHandCursor);
    ui->colorChooser->setText("");

    // 绑定文本框
    connect(ui->lineName, &QLineEdit::textEdited, [&](QString name){
        lineName = name;
    });

    // 选择颜色的逻辑
    connect(ui->colorChooser,&QPushButton::clicked,[&](bool){
        QColorDialog* dialog = new QColorDialog(this);
        int result = dialog->exec();
        if (result == QDialog::Accepted) {
            // 绑定颜色
            lineColor = dialog->selectedColor();
            // 显示刚刚选择的颜色
            QString colorString = QString("border-radius:10px; background-color:rgb(%1, %2, %3)").arg(lineColor.red()).arg(lineColor.green()).arg(lineColor.blue());
            ui->colorChooser->setStyleSheet(colorString);
        }
    });

    connect(okButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

addLineWindow::~addLineWindow()
{
    delete ui;
}
