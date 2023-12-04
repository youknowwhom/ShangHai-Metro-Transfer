#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
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
    ui->mainWindowFillet->setGraphicsEffect(shadow);

    // 画布界面
    metroGraphicsView *view = new metroGraphicsView(75, 180, 1000, 930, this);
    connect(view, &metroGraphicsView::pathGuidanceGenerated, this, &MainWindow::handleGuidanceGenerated);
    connect(view, &metroGraphicsView::errorMessage, this, &MainWindow::handleErrorMessage);
    connect(view, &metroGraphicsView::cueMessage, this, &MainWindow::handleCueMessage);
    view->loadConfig();
    view->show();

    // 右侧栏
    QWidget* rightBar = new QWidget(this);
    rightBar->setFixedSize(420, 960);
    rightBar->move(1095, 185);
    QGridLayout* layOut = new QGridLayout();

    QString headingStyle = "max-height:50px; font-size:30px; font-family:'MiSans DemiBold';";
    QString buttonStyle = "min-height:60px; border-radius:10px; font-size:22px; font-family:'MiSans'; background-color:rgba(195, 195, 195, 0.5);";

    QLabel* startLabel = new QLabel("起始站");
    QLabel* endLabel = new QLabel("终点站");
    startLabel->setStyleSheet(headingStyle);
    endLabel->setStyleSheet(headingStyle);

    QString lineEditStyle = "height: 55px; font-size:22px; font-family:'MiSans Light'; border-radius:10px; padding-left:15px; padding-right:15px;";
    stationCompleter = new QCompleter();
    startLineEdit = new QLineEdit();
    endLineEdit = new QLineEdit();
    connect(startLineEdit, &QLineEdit::textChanged, view, &metroGraphicsView::handleStartChanged);
    connect(endLineEdit, &QLineEdit::textChanged, view, &metroGraphicsView::handleEndChanged);
    startLineEdit->setCompleter(stationCompleter);

    QLabel* comboBoxLabel = new QLabel("寻路模式");
    comboBoxLabel->setStyleSheet("font-family:'MiSans Demibold'; font-size: 20px;");
    QComboBox* comboBox = new QComboBox();
    comboBox->setStyleSheet("min-height:50px; font-size:20px; font-family:'MiSans Light';");
    comboBox->addItem("站点数优先");
    comboBox->addItem("距离优先");
    comboBox->addItem("换乘次数优先");
    comboBox->setView(new QListView());
    setStyleSheet("QComboBox QAbstractItemView::item{height:30px;}");
    connect(comboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), view, &metroGraphicsView::handleModeChanged);

    QLabel* promptLabel = new QLabel("注：点击右侧图标后可直接在左侧视图中点击选择站点");
    promptLabel->setStyleSheet("font-family:'MiSans Light'; font-size: 15px; color:rgb(98, 98, 98); max-height: 20px; padding-bottom: 10px;");

    // 输入框阴影效果
    QGraphicsDropShadowEffect* lineShadowEffect = new QGraphicsDropShadowEffect(this);
    lineShadowEffect->setBlurRadius(10);
    lineShadowEffect->setColor(QColor(0, 0, 0, 64));
    lineShadowEffect->setOffset(3, 3);
    QGraphicsDropShadowEffect* lineShadowEffect2 = new QGraphicsDropShadowEffect(this);
    lineShadowEffect2->setBlurRadius(5);
    lineShadowEffect2->setColor(QColor(0, 0, 0, 64));
    lineShadowEffect2->setOffset(3, 3);

    // 选择按钮
    QIcon selectIcon(":image/choose.png");
    QPushButton* chooseStartButton = new QPushButton(selectIcon, "", this);
    chooseStartButton->setFixedSize(25, 25);
    chooseStartButton->setStyleSheet("background-color:rgba(0,0,0,0)");
    chooseStartButton->setIconSize(QSize(25, 25));
    chooseStartButton->setCursor(Qt::PointingHandCursor);
    chooseStartButton->setToolTip("在左侧视图选定起始站点");
    connect(chooseStartButton, &QPushButton::clicked, view, &metroGraphicsView::handlePickStart);
    connect(view, &metroGraphicsView::startPicked, this, &MainWindow::handlePickStart);

    QPushButton* chooseEndButton = new QPushButton(selectIcon, "", this);
    chooseEndButton->setFixedSize(25, 25);
    chooseEndButton->setStyleSheet("background-color:rgba(0,0,0,0)");
    chooseEndButton->setIconSize(QSize(25, 25));
    chooseEndButton->setCursor(Qt::PointingHandCursor);
    chooseEndButton->setToolTip("在左侧视图选定目的站点");
    connect(chooseEndButton, &QPushButton::clicked, view, &metroGraphicsView::handlePickEnd);
    connect(view, &metroGraphicsView::endPicked, this, &MainWindow::handlePickEnd);

    startLineEdit->setStyleSheet(lineEditStyle);
    endLineEdit->setStyleSheet(lineEditStyle);
    startLineEdit->setGraphicsEffect(lineShadowEffect);
    endLineEdit->setGraphicsEffect(lineShadowEffect2);

    // 下方的按钮组
    QPushButton* startFindPathButton = new QPushButton("查询线路");
    startFindPathButton->setStyleSheet(buttonStyle);
    startFindPathButton->setCursor(Qt::PointingHandCursor);
    connect(startFindPathButton, &QPushButton::clicked, view, &metroGraphicsView::handleFindPath);

    QPushButton* revertButton = new QPushButton("恢复");
    revertButton->setStyleSheet(buttonStyle);
    revertButton->setCursor(Qt::PointingHandCursor);
    connect(revertButton, &QPushButton::clicked, view, &metroGraphicsView::handleRevert);

    QPushButton* addStationButton = new QPushButton("添加站点");
    addStationButton->setStyleSheet(buttonStyle);
    addStationButton->setCursor(Qt::PointingHandCursor);
    connect(addStationButton, &QPushButton::clicked, view, &metroGraphicsView::handleNewStationCreate);

    QPushButton* addLineButton = new QPushButton("添加线路");
    addLineButton->setStyleSheet(buttonStyle);
    addLineButton->setCursor(Qt::PointingHandCursor);
    connect(addLineButton, &QPushButton::clicked, view, &metroGraphicsView::handleNewEdgeCreate);

    QPushButton* saveButton = new QPushButton("保存配置");
    saveButton->setStyleSheet(buttonStyle);
    saveButton->setCursor(Qt::PointingHandCursor);
    connect(saveButton, &QPushButton::clicked, view, &metroGraphicsView::handleSaveConfig);

    layOut->addWidget(startLabel, 0, 0, 1, 1);
    layOut->addWidget(chooseStartButton, 0, 1, 1, 1);
    layOut->addWidget(startLineEdit, 1, 0, 1, 4);
    layOut->addWidget(endLabel, 2, 0, 1, 1);
    layOut->addWidget(chooseEndButton, 2, 1, 1, 1);
    layOut->addWidget(endLineEdit, 3, 0, 1, 4);
    layOut->addWidget(promptLabel, 4, 0, 1, 4);
    layOut->addWidget(comboBoxLabel, 5, 0, 1, 1);
    layOut->addWidget(comboBox, 5, 1, 1, 3);
    layOut->addWidget(startFindPathButton, 6, 0, 1, 3);
    layOut->addWidget(revertButton, 6, 3, 1, 1);
    layOut->addWidget(addStationButton, 7, 0, 1, 2);
    layOut->addWidget(addLineButton, 7, 2, 1, 2);
    layOut->addWidget(saveButton, 8, 0, 1, 4);

    rightBar->setLayout(layOut);

    // 右上角最小化及关闭按钮
    QIcon minIcon(":image/minimize.png");
    QPushButton* minButton = new QPushButton(minIcon, "", this);
    minButton->setFixedSize(50, 50);
    minButton->setStyleSheet("background-color:rgba(0,0,0,0)");
    minButton->setIconSize(QSize(50, 50));
    minButton->setCursor(Qt::PointingHandCursor);
    minButton->move(1410, 42);
    minButton->show();
    connect(minButton, &QPushButton::clicked, this, &MainWindow::showMinimized);

    QIcon closeIcon(":image/close.png");
    QPushButton* closeButton = new QPushButton(closeIcon, "", this);
    closeButton->setFixedSize(50, 50);
    closeButton->setStyleSheet("background-color:rgba(0,0,0,0)");
    closeButton->setIconSize(QSize(50, 50));
    closeButton->setCursor(Qt::PointingHandCursor);
    closeButton->move(1480, 42);
    closeButton->show();
    connect(closeButton, &QPushButton::clicked, this, &MainWindow::close);

    QString mainStyleSheet = "QToolTip { font-family: 'MiSans Light'; }";
    this->setStyleSheet(mainStyleSheet);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::handleGuidanceGenerated(QString content)
{
    messageWindow* window = new messageWindow("换乘指南", content, this);
    window->show();
}

void MainWindow::handleErrorMessage(QString msg)
{
    messageWindow* window = new messageWindow("错误提示", msg, this);
    window->exec();
}

void MainWindow::handleCueMessage(QString msg)
{
    messageWindow* window = new messageWindow("提示信息", msg, this);
    window->exec();
}

void MainWindow::mousePressEvent(QMouseEvent *e)
{
    // 拖动区域限制
    if(e->button() == Qt::LeftButton && e->localPos().y() < 195){
        QPoint mousePst = e->globalPos();
        mouseOffset = mousePst - geometry().topLeft();
        isValidDragging = true;
    }
    QMainWindow::mousePressEvent(e);
}

void MainWindow::mouseMoveEvent(QMouseEvent *e)
{
    QPoint mousePst = e->globalPos();
    QPoint newWindowPst = mousePst - mouseOffset;
    if(isValidDragging)
        move(newWindowPst);
    QMainWindow::mouseMoveEvent(e);
}

void MainWindow::mouseReleaseEvent(QMouseEvent *e)
{
    Q_UNUSED(e);
    isValidDragging = false;
    QMainWindow::mouseReleaseEvent(e);
}

void MainWindow::handlePickStart(QString name)
{
    startLineEdit->setText(name);
}

void MainWindow::handlePickEnd(QString name)
{
    endLineEdit->setText(name);
}
