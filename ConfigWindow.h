#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QSystemTrayIcon>

#include "VncConfiguration.h"

#include "ui_ConfigWindow.h"

class ConfigWindow : public QMainWindow
{
    Q_OBJECT

public:
    ConfigWindow(VncConfiguration &data, bool dontClose = false);
    virtual ~ConfigWindow();

    virtual void showEvent(QShowEvent *);
    virtual void closeEvent(QCloseEvent *);

private:
    void apply();
    void done();

public slots:
    void trayActivated(QSystemTrayIcon::ActivationReason reason);
    void toggleVisible();

private slots:
    void buttonBarClicked(QAbstractButton *button);

private:
    Ui_ConfigWindow ui;

    VncConfiguration &m_data;
    bool m_dontClose;
};

#endif // CONFIGURATION_H
