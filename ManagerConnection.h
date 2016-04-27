#ifndef MANAGERCONNECTION_H
#define MANAGERCONNECTION_H

#include <QtCore/QString>
#include <QtCore/QTextStream>
#include <QtNetwork/QLocalSocket>


typedef struct _XDisplay Display;

class ManagerConnection
{
private:
    const QString socketPath = "/run/vncmanager/control/control";

    const std::size_t keyLength = 32;

public:
    ManagerConnection(Display *display);

    bool connect();

    bool connected() const { return m_connected; }

    void sendSessionInfo(bool visible);

private:
    Display *m_display;

    bool m_connected = false;

    QLocalSocket m_socket;
    QTextStream m_stream;
};

#endif // MANAGERCONNECTION_H
