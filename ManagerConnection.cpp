#include "ManagerConnection.h"

#include <assert.h>

#include <QtCore/QFile>
#include <QtCore/QIODevice>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xvnc.h>

// We don't need this macros and they polute our namespace.
#undef None
#undef Bool
#undef Status


ManagerConnection::ManagerConnection(Display *display)
    : m_display(display)
{}

bool ManagerConnection::connect()
{
    // Connect to vncmanager's control socket and send our display number
    m_socket.connectToServer(socketPath);
    if (!m_socket.waitForConnected()) {
        return false;
    }
    if (m_socket.state() != QLocalSocket::ConnectedState) {
        return false;
    }

    m_stream.setDevice(&m_socket);

    char *displayName = XDisplayName(nullptr);
    m_stream << QString(displayName).split(':').last() << '\n';
    m_stream.flush();

    m_socket.waitForReadyRead();
    QString response;
    m_stream >> response;
    if (response != "OK") {
        return false;
    }

    // Generate key
    QFile devrand("/dev/urandom");
    devrand.open(QIODevice::ReadOnly);

    QByteArray key;
    while (key.size() < keyLength) {
        key += devrand.read(keyLength - key.size());
    }
    devrand.close();

    QByteArray keyBase64 = key.toHex();

    // Send the key thru the Xvnc
    // vncmanager will catch the controller key and won't let it pass to the client, but just to keep things in order, lets remember the original name and set it back after
    char *originalDesktopName;
    int originalDesktopNameLength;
    if (!XVncExtGetParam(m_display, "Desktop", &originalDesktopName, &originalDesktopNameLength)) {
        return false;
    }
    if (!XVncExtSetParam(m_display, (QString("Desktop=CONTROLLER_KEY:") + keyBase64).toLatin1())) {
        return false;
    }

    // Send the key thru the socket
    m_stream << keyBase64 << '\n';
    m_stream.flush();

    // Wait for response
    m_socket.waitForReadyRead();
    m_stream >> response;

    // Set back the original name
    XVncExtSetParam(m_display, (QString("Desktop=") + QString::fromLatin1(originalDesktopName, originalDesktopNameLength)).toLatin1());
    XFree(originalDesktopName);

    // Check if the response was correct
    if (response != "OK") {
        return false;
    }

    m_connected = true;
    return true;
}

void ManagerConnection::sendSessionInfo(bool visible)
{
    assert(connected());

    m_stream << "VISIBLE " << visible << '\n';
    m_stream.flush();
}
