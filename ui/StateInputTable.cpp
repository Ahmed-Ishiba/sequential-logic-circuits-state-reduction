#include "StateInputTable.h"
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QFont>

StateInputTable::StateInputTable(int n, QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    auto* hint = new QLabel(
        "Fill in each row. <b>NS(x=0)</b> and <b>NS(x=1)</b> must be valid state labels. "
        "<b>Output</b> must be 0 or 1."
    );
    hint->setWordWrap(true);
    hint->setStyleSheet("color: white; font-size: 12px; margin-bottom: 4px;");
    layout->addWidget(hint);

    m_table = new QTableWidget(this);
    layout->addWidget(m_table);

    setupTable(n);
}

void StateInputTable::setupTable(int n) {
    m_n = n;

    m_table->clear();
    m_table->setRowCount(n);
    m_table->setColumnCount(4);

    QStringList headers;
    headers << "Present State" << "Next State (x=0)" << "Next State (x=1)" << "Output";
    m_table->setHorizontalHeaderLabels(headers);

    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->verticalHeader()->setVisible(false);
    m_table->setAlternatingRowColors(true);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);

    QFont bold = m_table->font();
    bold.setBold(true);

    for (int i = 0; i < n; i++) {
        // Col 0: Present state label — read-only, auto-generated
        auto* psItem = new QTableWidgetItem(QString(QChar('A' + i)));
        psItem->setFlags(Qt::ItemIsEnabled);          // not editable
        psItem->setFont(bold);
        psItem->setTextAlignment(Qt::AlignCenter);
        psItem->setBackground(palette().midlight());
        m_table->setItem(i, 0, psItem);

        // Cols 1-3: editable
        for (int c = 1; c <= 3; c++) {
            auto* cell = new QTableWidgetItem("");
            cell->setTextAlignment(Qt::AlignCenter);
            m_table->setItem(i, c, cell);
        }
    }

    connect(m_table, &QTableWidget::itemChanged, this, &StateInputTable::dataChanged);
}

void StateInputTable::resize(int n) {
    std::vector<std::array<QString,3>> old;
    for (int i = 0; i < m_n && i < m_table->rowCount(); i++) {
        std::array<QString,3> row;
        for (int c = 0; c < 3; c++) {
            auto* item = m_table->item(i, c + 1);
            row[c] = item ? item->text() : "";
        }
        old.push_back(row);
    }

    setupTable(n);

    for (int i = 0; i < (int)old.size() && i < n; i++) {
        for (int c = 0; c < 3; c++) {
            m_table->item(i, c + 1)->setText(old[i][c]);
        }
    }
}

std::vector<State> StateInputTable::getStates() const {
    std::vector<State> states;
    QStringList validLabels;
    for (int i = 0; i < m_n; i++)
        validLabels << QString(QChar('A' + i));

    for (int i = 0; i < m_n; i++) {
        auto* ns0Item  = m_table->item(i, 1);
        auto* ns1Item  = m_table->item(i, 2);
        auto* outItem  = m_table->item(i, 3);

        if (!ns0Item || !ns1Item || !outItem ||
            ns0Item->text().trimmed().isEmpty() ||
            ns1Item->text().trimmed().isEmpty() ||
            outItem->text().trimmed().isEmpty())
        {
            QMessageBox::warning(nullptr, "Incomplete Data",
                QString("Row %1 (%2) has empty cells. Please fill all fields.")
                    .arg(i + 1)
                    .arg(QChar('A' + i)));
            return {};
        }

        QString ns0 = ns0Item->text().trimmed().toUpper();
        QString ns1 = ns1Item->text().trimmed().toUpper();
        QString out = outItem->text().trimmed();

        if (!validLabels.contains(ns0)) {
            QMessageBox::warning(nullptr, "Invalid Next State",
                QString("Row %1: NS(x=0) = \"%2\" is not a valid state label.\n"
                        "Valid labels are: %3")
                    .arg(i + 1).arg(ns0).arg(validLabels.join(", ")));
            return {};
        }
        if (!validLabels.contains(ns1)) {
            QMessageBox::warning(nullptr, "Invalid Next State",
                QString("Row %1: NS(x=1) = \"%2\" is not a valid state label.\n"
                        "Valid labels are: %3")
                    .arg(i + 1).arg(ns1).arg(validLabels.join(", ")));
            return {};
        }

        bool ok;
        int outVal = out.toInt(&ok);
        if (!ok) {
            QMessageBox::warning(nullptr, "Invalid Output",
                QString("Row %1: Output \"%2\" is not an integer.")
                    .arg(i + 1).arg(out));
            return {};
        }

        State s;
        s.label  = 'A' + i;
        s.ns[0]  = ns0[0].toLatin1();
        s.ns[1]  = ns1[0].toLatin1();
        s.output = outVal;
        states.push_back(s);
    }

    return states;
}

bool StateInputTable::validateCell(int /*row*/, int /*col*/, QString& /*errMsg*/) const {
    return true; 
}
