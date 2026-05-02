#include <QApplication>
#include "ui/SetupDialog.h"
#include "ui/MainWindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("State Machine Minimizer");
    app.setOrganizationName("SMMin");
    SetupDialog setup;
    if (setup.exec() != QDialog::Accepted)
        return 0;

    int n = setup.stateCount();

    MainWindow win(n);
    win.show();

    return app.exec();
}
