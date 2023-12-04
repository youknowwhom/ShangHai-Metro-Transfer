#ifndef GRAPHVIEW_H
#define GRAPHVIEW_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QEventLoop>
#include <QTimer>
#include <QMouseEvent>
#include <QTimeLine>
#include <QPainter>
#include <QBrush>
#include <QVector>
#include <QFont>
#include <QPen>
#include <QColor>
#include <QDebug>
#include <QtMath>
#include "stationtype.h"
#include "addstationwindow.h"
#include "addedgewindow.h"
#include "messagewindow.h"

// 地铁显示的画布
class metroGraphicsView;
// 地铁的可视化站点
class metroGraphicsStationItem;
// 地铁的可视化边
class metroGraphicsEdgeItem;
// 地铁线路信息
class metroLineData;

// 前向声明地铁信息
class metroMapData;
// 前向声明站点信息类
class metroStationData;
// 前向声明站边信息类
class metroEdgeData;

enum class FIND_PATH_METHOD { COUNT, DISTANCE, TRANSFER };

// 地铁显示的画布
class metroGraphicsView: public QGraphicsView
{
    Q_OBJECT

    friend class metroGraphicsEdgeItem;
    friend class metroGraphicsStationItem;

private:
    // 画布的位置
    qint16 leftTopx, leftTopy;
    qint16 width, height;

    // 图例
    const QPoint legendLeftTop =  QPoint(0, 1700);          // 左上角位置
    QFont legendFont = QFont("MiSans", 8, QFont::Normal);   // 字体信息
    QVector<QGraphicsRectItem *> legendRect;                // 存储指针用于清除

    // graphicScene
    QGraphicsScene* metroGraphicsScene;

    // 地铁信息
    metroMapData* metroMap;

    // 所有的可视化边
    QVector<metroGraphicsEdgeItem *> metroGraphicsEdges;

    // 所有的可视化站点
    QVector<metroGraphicsStationItem *> metroGraphicsStations;

    // 是否在选择站点
    bool isPickingStart = false, isPickingEnd = false;

    // 是否在创建可视化新站点
    bool isCreatingNewStation = false;
    bool isDragging = false;    // 如果单击之后在松开前拖动了 就不是创建节点而是拖画布

    // 即将建立的新站点信息
    metroStationData* newStation;

    // 选择的待寻路的站点名称
    QString start, end;

    // 寻路模式
    int mode = 0;

protected:
    void mousePressEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    void wheelEvent (QWheelEvent * e) Q_DECL_OVERRIDE;

public:
    metroGraphicsView(qint16 _leftTopx = 0, qint16 _leftTopy = 0, qint16 _width = 780, qint16 _height = 640, QWidget* parent = nullptr);
    ~metroGraphicsView() Q_DECL_OVERRIDE;
    // 加载配置信息
    void loadConfig();
    // 可视化寻路
    void visualizeFindPath(QString start, QString end, FIND_PATH_METHOD method);
    // 处理开始选择站点
    void handlePickStart();
    void handlePickEnd();
    // 处理开始寻路信号
    void handleFindPath();
    // 处理开始恢复可视化
    void handleRevert();
    // 处理模式变化
    void handleModeChanged(int _mode);
    // 处理站点信息变化
    void handleStartChanged(const QString& name);
    void handleEndChanged(const QString& name);
    // 处理新站点创立按钮点击
    void handleNewStationCreate();
    // 处理新连线创立按钮点击
    void handleNewEdgeCreate();
    // 处理新站点信息提交
    void handleNewStationSubmmited(metroStationData* newStation);
    // 处理新连线信息提交
    void handleNewEdgeSubmmited(QString name1, QString name2, QString line);
    // 处理新线路创建
    void handleNewLineCreated(QString name, QColor color);
    // 处理配置保存
    void handleSaveConfig();

private:
    // 绘制图例
    void drawLegend();
    // 清除图例
    void clearLegend();
    // 处理站点点击
    void handleStationClicked(QString name);

signals:
    void startPicked(QString name);
    void endPicked(QString name);
    void pathGuidanceGenerated(QString content);
    void errorMessage(QString msg);
    void cueMessage(QString msg);
};


// 地铁的可视化站点
class metroGraphicsStationItem: public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

    friend class metroGraphicsView;

private:
    // 站点显示形态
    STATION_TYPE type;
    const qint16 radius = 7;
    QColor showColor;

    // 地图位置与实际位置
    QPoint graphPosition;
    QPointF realPosition;

    // 编号与名称
    qint16 id;  // 站点的编号（唯一）
    QString name;

    // NameTag
    QGraphicsSimpleTextItem* nameTag;
    QFont nameFont = QFont("MiSans", 8, QFont::Normal);
    NAMETAG_POSITION nametagPosition;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *e) Q_DECL_OVERRIDE;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) Q_DECL_OVERRIDE;
    QRectF boundingRect() const Q_DECL_OVERRIDE;

public:
    metroGraphicsStationItem(qint16 _id, QString _name, QPoint _graphPst, QPointF _realPst, STATION_TYPE type, NAMETAG_POSITION _tagPst, QGraphicsItem* parent = nullptr);
    QPoint getGraphPosition() const;
    // 点的变灰色与恢复（显示换乘路线用）
    void colorTurnGray();
    void colorRevert();

signals:
    void clicked(QString name);
};


// 地铁的可视化边
class metroGraphicsEdgeItem: public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

    friend class metroGraphicsView;

    // 边的起止信息
    QPoint start, end;
    QVector<QPoint> via;

    // 颜色
    QColor lineColor;   // 对应的线路颜色
    QColor showColor;   // 显示的颜色

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) Q_DECL_OVERRIDE;
    QRectF boundingRect() const Q_DECL_OVERRIDE;

public:
    metroGraphicsEdgeItem(QPoint _start, QPoint _end, QColor _lineColor, QVector<QPoint> _via, QGraphicsItem* parent = nullptr);
    // 边的变灰色与恢复（显示换乘路线用）
    void colorTurnGray();
    void colorRevert();
};



#endif // GRAPHVIEW_H
