#pragma once
#include <QDialog>

class QSpinBox;

class SetupDialog : public QDialog {
    Q_OBJECT

public:
    explicit SetupDialog(QWidget* parent = nullptr);
    int stateCount() const;

private:
    QSpinBox* m_spin;
};
