#ifndef CONSOLECONNECTION_H
#define CONSOLECONNECTION_H

#include <QtCore/QObject>
#include <QtCore/QTextStream>

/**
 * This class is meant to be used in connection with gnome-shell extension to compensate for the lack of tray icons in gnome-shell.
 */
class StdinConnection : public QObject
{
    Q_OBJECT

public:
    StdinConnection(QObject *parent);
    virtual ~StdinConnection();

signals:
    void toggleVisible();
    void close();

private slots:
    void read();

private:
    QTextStream m_stdin;
};

#endif // CONSOLECONNECTION_H
