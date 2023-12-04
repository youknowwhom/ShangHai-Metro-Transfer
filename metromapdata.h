#ifndef METROMAP_H
#define METROMAP_H

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <QJsonValue>
#include <QFile>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QMouseEvent>
#include <QVector>
#include <QSet>
#include <QQueue>
#include <queue>
#include <QFont>
#include <QColor>
#include <QDebug>
#include "stationtype.h"

// 存储整个地铁信息
class metroMapData;
// 存储地铁站信息
class metroStationData;
// 存储地铁边信息
class metroEdgeData;
// 存储地铁线路信息
class metroLineData;

// 前向声明可视化画布类
class metroGraphicsView;
// 前向声明可视化站点类
class metroGraphicsStationItem;
// 前向声明可视化边类
class metroGraphicsEdgeItem;


// 存储地铁网络结构的类
class metroMapData
{
private:
    // 地铁线路信息
    QMap<QString, metroLineData *> metroLines;
    QVector<metroLineData *> metroLinesVec;     // 按线路顺序
    // 地铁站点信息
    QMap<QString, metroStationData *> metroStations;

    // 可视化地图的所有站点
    QVector<metroGraphicsStationItem *> metroGraphicStations;
    // 可视化地图的所有边
    QVector<metroGraphicsEdgeItem *> metroGraphicEdges;

    // 暂存的寻路路径
    QQueue<metroEdgeData *> path;

    // 配置信息
    const QString configPath = QString("./metroInfo.json");

    static int getDistance(QPointF start, QPointF end);

public:
    metroMapData();
    ~metroMapData();

    // 读写配置文件(成功true，失败false)
    bool readConfig();
    bool writeConfig();

    // 获取Map基本信息
    QVector<metroLineData *> getMetroLines() const;
    QVector<metroGraphicsStationItem *> getMetroGraphicStations() const;
    QVector<metroGraphicsEdgeItem *> getMetroGraphicEdges() const;
    qint16 getStationNum();
    const QColor& getLineColor(QString name);
    QStringList getLinesName();
    bool isStationExisted(QString name);
    bool isLineExisted(QString name);

    // 地铁换乘寻路
    const QQueue<metroEdgeData *>& findPath(QString start, QString end, int (* getDistance)(const QQueue<metroEdgeData *>&, metroEdgeData *));
    static int getDistanceByCount(const QQueue<metroEdgeData *> & queue, metroEdgeData * edge);
    static int getDistanceByDistance(const QQueue<metroEdgeData *> & queue, metroEdgeData * edge);
    static int getDistanceByTransfer(const QQueue<metroEdgeData *> & queue, metroEdgeData * edge);

    // 用户增加新站点、线路
    metroGraphicsStationItem* addNewStation(metroStationData* newStation);
    metroGraphicsEdgeItem * addNewEdges(QString name1, QString name2, QString line);
    void addNewLine(QString name, QColor color);
};


// 存储地铁站信息的类
class metroStationData
{
    friend class metroMapData;
    friend class metroEdgeData;

    // 编号（唯一）与站点名称
    qint16 id;
    QString name;

    // 实际位置（经纬度）与地图位置
    QPointF realPosition;
    QPoint graphPosition;

    // 站点形态与标签位置
    STATION_TYPE type;
    NAMETAG_POSITION nametagPosition;

    // 与该站点相连的边
    QVector<metroEdgeData *> edges;

    // 对应的可视化站点
    metroGraphicsStationItem* graphicItem = nullptr;


public:
    metroStationData(qint16 _id, QString _name, QPoint _graphPos, QPointF _realPos, STATION_TYPE _type, NAMETAG_POSITION _tagPos);
    metroStationData(QString _name, QPointF _realPos, STATION_TYPE _type, NAMETAG_POSITION _tagPos);
    metroGraphicsStationItem* getGraphicItem() const;

    // 设置基础信息
    void setId(qint16 id);
    void setGraphPosition(const QPoint& position);

    // 获取基础信息
    const QString& getName() const;
    const QPoint& getGraphPosition() const;
};


// 存储地铁边结构的类
class metroEdgeData
{
    friend metroMapData;

    // 起止信息
    QString startName, endName;
    metroStationData* start, * end;

    // 线路信息
    QString line;
    QColor lineColor;

    // 经过的点
    QVector<QPoint> via;

    // 对应的可视化边
    metroGraphicsEdgeItem* graphicItem;

public:
    metroEdgeData(QString _startName, QString _endName, QString _line, QColor _color, QVector<QPoint> _via);
    void fillInformation(const QMap<QString, metroStationData *>& metroStations);   // 填充结点信息

    // 获取基本信息
    metroGraphicsEdgeItem* getGraphicItem() const;
    metroStationData* getStart() const;
    metroStationData* getEnd() const;
    const QString& getLine() const;
};


// 地铁线路信息
class metroLineData
{
    friend class metroMapData;

    QString name;
    QColor color;

public:
    metroLineData(QString _name, QColor _color);
    const QColor& getColor() const;
    const QString& getName() const;
};

#endif // METROMAP_H
