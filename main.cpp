#include <QtWidgets/QApplication>
#include <QtWidgets/QMenu>
#include <QtWidgets/QSystemTrayIcon>
#include <QtX11Extras/QX11Info>

#include "ConfigWindow.h"
#include "StdinConnection.h"

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xvnc.h>


int main(int argc, char **argv)
{
    bool tray = false;
    bool stdin_connnection = false;
    bool start_setup = false;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--tray") == 0) {
            tray = true;
        } else if (strcmp(argv[i], "--stdin") == 0) {
            stdin_connnection = true;
        } else if (strcmp(argv[i], "--start-setup") == 0) {
            start_setup = true;
        }
    }

    QApplication app(argc, argv);

    Display *display = QX11Info::display();
    if (!display) {
        qWarning("Not running on X server, quitting.");
        return 1;
    }

    int event_base, error_base;
    if (!XVncExtQueryExtension(display, &event_base, &error_base)) {
        qWarning("No VNC extension found, quitting.");
        return 1;
    }

    VncConfiguration configuration(display);
    configuration.read();
    if (!configuration.connect()) {
        qWarning("VNC not managed by vncmanager, quitting.");
        return 1;
    }
    if (start_setup) {
        configuration.normalizeSettings();
        configuration.apply();
    }

    ConfigWindow window(configuration, tray || stdin_connnection);

    if (tray) {
        QSystemTrayIcon *tray = new QSystemTrayIcon(window.style()->standardIcon(QStyle::SP_DesktopIcon), &window); // XXX: Better icon?
        tray->setToolTip(ConfigWindow::tr("VNC Session Configuration"));

        QMenu *menu = new QMenu();
        QAction *quitAction = menu->addAction("Quit");
        QObject::connect(quitAction, SIGNAL(triggered(bool)), &app, SLOT(quit()));
        tray->setContextMenu(menu);

        QObject::connect(tray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), &window, SLOT(trayActivated(QSystemTrayIcon::ActivationReason)));

        tray->show();
    } else if (stdin_connnection) {
        StdinConnection *connection = new StdinConnection(&window);

        QObject::connect(connection, SIGNAL(toggleVisible()), &window, SLOT(toggleVisible()));
        QObject::connect(connection, SIGNAL(close()), &app, SLOT(quit()));
    } else {
        window.show();
    }

    return app.exec();
}
