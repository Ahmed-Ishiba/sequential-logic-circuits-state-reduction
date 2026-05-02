#include "MainWindow.h"
#include "StateInputTable.h"
#include "ResultWidget.h"
#include "DiagramView.h"
#include "../engine/ImplicationTable.h"

#include <QTabWidget>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolBar>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QStatusBar>
#include <QSplitter>
#include <QGroupBox>
#include <QMessageBox>
#include <QAction>

MainWindow::MainWindow(int stateCount, QWidget* parent)
    : QMainWindow(parent), m_stateCount(stateCount)
{
    setWindowTitle("State Machine Minimizer");
    setMinimumSize(900, 640);
    resize(1100, 720);

    buildToolBar();

    m_tabs = new QTabWidget(this);
    m_tabs->setTabPosition(QTabWidget::North);
    setCentralWidget(m_tabs);

    buildInputTab();
    buildResultTab();
    buildDiagramTab();

    m_statusLabel = new QLabel("Ready — enter state data then click Run.");
    statusBar()->addWidget(m_statusLabel);
}

void MainWindow::buildToolBar() {
    auto* tb = addToolBar("Main");
    tb->setMovable(false);
    tb->setFloatable(false);
    tb->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    tb->addWidget(new QLabel("  States: "));
    m_stateSpin = new QSpinBox();
    m_stateSpin->setRange(2, 26);
    m_stateSpin->setValue(m_stateCount);
    m_stateSpin->setFixedWidth(60);
    m_stateSpin->setToolTip("Change number of states (preserves existing data)");
    tb->addWidget(m_stateSpin);
    connect(m_stateSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::onStateCountChanged);

    tb->addSeparator();

    m_runBtn = new QPushButton("▶  Run Minimizer");
    m_runBtn->setFixedHeight(28);
    m_runBtn->setToolTip("Run the implication table algorithm and show results");
    tb->addWidget(m_runBtn);
    connect(m_runBtn, &QPushButton::clicked, this, &MainWindow::onRun);

    tb->addSeparator();

    auto* resetBtn = new QPushButton("↺  Reset");
    resetBtn->setFixedHeight(28);
    resetBtn->setToolTip("Clear all input and results");
    tb->addWidget(resetBtn);
    connect(resetBtn, &QPushButton::clicked, this, &MainWindow::onReset);

    tb->addSeparator();

    auto* helpLbl = new QLabel(
        "  Enter next-state labels (e.g. A, B) and integer outputs. "
        "Labels are auto-assigned A, B, C…"
    );
    helpLbl->setStyleSheet("color: palette(mid); font-size: 11px;");
    tb->addWidget(helpLbl);
}

void MainWindow::buildInputTab() {
    auto* page   = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(8);

    auto* hdr = new QLabel(
        "<b>Original State Table</b> — "
        "Enter the next states and output for each present state."
    );
    hdr->setWordWrap(true);
    layout->addWidget(hdr);

    m_inputTable = new StateInputTable(m_stateCount, page);
    layout->addWidget(m_inputTable);

    m_tabs->addTab(page, "① Input");
}

void MainWindow::buildResultTab() {
    auto* page   = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(12, 12, 12, 12);

    auto* hdr = new QLabel(
        "<b>Minimization Results</b> — "
        "Equivalence classes and the reduced state table appear here after running."
    );
    hdr->setWordWrap(true);
    layout->addWidget(hdr);

    m_resultWidget = new ResultWidget(page);
    layout->addWidget(m_resultWidget);

    m_tabs->addTab(page, "② Results");
}

void MainWindow::buildDiagramTab() {
    auto* page    = new QWidget();
    auto* layout  = new QVBoxLayout(page);
    layout->setContentsMargins(4, 4, 4, 4);

    auto* splitter = new QSplitter(Qt::Horizontal, page);

    m_origDiagram    = new DiagramView("Original Machine", splitter);
    m_reducedDiagram = new DiagramView("Reduced Machine",  splitter);

    splitter->addWidget(m_origDiagram);
    splitter->addWidget(m_reducedDiagram);
    splitter->setSizes({500, 500});

    layout->addWidget(splitter);
    m_tabs->addTab(page, "③ Diagrams");
}

void MainWindow::onStateCountChanged(int n) {
    m_stateCount = n;
    if (m_inputTable) m_inputTable->resize(n);
    m_statusLabel->setText(QString("State count changed to %1 — data preserved where possible.").arg(n));
}

void MainWindow::onRun() {

    std::vector<State> states = m_inputTable->getStates();
    if (states.empty()) return;

    ImplicationTable table((int)states.size());
    table.init(states);
    table.reduce(states);


    ReducedMachine rm = table.buildReduced(states);

    m_resultWidget->setResult(rm, (int)states.size());

    std::vector<State> reducedStates;
    for (auto& ec : rm.classes)
        reducedStates.push_back(ec.representative);

    m_origDiagram->setMachine(states);
    m_reducedDiagram->setMachine(reducedStates);

    m_tabs->setCurrentIndex(1);
    int saved = (int)states.size() - (int)rm.classes.size();
    if (saved > 0)
        m_statusLabel->setText(
            QString("Done — reduced from %1 to %2 states (%3 state(s) eliminated).")
                .arg(states.size()).arg(rm.classes.size()).arg(saved));
    else
        m_statusLabel->setText(
            QString("Done — machine is already minimal (%1 states, no reduction possible).")
                .arg(states.size()));
}

void MainWindow::onReset() {
    auto reply = QMessageBox::question(this, "Reset",
        "Clear all input data and results?",
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);
    if (reply != QMessageBox::Yes) return;

    m_inputTable->resize(m_stateCount);
    m_resultWidget->clear();
    m_origDiagram->clear();
    m_reducedDiagram->clear();
    m_tabs->setCurrentIndex(0);
    m_statusLabel->setText("Reset — enter state data then click Run.");
}
