#include "metromapdata.h"
#include "graphview.h"

/* 地铁总信息:metroMapData */

metroMapData::metroMapData()
{
}

// 读入配置文件
bool metroMapData::readConfig(){
    QFile file(configPath);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qDebug() << "无法打开配置文件！";
        return false;
    }

    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    QString str = stream.readAll();

    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(str.toUtf8(), &parseError);
    if(parseError.error != QJsonParseError::NoError && !doc.isNull()){
        qDebug() << "解析配置文件失败！";
        return false;
    }

    // 解析线路信息
    QJsonObject jsonRoot = doc.object();
    QJsonArray lines = jsonRoot["lines"].toArray();
    for(int i = 0; i < lines.count(); i++){
        QJsonObject line = lines[i].toObject();
        QString name = line["name"].toString();
        int r = line["color"].toArray()[0].toInt();
        int g = line["color"].toArray()[1].toInt();
        int b = line["color"].toArray()[2].toInt();
        metroLineData* newline = new metroLineData(name, QColor(r, g, b));

        // 记录新线路
        metroLines.insert(name, newline);
        metroLinesVec.push_back(newline);
    }

    // 解析站点信息
    QJsonArray stations = jsonRoot["stations"].toArray();
    qint16 id = 0;
    // 所有的地铁边信息
    QVector<metroEdgeData *> metroEdges;

    for(int i = 0; i < stations.count(); i++){
        QJsonObject station = stations[i].toObject();
        QString name = station["name"].toString();
        QPoint graph_position(station["graph-position"].toArray()[0].toInt(), station["graph-position"].toArray()[1].toInt());
        QPointF real_position(station["real-position"].toArray()[0].toDouble(), station["real-position"].toArray()[1].toDouble());

        // 该站点的画布信息无效 跳过
        if(graph_position==QPointF(0,0))
            continue;
        // 每个站点有唯一编号
        else
            id++;

        STATION_TYPE type;
        if(station["type"].toString() == "vertical")
            type = STATION_TYPE::VERTICAL;
        else if(station["type"].toString() == "horizontal")
            type = STATION_TYPE::HORIZONTAL;
        else if(station["type"].toString() == "diagonal")
            type = STATION_TYPE::DIAGONAL;
        else if(station["type"].toString() == "backdiagonal")
            type = STATION_TYPE::B_DIAGONAL;
        else
            type = STATION_TYPE::NORMAL;

        NAMETAG_POSITION tag;
        if(station["tag"].toString() == "left")
            tag = NAMETAG_POSITION::LEFT;
        else if(station["tag"].toString() == "right")
            tag = NAMETAG_POSITION::RIGHT;
        else if(station["tag"].toString() == "bottom")
            tag = NAMETAG_POSITION::BOTTOM;
        else if(station["tag"].toString() == "topleft")
            tag = NAMETAG_POSITION::TOP_LEFT;
        else if(station["tag"].toString() == "topright")
            tag = NAMETAG_POSITION::TOP_RIGHT;
        else if(station["tag"].toString() == "bottomleft")
            tag = NAMETAG_POSITION::BOTTOM_LEFT;
        else if(station["tag"].toString() == "bottomright")
            tag = NAMETAG_POSITION::BOTTOM_RIGHT;
        else
            tag = NAMETAG_POSITION::TOP;

        metroStationData* newstation = new metroStationData(id, name, graph_position, real_position, type, tag);

        // 建立与其他站点的连边
        QJsonArray edges = station["edges"].toArray();
        for(int i = 0; i < edges.count(); i++){
            QJsonObject edge = edges[i].toObject();
            QString to = edge["to"].toString();
            QString line = edge["line"].toString();

            // 装填经过的点位置
            QVector<QPoint> newvias;
            QJsonArray vias = edge["via"].toArray();
            for(int i = 0; i < vias.count(); i++){
                QJsonArray coordinate = vias[i].toArray();
                QPoint point(coordinate[0].toInt(), coordinate[1].toInt());
                newvias.append(point);
            }
            metroEdgeData* newedge = new metroEdgeData(name, to, line, metroLines[line]->color, newvias);

            // 记录到该站点
            newstation->edges.append(newedge);
            // 全局记录边信息
            metroEdges.append(newedge);
        }
        // 全局记录该站点信息
        metroStations[name] = newstation;
        metroGraphicStations.push_back(newstation->getGraphicItem());
    }

    // 装填边的信息 并保存可视化边
    for(auto& edge: metroEdges){
        edge->fillInformation(metroStations);
        // 重边只画一条
        if(edge->start->id < edge->end->id)
            metroGraphicEdges.append(edge->getGraphicItem());
    }

    // 刚才申请了所有可视化边，现在把重边的相反边也赋值
    for(auto& edge: metroEdges){
        if(!edge->graphicItem){
            for(auto& reverse_edge: edge->end->edges){
                if(reverse_edge->end == edge->start && reverse_edge->line == edge->line){
                    edge->graphicItem = reverse_edge->graphicItem;
                    break;
                }
            }
        }
    }

    return true;
}

bool metroMapData::writeConfig()
{
    QJsonObject root;
    QJsonArray lines;
    QJsonArray stations;

    for(auto& line: metroLinesVec){
        QJsonObject lineObj;
        QJsonValue name = line->name;
        QJsonArray color;
        color.append(line->color.red());
        color.append(line->color.green());
        color.append(line->color.blue());
        lineObj["name"] = name;
        lineObj["color"] = color;
        lines.append(lineObj);
    }

    // 先把map里无序的station按id排序
    QVector<metroStationData *> stationVec;
    stationVec.resize(this->getStationNum());
    for(auto& station: metroStations)
        stationVec[station->id - 1] = station;

    // 按顺序写入
    for(auto& station:stationVec){
        QJsonObject stationObj;

        QJsonValue name = station->name;

        QJsonArray graphPst;
        graphPst.append(station->graphPosition.x());
        graphPst.append(station->graphPosition.y());

        QJsonArray realPst;
        realPst.append(station->realPosition.x());
        realPst.append(station->realPosition.y());

        QJsonValue type, tag;

        switch (station->type) {
            case STATION_TYPE::VERTICAL:
                type="vertical";
                break;
            case STATION_TYPE::HORIZONTAL:
                type = "horizontal";
                break;
            case STATION_TYPE::DIAGONAL:
                type = "diagonal";
                break;
            case STATION_TYPE::B_DIAGONAL:
                type = "backdiagonal";
                break;
            default:
                type = "normal";
                break;
        }

        switch(station->nametagPosition){
            case NAMETAG_POSITION::LEFT:
                tag = "left";
                break;
            case NAMETAG_POSITION::RIGHT:
                tag = "right";
                break;
            case NAMETAG_POSITION::BOTTOM:
                tag = "bottom";
                break;
            case NAMETAG_POSITION::TOP_LEFT:
                tag = "topleft";
                break;
            case NAMETAG_POSITION::TOP_RIGHT:
                tag = "topright";
                break;
            case NAMETAG_POSITION::BOTTOM_LEFT:
                tag = "bottomleft";
                break;
            case NAMETAG_POSITION::BOTTOM_RIGHT:
                tag = "bottomright";
                break;
            default:
                tag = "top";
                break;
        }

        QJsonArray edges;
        for(auto& edge: station->edges){
            QJsonObject edgeObj;
            QJsonValue to, line;
            QJsonArray vias;
            to = edge->endName;
            line = edge->line;
            for(auto& via: edge->via){
                QJsonArray position;
                position.append(via.x());
                position.append(via.y());
                vias.append(position);
            }
            edgeObj["to"] = to;
            edgeObj["line"] = line;
            edgeObj["via"] = vias;

            edges.append(edgeObj);
        }

        stationObj["name"] = name;
        stationObj["type"] = type;
        stationObj["tag"] = tag;
        stationObj["graph-position"] = graphPst;
        stationObj["real-position"] = realPst;
        stationObj["edges"] = edges;

        stations.append(stationObj);
    }

    root["lines"] = lines;
    root["stations"] = stations;

    // 创建一个JSON文档并将JSON对象添加到其中
    QJsonDocument configDoc(root);

    // 打开文件以写入模式
    QFile configFile(configPath);
    if (!configFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << "无法打开文件：" << configPath;
        return false;
    }

    // 将JSON文档写入文件
    QTextStream out(&configFile);
    out.setCodec("UTF-8");
    out << configDoc.toJson();

    // 关闭文件
    configFile.close();

    return true;
}

QVector<metroLineData *> metroMapData::getMetroLines() const
{
    return metroLinesVec;
}

QVector<metroGraphicsStationItem *> metroMapData::getMetroGraphicStations() const
{
    return metroGraphicStations;
}

QVector<metroGraphicsEdgeItem *> metroMapData::getMetroGraphicEdges() const
{
    return metroGraphicEdges;
}

const QQueue<metroEdgeData *>& metroMapData::findPath(QString start, QString end, int (* getDistance)(const QQueue<metroEdgeData *>&, metroEdgeData *))
{
    metroStationData* startStation;

    if(!metroStations.contains(start) || !metroStations.contains(end)){
        path.clear();
        return path;
    }

    startStation = metroStations[start];

    QMap<QString, int> distance;
    QSet<QString> visit;

    struct element{
        metroStationData * station;
        int distance;
        QQueue<metroEdgeData *> path;
        bool operator < (const element& that) const
        {
           return distance > that.distance;
        }
    };

    std::priority_queue<element> queue;
    queue.push({ startStation, 0, QQueue<metroEdgeData *>()});

    path.clear();

    while(!queue.empty()){
        element cur = queue.top();
        queue.pop();

        if(visit.contains(cur.station->name) && (!distance.contains(cur.station->name) || cur.distance > distance[cur.station->name]))
            continue;

        // 由于距离相等的情况不直接跳过 要防止两个站点来回横跳的情况（出现在换乘次数优先时）
        bool continued = false;
        for(auto& edge: cur.path){
            if(edge->start == cur.station){
                continued = true;
                break;
            }
        }
        if(continued)
            continue;

        visit.insert(cur.station->name);

        // 到达目的地 进行记录
        if(cur.station->name == end){
            if(path.empty() || cur.distance < distance[end])
                this->path = cur.path;
            else if(cur.distance == distance[end]){
                // 如果计算的距离相同，最终还是以站点数小的优先
                if(cur.path.count() < path.count())
                    this->path = cur.path;
            }
        }

        // 遍历当前结点的所有边
        for(auto& edge: cur.station->edges){
            if(!distance.contains(edge->end->name) || cur.distance + getDistance(cur.path, edge) <= distance.value(edge->end->name)){
                distance[edge->end->name] = cur.distance + getDistance(cur.path, edge);
                QQueue<metroEdgeData *> newpath = cur.path;
                newpath.push_back(edge);
                queue.push({edge->end, cur.distance + getDistance(cur.path, edge), newpath});
            }
        }
    }
    return path;
}


// 根据经纬度求算距离
int metroMapData::getDistance(QPointF start, QPointF end)
{
    const int EARTH_RADIUS = 6378137;

    // 纬度
    double lat1 = qDegreesToRadians(start.y());
    double lat2 = qDegreesToRadians(end.y());
    // 经度
    double lng1 = qDegreesToRadians(start.x());
    double lng2 = qDegreesToRadians(end.x());
    // 纬度之差
    double a = lat1 - lat2;
    // 经度之差
    double b = lng1 - lng2;

    // 计算两点距离的公式
    double s = 2 * qAsin(qSqrt(qPow(qSin(a / 2), 2) +
            qCos(lat1) * qCos(lat2) * qPow(qSin(b / 2), 2)));

    // 弧长乘地球半径, 返回单位: 米
    s =  s * EARTH_RADIUS;

    return int(s);
}

int metroMapData::getDistanceByCount(const QQueue<metroEdgeData *> & queue, metroEdgeData * edge)
{
    // 边权都是1（换乘代价为3，避免盲目换乘）
    if(!queue.isEmpty() && queue.back()->line != edge->line)
        return 3;
    else
        return 1;
}

int metroMapData::getDistanceByDistance(const QQueue<metroEdgeData *>& queue, metroEdgeData * edge)
{
    int avgStationDistance = 2500;

    int retDistance = getDistance(edge->start->realPosition, edge->end->realPosition);

    // 若是换乘站，算上三次平均站间距
    if(!queue.isEmpty() && queue.back()->line != edge->line)
        retDistance += avgStationDistance * 3;

    return retDistance;
}


int metroMapData::getDistanceByTransfer(const QQueue<metroEdgeData *>& queue, metroEdgeData * edge)
{
    // 只考虑换乘次数
    // 因为visit数组略作改动（为了适应换乘的计算方式），略微记录路径耗散防止两点来回横跳的情况
    if(!queue.isEmpty() && queue.back()->line != edge->line)
        return 1;
    else
        return 0;
}


bool metroMapData::isStationExisted(QString name)
{
    return metroStations.count(name);
}


bool metroMapData::isLineExisted(QString name)
{
    return metroLines.count(name);
}

qint16 metroMapData::getStationNum()
{
    return qint16(metroStations.count());
}

const QColor& metroMapData::getLineColor(QString name)
{
    return metroLines[name]->color;
}

QStringList metroMapData::getLinesName()
{
    QStringList list;
    for(auto& line: metroLinesVec)
        list << line->name;
    return list;
}

metroGraphicsStationItem* metroMapData::addNewStation(metroStationData *newStation)
{
    // 像地铁站信息map中进行记录
    metroStations[newStation->name] = newStation;
    return newStation->graphicItem;
}

metroGraphicsEdgeItem * metroMapData::addNewEdges(QString name1, QString name2, QString line)
{
    metroStationData *start, *end;
    // 先保证start.id < endid方便处理
    if(metroStations[name1]->id < metroStations[name2]->id){
        start = metroStations[name1];
        end = metroStations[name2];
    }
    else{
        start = metroStations[name2];
        end = metroStations[name1];
    }

    // 建立edge对象
    metroEdgeData* line1 = new metroEdgeData(start->name, end->name, line, metroLines[line]->color,  QVector<QPoint>());
    metroEdgeData* line2 = new metroEdgeData(end->name, start->name, line, metroLines[line]->color,  QVector<QPoint>());

    // 填充顶点位置信息并创立可视化边
    line1->fillInformation(metroStations);
    line2->fillInformation(metroStations);
    line2->graphicItem = line1->graphicItem;

    // 将边记录到对应的顶点里
    start->edges.push_back(line1);
    end->edges.push_back(line2);

    return line1->graphicItem;
}

void metroMapData::addNewLine(QString name, QColor color)
{
    // 记录新线路
    metroLineData* newLine = new metroLineData(name, color);
    metroLines[name] = newLine;
    metroLinesVec.append(newLine);
}


/* 地铁站点信息:metroStationData */
metroStationData::metroStationData(qint16 _id, QString _name, QPoint _graphPos, QPointF _realPos, STATION_TYPE _type, NAMETAG_POSITION _tagPos):
    id(_id),
    name(_name),
    realPosition(_realPos),
    graphPosition(_graphPos),
    type(_type),
    nametagPosition(_tagPos)
{
    graphicItem = new metroGraphicsStationItem(id, name, graphPosition, realPosition, type, nametagPosition);
}

// 用于新建站点的构造函数 暂时缺少id和graphPosition信息
metroStationData::metroStationData(QString _name, QPointF _realPos, STATION_TYPE _type, NAMETAG_POSITION _tagPos):
    name(_name),
    realPosition(_realPos),
    type(_type),
    nametagPosition(_tagPos)
{
}

const QPoint& metroStationData::getGraphPosition() const
{
    return graphPosition;
}

const QString& metroStationData::getName() const
{
    return name;
}

metroGraphicsStationItem* metroStationData::getGraphicItem() const
{
    return graphicItem;
}

void metroStationData::setId(qint16 _id)
{
    id = _id;
}

void metroStationData::setGraphPosition(const QPoint& _position)
{
    graphPosition = _position;
    // 得到了位置信息 可以创建可视化item对象
    if(!graphicItem)
        graphicItem = new metroGraphicsStationItem(id, name, graphPosition, realPosition, type, nametagPosition);
}

// 析构函数 释放空间
metroMapData::~metroMapData(){
    for(auto& station: metroStations){
        // 释放其中的边
        for(auto& edge: station->edges)
            delete edge;
        // 释放站点
        delete station;
    }

    for(auto& line: metroLines)
        delete line;
}

/* 地铁边信息:metroEdgeData */
metroEdgeData::metroEdgeData(QString _startName, QString _endName, QString _line, QColor _color, QVector<QPoint> _via):
    startName(_startName),
    endName(_endName),
    line(_line),
    lineColor(_color),
    via(_via)
{
}

void metroEdgeData::fillInformation(const QMap<QString, metroStationData *>& metroStations)
{
    start = metroStations[startName];
    end = metroStations[endName];
    if(start->id < end->id)
        graphicItem = new metroGraphicsEdgeItem(start->getGraphPosition(), end->getGraphPosition(), lineColor, via);
    // 等待之后再装填 重边在地图上只对应一个可视化边
    else
        graphicItem = nullptr;
}

metroGraphicsEdgeItem* metroEdgeData::getGraphicItem() const
{
    return graphicItem;
}

metroStationData* metroEdgeData::getStart() const
{
    return start;
}

metroStationData* metroEdgeData::getEnd() const
{
    return end;
}

const QString& metroEdgeData::getLine() const
{
    return line;
}


/* 地铁线路:metroLineData */
metroLineData::metroLineData(QString _name, QColor _color):
    name(_name),
    color(_color)
{
}

const QColor& metroLineData::getColor() const
{
    return color;
}

const QString& metroLineData::getName() const
{
    return name;
}

