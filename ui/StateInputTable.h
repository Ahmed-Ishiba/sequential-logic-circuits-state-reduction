#pragma once
#include <QWidget>
#include "../engine/StateTypes.h"
#include <vector>

class QTableWidget;
class QLabel;

class StateInputTable : public QWidget {
    Q_OBJECT

public:
    explicit StateInputTable(int n, QWidget* parent = nullptr);

    std::vector<State> getStates() const;

    void resize(int n);

signals:
    void dataChanged();

private:
    void setupTable(int n);
    bool validateCell(int row, int col, QString& errMsg) const;

    QTableWidget* m_table = nullptr;
    int           m_n     = 0;
};
