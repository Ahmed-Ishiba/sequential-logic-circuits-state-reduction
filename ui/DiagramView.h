#pragma once
#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QPointF>
#include "../engine/StateTypes.h"
#include <vector>
#include <map>

class StateNode;
class TransitionArrow;

class DiagramScene : public QGraphicsScene {
    Q_OBJECT
public:
    explicit DiagramScene(QObject* parent = nullptr);


    void setMachine(const std::vector<State>& states, const QString& title = {});

    void clear();

private:
    void doLayout(const std::vector<State>& states);
    void addArrow(char from, char to, int inputBit, int outputVal,
                  const std::map<char, StateNode*>& nodes);

    static QPointF circlePosition(int index, int total, double radius, QPointF center);
};

class DiagramView : public QWidget {
    Q_OBJECT
public:
    explicit DiagramView(const QString& title, QWidget* parent = nullptr);

    void setMachine(const std::vector<State>& states);
    void clear();

private:
    QGraphicsView* m_view  = nullptr;
    DiagramScene*  m_scene = nullptr;
    QString        m_title;
};
