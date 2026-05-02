#include "SetupDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QPushButton>
#include <QFormLayout>

SetupDialog::SetupDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("State Machine Minimizer");
    setModal(true);
    setMinimumWidth(320);

    auto* root = new QVBoxLayout(this);
    root->setSpacing(16);
    root->setContentsMargins(24, 24, 24, 24);

    auto* title = new QLabel("<b>State Machine Minimizer</b>");
    title->setStyleSheet("font-size: 15px;");
    root->addWidget(title);

    auto* sub = new QLabel("Enter the number of states in your original machine:");
    sub->setWordWrap(true);
    root->addWidget(sub);

    auto* form = new QFormLayout();
    m_spin = new QSpinBox();
    m_spin->setRange(2, 26);  
    m_spin->setValue(4);
    m_spin->setFixedWidth(80);
    form->addRow("Number of states:", m_spin);
    root->addLayout(form);

    auto* btnRow = new QHBoxLayout();
    btnRow->addStretch();
    auto* ok = new QPushButton("Continue");
    ok->setDefault(true);
    ok->setFixedWidth(100);
    btnRow->addWidget(ok);
    root->addLayout(btnRow);

    connect(ok, &QPushButton::clicked, this, &QDialog::accept);
}

int SetupDialog::stateCount() const {
    return m_spin->value();
}
