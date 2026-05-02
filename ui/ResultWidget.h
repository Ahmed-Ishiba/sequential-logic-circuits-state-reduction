#pragma once
#include <QWidget>
#include "../engine/StateTypes.h"

class QTableWidget;
class QLabel;

class ResultWidget : public QWidget {
    Q_OBJECT

public:
    explicit ResultWidget(QWidget* parent = nullptr);

    void setResult(const ReducedMachine& rm, int originalCount);
    void clear();

private:
    void buildClassLabel(const ReducedMachine& rm, int originalCount);
    void buildTable(const ReducedMachine& rm);

    QLabel*       m_classLabel  = nullptr;
    QTableWidget* m_table       = nullptr;
};
