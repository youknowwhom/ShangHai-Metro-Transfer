#include "graphview.h"
#include "metromapdata.h"

/* metroGraphicsView */

metroGraphicsView::metroGraphicsView(qint16 _leftTopx, qint16 _leftTopy, qint16 _width, qint16 _height, QWidget* parent):
    QGraphicsView (parent),
    leftTopx(_leftTopx),
    leftTopy(_leftTopy),
    width(_width),
    height(_height)
{
    this->move(leftTopx, leftTopy);
    this->resize(width, height);
    this->setStyleSheet("border-radius: 20px; background-color: white;");
    this->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform); // 抗锯齿
    this->setMouseTracking(true);                                       // 不按下时也追踪鼠标
    this->setDragMode(QGraphicsView::ScrollHandDrag);                   // 拖拽模式
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);         // 隐藏水平滚动条
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);           // 隐藏竖直滚动条
    this->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);     // 以鼠标居中进行放缩
    this->setResizeAnchor(QGraphicsView::AnchorUnderMouse);

    // 绑定metroGraphicsScene
    metroGraphicsScene = new QGraphicsScene(this);
    this->scale(0.49, 0.49);
    this->setScene(metroGraphicsScene);

    // 建立metroMapData类
    metroMap = new metroMapData;
}

void metroGraphicsView::loadConfig()
{
    if(!metroMap->readConfig())
        emit errorMessage("读取配置文件错误！请务必保证配置文件正确！");

    // 先打印所有边
    for(auto& edge: metroMap->getMetroGraphicEdges()){
        metroGraphicsScene->addItem(edge);
        metroGraphicsEdges.append(edge);
    }

    // 再打印站点
    for(auto& station: metroMap->getMetroGraphicStations()){
        metroGraphicsScene->addItem(station);
        metroGraphicsStations.append(station);
        connect(station, &metroGraphicsStationItem::clicked, this, &metroGraphicsView::handleStationClicked);
        station->setCursor(Qt::PointingHandCursor);    // 设置鼠标形态
    }

    // 绘制图例
    drawLegend();
}

void metroGraphicsView::visualizeFindPath(QString start, QString end, FIND_PATH_METHOD method)
{
    QQueue<metroEdgeData *> path;

    if(start == "" || end == ""){
        emit errorMessage("站点名称不能为空！");
        return;
    }
    else if(start == end){
        emit errorMessage("起始站不能与目的站相同！");
        return;
    }

    switch (method) {
        case FIND_PATH_METHOD::COUNT:
            path = metroMap->findPath(start, end, &metroMapData::getDistanceByCount);
            break;
        case FIND_PATH_METHOD::DISTANCE:
            path = metroMap->findPath(start, end, &metroMapData::getDistanceByDistance);
            break;
        case FIND_PATH_METHOD::TRANSFER:
            path = metroMap->findPath(start, end, &metroMapData::getDistanceByTransfer);
            break;
    }

    if(path.empty()){
        emit errorMessage("站点名称输入有误！");
        return;
    }

    // 先将所有点和边置为灰色
    for(auto& station: metroGraphicsStations){
        station->setZValue(-1);
        station->colorTurnGray();
    }
    for(auto& edge: metroGraphicsEdges){
        edge->setZValue(-2);
        edge->colorTurnGray();
    }

    // 输出的换乘指南
    QString content;
    QString lastLine;

    // 首站点额外恢复
    path.front()->getStart()->getGraphicItem()->colorRevert();
    content += "<p style='line-height:40px'>在起始站" + path.front()->getStart()->getName() + "乘坐" + path.front()->getLine() + "<br/>";
    lastLine = path.front()->getLine();

    for(auto& edge: path){
        // qDebug() << edge->getStart()->getName() << edge->getLine() << edge->getEnd()->getName();
        // 边显色
        edge->getGraphicItem()->colorRevert();
        edge->getGraphicItem()->setZValue(1);

        // 初始站点和终止站点都要重新显示 防止被遮盖
        edge->getStart()->getGraphicItem()->colorRevert();
        edge->getEnd()->getGraphicItem()->colorRevert();
        edge->getStart()->getGraphicItem()->setZValue(2);
        edge->getEnd()->getGraphicItem()->setZValue(2);

        if(edge->getLine() != lastLine){
            content += "在" + edge->getStart()->getName() + "换乘" + edge->getLine() + "<br/>";
            lastLine = edge->getLine();
        }
    }

    content += "抵达目的站" + path.back()->getEnd()->getName() + "</p>";

    emit pathGuidanceGenerated(content);
}

metroGraphicsView::~metroGraphicsView()
{
    delete metroMap;
}

void metroGraphicsView::drawLegend()
{
    QVector<metroLineData *> metroLines = metroMap->getMetroLines();
    for(int i  = 0; i <  metroLines.count(); i++){
        qreal xDelta = i % 6 * 90;  // 每六个一行
        qreal yDelta = i / 6 * 70;

        QGraphicsRectItem* newRect = new QGraphicsRectItem(legendLeftTop.x() + xDelta, legendLeftTop.y() + yDelta, 25, 25);
        QGraphicsTextItem* newText = new QGraphicsTextItem(metroLines[i]->getName(), newRect);

        newRect->setBrush(metroLines[i]->getColor());
        newRect->setPen(Qt::NoPen);

        newText->setPos(legendLeftTop.x() + xDelta + 30, legendLeftTop.y() + yDelta);
        newText->setFont(legendFont);

        legendRect.append(newRect);

        metroGraphicsScene->addItem(newRect);
    }
}

// 清除绘制的图例
void metroGraphicsView::clearLegend()
{
    for(auto& rect: legendRect){
        metroGraphicsScene->removeItem(rect);
        delete rect;
    }
}


void metroGraphicsView::mousePressEvent(QMouseEvent *e)
{
    // 先置为不在拖拽
    isDragging = false;

    // 调用基类方法向下传递
    QGraphicsView::mousePressEvent(e);
}

void metroGraphicsView::mouseMoveEvent(QMouseEvent *e)
{
    // 在拖拽
    isDragging = true;

    // 调用基类方法向下传递
    QGraphicsView::mouseMoveEvent(e);
}

void metroGraphicsView::mouseReleaseEvent(QMouseEvent *e)
{
    // 在创建新站点 但并非拖拽
    if(!isDragging && isCreatingNewStation){
        // 此时创建新站点
        newStation->setId(metroMap->getStationNum() + 1);
        newStation->setGraphPosition(mapToScene(e->pos()).toPoint());

        metroGraphicsStationItem* graphicStation =  metroMap->addNewStation(newStation);
        // 绘制到画布并进行记录
        metroGraphicsScene->addItem(graphicStation);
        metroGraphicsStations.push_back(graphicStation);

        // 建立新的信号槽并设置鼠标可点击
        graphicStation->setCursor(Qt::PointingHandCursor);
        connect(graphicStation, &metroGraphicsStationItem::clicked, this, &metroGraphicsView::handleStationClicked);

        isCreatingNewStation = false;
    }

    // 调用基类方法向下传递
    QGraphicsView::mouseReleaseEvent(e);
}

void metroGraphicsView::wheelEvent (QWheelEvent * e)
{
    // 最大缩放倍数
    if((e->delta() > 0)&&(this->matrix().m11() >= 5))
        return;
    // 最小缩放倍数
    else if((e->delta() < 0)&&(this->matrix().m11() <= 0.49))
        return;
    else    {
        int wheelDeltaValue = e->delta();
        // 向上滚动，放大
        if (wheelDeltaValue > 0)
            this->scale(1.2, 1.2);
        // 向下滚动，缩小
        else
            this->scale(1.0 / 1.2, 1.0 / 1.2);
        update();
    }
}

void metroGraphicsView::handlePickStart()
{
    isPickingStart = true;
    isPickingEnd = false;
}

void metroGraphicsView::handlePickEnd()
{
    isPickingEnd = true;
    isPickingStart = false;
}

void metroGraphicsView::handleFindPath()
{
    visualizeFindPath(start, end, static_cast<FIND_PATH_METHOD>(mode));
}

void metroGraphicsView::handleRevert()
{
    for(auto& edge: metroGraphicsEdges)
        edge->colorRevert();
    for(auto& station: metroGraphicsStations)
        station->colorRevert();
}

void metroGraphicsView::handleModeChanged(int _mode)
{
    mode = _mode;
}

void metroGraphicsView::handleStartChanged(const QString& name)
{
    start = name;
}

void metroGraphicsView::handleEndChanged(const QString& name)
{
    end = name;
}

void metroGraphicsView::handleStationClicked(QString name)
{
    if(isPickingStart){
        emit startPicked(name);
        isPickingStart = false;
    }
    else if(isPickingEnd){
        emit endPicked(name);
        isPickingEnd = false;
    }
}

// 处理新建站点按钮点击
void metroGraphicsView::handleNewStationCreate()
{
    addStationWindow window;
    connect(&window, &addStationWindow::newStationCreated, this, &metroGraphicsView::handleNewStationSubmmited);
    window.exec();
}

// 处理新建连边按钮点击
void metroGraphicsView::handleNewEdgeCreate()
{
    addEdgeWindow window(metroMap->getLinesName());
    connect(&window, &addEdgeWindow::newEdgeCreated, this, &metroGraphicsView::handleNewEdgeSubmmited);
    connect(&window, &addEdgeWindow::newLineCreated, this, &metroGraphicsView::handleNewLineCreated);
    window.exec();
}

void metroGraphicsView::handleNewStationSubmmited(metroStationData* _newStation)
{
    if(_newStation->getName() == ""){
        emit errorMessage("站点名称不能为空，创建站点失败！");
        delete _newStation;
    }
    else if(metroMap->isStationExisted(_newStation->getName())){
        emit errorMessage("已有同名站点，创建站点失败！");
        delete _newStation;  // 创建失败 释放
    }
    else{
        emit cueMessage("站点记录成功，请在线路图上挑选位置点击以放置站点！");
        isCreatingNewStation = true;    // 开始选取新站点
        newStation = _newStation;
    }
}

void metroGraphicsView::handleNewEdgeSubmmited(QString name1, QString name2, QString line)
{
    if(!metroMap->isStationExisted(name1) || !metroMap->isStationExisted(name2)){
        emit errorMessage("站点名称不存在，创建线路连边失败！");
    }
    else{
        metroGraphicsEdgeItem * edge = metroMap->addNewEdges(name1, name2, line);

        // 绘制到画布并进行记录
        metroGraphicsScene->addItem(edge);
        edge->setZValue(-2);    // 防止遮挡其他边
        metroGraphicsEdges.push_back(edge);

        emit cueMessage("创建线路连边成功！");
    }
}

void metroGraphicsView::handleNewLineCreated(QString name, QColor color)
{
    // 记录新的线路
    metroMap->addNewLine(name, color);
    // 清除并重新绘制图例
    clearLegend();
    drawLegend();
}

void metroGraphicsView::handleSaveConfig()
{
    if(metroMap->writeConfig())
        emit cueMessage("已成功保存到配置文件！");
    else
        emit errorMessage("配置文件保存失败！");
}


/* 地铁站点：metroGraphicsStationItem */

metroGraphicsStationItem::metroGraphicsStationItem(qint16 _id, QString _name, QPoint _graphPst, QPointF _realPst, STATION_TYPE _type, NAMETAG_POSITION _tagPst, QGraphicsItem* parent):
    QGraphicsItem (parent),
    type(_type),
    showColor(Qt::black),
    graphPosition(_graphPst),
    realPosition(_realPst),
    id(_id),
    name(_name),
    nametagPosition(_tagPst)
{
    QPointF offset;

    nameTag = new QGraphicsSimpleTextItem(this);
    nameTag->setFont(nameFont);
    // 不显示括号内的注释
    QStringList processName = name.split("(");
    nameTag->setText(processName[0]);

    // 计算偏移距离
    if(nametagPosition == NAMETAG_POSITION::BOTTOM)
        offset = QPointF(- nameTag->boundingRect().width() / 2, 1.5 * radius);
    else if(nametagPosition == NAMETAG_POSITION::LEFT)
        offset = QPointF(-2 * radius - nameTag->boundingRect().width(), - QFontMetrics(nameFont).height() / 2);
    else if(nametagPosition == NAMETAG_POSITION::RIGHT)
        offset = QPointF(2 * radius, - QFontMetrics(nameFont).height() / 2);
    else if(nametagPosition == NAMETAG_POSITION::BOTTOM_LEFT)
        offset = QPointF(-1.2 * radius - nameTag->boundingRect().width(), 1.2 * radius);
    else if(nametagPosition == NAMETAG_POSITION::TOP_LEFT)
        offset = QPointF(-1.2 * radius - nameTag->boundingRect().width(), -1.2 * radius - QFontMetrics(nameFont).height());
    else if(nametagPosition == NAMETAG_POSITION::BOTTOM_RIGHT)
        offset = QPointF(1.2 * radius, 1.2 * radius);
    else if(nametagPosition == NAMETAG_POSITION::TOP_RIGHT)
        offset = QPointF(1.2 * radius, - 1.2 * radius - QFontMetrics(nameFont).height());
    else // 默认tag在上方
        offset = QPointF(- nameTag->boundingRect().width() / 2, - 1.5 * radius - QFontMetrics(nameFont).height());

    nameTag->setPos(graphPosition + offset);
    nameTag->setBrush(Qt::black);

    // 设置悬浮Tips
    this->setToolTip(name + "\n东经：" + QString::number(realPosition.x()) + "°\n北纬：" + QString::number(realPosition.y()) + "°");
}

void metroGraphicsStationItem::paint(QPainter* painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setBrush(Qt::white);
    painter->setPen(QPen(showColor, 2));

    QPainterPath path;

    if(type == STATION_TYPE::DIAGONAL){
        path.moveTo(graphPosition.x() - radius * M_SQRT2, graphPosition.y());
        path.arcTo(QRectF(graphPosition.x() + radius * (-1 - M_SQRT1_2), graphPosition.y() + radius * (-1 + M_SQRT1_2), 2 * radius, 2 * radius), 135, 180);
        path.lineTo(graphPosition.x() + radius * M_SQRT2, graphPosition.y());
        path.arcTo(QRectF(graphPosition.x() + radius * (-1 + M_SQRT1_2), graphPosition.y() + radius * (-1 - M_SQRT1_2), 2 * radius, 2 * radius), 315, 180);
        path.closeSubpath();
    }
    else if(type == STATION_TYPE::B_DIAGONAL){
        path.moveTo(graphPosition.x(), graphPosition.y() + radius * M_SQRT2);
        path.arcTo(QRectF(graphPosition.x() + radius * (-1 + M_SQRT1_2), graphPosition.y() + radius * (-1 + M_SQRT1_2), 2 * radius, 2 * radius), 225, 180);
        path.lineTo(graphPosition.x(), graphPosition.y() - radius * M_SQRT2);
        path.arcTo(QRectF(graphPosition.x() + radius * (-1 - M_SQRT1_2), graphPosition.y() + radius * (-1 - M_SQRT1_2), 2 * radius, 2 * radius), 45, 180);
        path.closeSubpath();
    }
    else if(type == STATION_TYPE::HORIZONTAL){
        path.moveTo(graphPosition.x() - radius, graphPosition.y() - radius);
        path.arcTo(QRect(graphPosition.x() - 2 * radius, graphPosition.y() - radius, 2 * radius,  2 * radius), 90, 180);
        path.lineTo(graphPosition.x() + radius, graphPosition.y() + radius);
        path.arcTo(QRect(graphPosition.x(), graphPosition.y() - radius, 2 * radius,  2 * radius), 270, 180);
        path.closeSubpath();
    }
    else if(type == STATION_TYPE::VERTICAL){
        path.moveTo(graphPosition.x() - radius, graphPosition.y() + radius);
        path.arcTo(QRect(graphPosition.x() - radius, graphPosition.y(), 2 * radius,  2 * radius), 180, 180);
        path.lineTo(graphPosition.x() + radius, graphPosition.y() - radius);
        path.arcTo(QRect(graphPosition.x() - radius, graphPosition.y() - 2 * radius, 2 * radius,  2 * radius), 0, 180);
        path.closeSubpath();
    }
    // 默认形态是圆形
    else
        path.addEllipse(graphPosition.x() - radius, graphPosition.y() - radius, 2 * radius, 2 * radius);

    painter->drawPath(path);
    painter->fillPath(path, Qt::white);
}


QRectF metroGraphicsStationItem::boundingRect() const
{
    return QRectF(graphPosition.x() - 2 * radius, graphPosition.y() - 2 * radius, 4 * radius, 4 * radius);
}


QPoint metroGraphicsStationItem::getGraphPosition() const
{
    return graphPosition;
}

void metroGraphicsStationItem::mousePressEvent(QGraphicsSceneMouseEvent *e)
{
    Q_UNUSED(e);
    emit clicked(name);
}

void metroGraphicsStationItem::colorTurnGray()
{
    setZValue(10);
    nameTag->setBrush(QColor(220, 220, 220));
    showColor = QColor(220, 220, 220);
    update();
}

void metroGraphicsStationItem::colorRevert()
{
    setZValue(10);
    nameTag->setBrush(Qt::black);
    showColor = Qt::black;
    update();
}



/* 地铁边：metroGraphicsEdgeItem */

metroGraphicsEdgeItem::metroGraphicsEdgeItem(QPoint _start, QPoint _end, QColor _lineColor, QVector<QPoint> _via, QGraphicsItem* parent):
    QGraphicsItem (parent),
    start(_start),
    end(_end),
    via(_via),
    lineColor(_lineColor),
    showColor(_lineColor)
{
}

void metroGraphicsEdgeItem::paint(QPainter* painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setBrush(Qt::white);
    painter->setPen(QPen(showColor, 10, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

    QPoint pre = start;
    for(int i = 0; i < via.count(); i++){
        painter->drawLine(pre, via[i]);
        pre = via[i];
    }
    painter->drawLine(pre, end);
}


QRectF metroGraphicsEdgeItem::boundingRect() const
{
    qreal minx = qMin(start.x(), end.x()) - 10;
    qreal maxx = qMax(start.x(), end.x()) + 10;
    qreal miny = qMin(start.y(), end.y()) - 10;
    qreal maxy = qMax(start.y(), end.y()) + 10;
    return QRectF(minx, miny, maxx - minx, maxy - miny);
}

void metroGraphicsEdgeItem::colorTurnGray()
{
    showColor = QColor(220, 220, 220);
    update();
}

void metroGraphicsEdgeItem::colorRevert()
{
    showColor = lineColor;
    update();
}
