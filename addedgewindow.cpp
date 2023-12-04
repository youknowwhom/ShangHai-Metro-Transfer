#include "addedgewindow.h"
#include "ui_addedgewindow.h"

addEdgeWindow::addEdgeWindow(QStringList _lines, QWidget *parent) :
    QDialog(parent),
    lines(_lines),
    ui(new Ui::addEdgeWindow)
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

    // 处理新增线路
    ui->addLine->setCursor(Qt::PointingHandCursor);
    connect(ui->addLine, &QPushButton::clicked, this, &addEdgeWindow::handleNewLineCreate);

    QPushButton *okButton = ui->buttonBox->button(QDialogButtonBox::Ok);
    QPushButton *cancelButton = ui->buttonBox->button(QDialogButtonBox::Cancel);
    okButton->setText("确认");
    okButton->setCursor(Qt::PointingHandCursor);
    cancelButton->setText("取消");
    cancelButton->setCursor(Qt::PointingHandCursor);

    // 添加线路候选项
    ui->line->addItems(lines);

    connect(okButton, &QPushButton::clicked, this, &addEdgeWindow::handleOk);
    connect(cancelButton, &QPushButton::clicked, this, &addEdgeWindow::close);
}


void addEdgeWindow::handleOk()
{
    emit newEdgeCreated(ui->station1->text(), ui->station2->text(), ui->line->currentText());
    addEdgeWindow::close();
}

void addEdgeWindow::handleNewLineCreate(){
    addLineWindow* window = new addLineWindow;
    int ret = window->exec();
    if(ret == QDialog::Accepted){
        if(window->lineName == ""){
            messageWindow msg("错误提示", "线路名称不能为空，新建失败！");
            msg.exec();
        }
        else if(lines.contains(window->lineName)){
            messageWindow msg("错误提示", "该线路已经存在，新建失败！");
            msg.exec();
        }
        else{
            messageWindow msg("提示信息", "新线路[" + window->lineName + "]创建成功！");
            msg.exec();
            ui->line->addItem(window->lineName);
            emit newLineCreated(window->lineName, window->lineColor);
        }
    }
}

addEdgeWindow::~addEdgeWindow()
{
    delete ui;
}
