#include "ResultWidget.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QFont>
#include <QGroupBox>

ResultWidget::ResultWidget(QWidget* parent) : QWidget(parent) {
    auto* root = new QVBoxLayout(this);
    root->setSpacing(12);
    root->setContentsMargins(0, 0, 0, 0);

    auto* classBox = new QGroupBox("Equivalence Classes");
    auto* classBoxLayout = new QVBoxLayout(classBox);
    m_classLabel = new QLabel("Run the minimizer to see results.");
    m_classLabel->setWordWrap(true);
    m_classLabel->setStyleSheet("font-family: monospace; font-size: 13px;");
    classBoxLayout->addWidget(m_classLabel);
    root->addWidget(classBox);

    auto* tableBox = new QGroupBox("Reduced State Table");
    auto* tableBoxLayout = new QVBoxLayout(tableBox);

    m_table = new QTableWidget();
    m_table->setColumnCount(4);
    QStringList headers;
    headers << "Present State" << "Next State (x=0)" << "Next State (x=1)" << "Output";
    m_table->setHorizontalHeaderLabels(headers);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->verticalHeader()->setVisible(false);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setAlternatingRowColors(true);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);

    tableBoxLayout->addWidget(m_table);
    root->addWidget(tableBox);

    root->addStretch();
}

void ResultWidget::setResult(const ReducedMachine& rm, int originalCount) {
    buildClassLabel(rm, originalCount);
    buildTable(rm);
}

void ResultWidget::clear() {
    m_classLabel->setText("Run the minimizer to see results.");
    m_table->setRowCount(0);
}

void ResultWidget::buildClassLabel(const ReducedMachine& rm, int originalCount) {
    int reducedCount = (int)rm.classes.size();
    QString text;
    text += QString("<b>Original states:</b> %1 &nbsp;&nbsp; "
                    "<b>Reduced states:</b> %2<br><br>")
                .arg(originalCount).arg(reducedCount);

    for (auto& ec : rm.classes) {
        QString members;
        for (char c : ec.members)
            members += QString(c) + " ";
        members = members.trimmed();
        text += QString("<b>%1</b> &larr; { %2 }<br>").arg(ec.newLabel).arg(members);
    }

    m_classLabel->setText(text);
}

void ResultWidget::buildTable(const ReducedMachine& rm) {
    int n = (int)rm.classes.size();
    m_table->setRowCount(n);

    QFont bold = m_table->font();
    bold.setBold(true);

    for (int i = 0; i < n; i++) {
        const auto& ec  = rm.classes[i];
        const auto& rep = ec.representative;

        auto makeItem = [&](const QString& text, bool isBold = false) {
            auto* item = new QTableWidgetItem(text);
            item->setTextAlignment(Qt::AlignCenter);
            item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            if (isBold) item->setFont(bold);
            return item;
        };

        m_table->setItem(i, 0, makeItem(QString(ec.newLabel), true));
        m_table->setItem(i, 1, makeItem(QString(rep.ns[0])));
        m_table->setItem(i, 2, makeItem(QString(rep.ns[1])));
        m_table->setItem(i, 3, makeItem(QString::number(rep.output)));
    }
}
