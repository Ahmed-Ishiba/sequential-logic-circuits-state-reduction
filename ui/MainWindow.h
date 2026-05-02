#pragma once
#include <QMainWindow>
#include "../engine/StateTypes.h"
#include <vector>

class QTabWidget;
class QSpinBox;
class QPushButton;
class QLabel;
class StateInputTable;
class ResultWidget;
class DiagramView;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(int stateCount, QWidget* parent = nullptr);

private slots:
    void onRun();
    void onReset();
    void onStateCountChanged(int n);

private:
    void buildInputTab();
    void buildResultTab();
    void buildDiagramTab();
    void buildToolBar();

    QTabWidget*      m_tabs           = nullptr;
    StateInputTable* m_inputTable     = nullptr;
    ResultWidget*    m_resultWidget   = nullptr;
    DiagramView*     m_origDiagram    = nullptr;
    DiagramView*     m_reducedDiagram = nullptr;
    QSpinBox*        m_stateSpin      = nullptr;
    QPushButton*     m_runBtn         = nullptr;
    QLabel*          m_statusLabel    = nullptr;

    int              m_stateCount     = 4;
};
