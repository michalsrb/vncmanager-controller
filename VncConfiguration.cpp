#include "VncConfiguration.h"

#include <strings.h>
#include <unistd.h>

#include <iostream>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xvnc.h>

// We don't need this macros and they polute our namespace.
#undef None
#undef Bool
#undef Status

#include <QtNetwork/QLocalSocket>
#include <QtCore/QProcessEnvironment>
#include <QtCore/QSettings>


VncConfiguration::VncConfiguration(Display *display)
    : m_display(display)
    , m_managerConnection(display)
{}

bool VncConfiguration::connect()
{
    return m_managerConnection.connect();
}

void VncConfiguration::read()
{
    int length;
    char *value;

    // Persistence
    if (XVncExtGetParam(m_display, "MaxDisconnectionTime", &value, &length)) {
        if (length == 1 && value[0] == '0') {
            m_persistence = Persistence::Persistent;
        } else {
            m_persistence = Persistence::NonPersistent;
        }
        XFree(value);
    }

    // Sharing
    bool NeverShared = false;
    if (XVncExtGetParam(m_display, "NeverShared", &value, &length)) {
        NeverShared = (length == 1 && value[0] == '1');
        XFree(value);
    }
    bool DisconnectClients = false;
    if (XVncExtGetParam(m_display, "DisconnectClients", &value, &length)) {
        DisconnectClients = (length == 1 && value[0] == '1');
        XFree(value);
    }

    if (NeverShared && DisconnectClients) {
        m_sharing = Sharing::OneClient;
    } else if (!NeverShared && !DisconnectClients) {
        m_sharing = Sharing::MultipleClients;
    } else {
        m_sharing = Sharing::Unknown;
    }

    // Security
    m_plainUsers.clear();

    if (XVncExtGetParam(m_display, "SecurityTypes", &value, &length)) {
        if (strcasecmp(value, "None") == 0) {
            m_security = Security::None;
        } else if (strcasecmp(value, "VncAuth") == 0) {
            m_security = Security::VncAuth;
        } else if (strcasecmp(value, "Plain") == 0) {
            m_security = Security::Plain;
        } else {
            m_security = Security::Unknown;
        }
        XFree(value);
    }

    // Reading this in no matter what is the current security type
    if (XVncExtGetParam(m_display, "PasswordFile", &value, &length)) {
        m_passwordFile = QString::fromLatin1(value, length); // TODO: Verify if it's ascii
        XFree(value);
    }

    // Reading this in no matter what is the current security type
    if (XVncExtGetParam(m_display, "PlainUsers", &value, &length)) {
        m_plainUsers = QString::fromLatin1(value, length).split(',', QString::SkipEmptyParts); // TODO: Verify if it's ascii
        XFree(value);
    }

    // Name
    if (XVncExtGetParam(m_display, "Desktop", &value, &length)) {
        m_sessionName = QString::fromLatin1(value, length);
        XFree(value);
    }
}

void VncConfiguration::normalizeSettings()
{
    // Persistence
    if (m_persistence == Persistence::Unknown) {
        m_persistence = Persistence::NonPersistent;
    }

    // Sharing
    if (m_sharing == Sharing::Unknown) {
        m_sharing = Sharing::OneClient;
    }

    // Security
    QString possibleUsername = queryUserName();
    if (!possibleUsername.isEmpty()) {
        if (m_security == Security::Unknown || m_security == Security::None) {
            m_security = Security::Plain;
            m_plainUsers.clear();
            m_plainUsers.append(possibleUsername);
        }
    }

    // Name
    QString possibleName = querySessionName();
    if (!possibleName.isEmpty()) {
        m_sessionName = possibleName;
    }
}

void VncConfiguration::apply()
{
    // Sharing 1
    switch (m_sharing) {
    case Sharing::OneClient:
        XVncExtSetParam(m_display, "NeverShared=1");
        XVncExtSetParam(m_display, "DisconnectClients=1");
        break;

    case Sharing::MultipleClients:
        // Will be done after setting security
        break;
    }

    // Security
    switch (m_security) {
    case Security::None:
        XVncExtSetParam(m_display, "SecurityTypes=None");
        break;

    case Security::VncAuth:
        XVncExtSetParam(m_display, "SecurityTypes=VncAuth");
        XVncExtSetParam(m_display, (QString("PasswordFile=") + m_passwordFile).toLatin1());
        break;

    case Security::Plain:
        XVncExtSetParam(m_display, "SecurityTypes=Plain");
        XVncExtSetParam(m_display, (QString("PlainUsers=") + m_plainUsers.join(",")).toLatin1());
        break;
    }

    // Sharing 2
    switch (m_sharing) {
    case Sharing::OneClient:
        // Was done before setting security
        break;

    case Sharing::MultipleClients:
        XVncExtSetParam(m_display, "DisconnectClients=0");
        XVncExtSetParam(m_display, "NeverShared=0");
        break;
    }

    // Persistence
    switch (m_persistence) {
    case Persistence::Persistent:
        XVncExtSetParam(m_display, "MaxDisconnectionTime=0");
        break;

    case Persistence::NonPersistent:
        XVncExtSetParam(m_display, "MaxDisconnectionTime=5"); // XXX: Too hardcoded
        break;
    }

    // Name
    XVncExtSetParam(m_display, ("Desktop=" + m_sessionName).toLatin1());

    if (m_managerConnection.connected()) {
        // Send session info
        m_managerConnection.sendSessionInfo(m_persistence == Persistence::Persistent);
    }
}

void VncConfiguration::setSessionName(QString sessionName)
{
    m_sessionName = sessionName;
}

void VncConfiguration::setPersistence(VncConfiguration::Persistence persistence)
{
    m_persistence = persistence;
}

void VncConfiguration::setSharing(VncConfiguration::Sharing sharing)
{
    m_sharing = sharing;
}

void VncConfiguration::setSecurityNone()
{
    m_security = Security::None;
}

void VncConfiguration::setSecurityVncAuth(QString password, QString viewonlyPassword)
{
    m_security = Security::VncAuth;

    throw std::runtime_error("Not implemented.");
}

void VncConfiguration::setSecurityVncAuth(QString authFilename)
{
    m_security = Security::VncAuth;

    throw std::runtime_error("Not implemented.");
}

void VncConfiguration::setSecurityPlain()
{
    m_security = Security::Plain;
}

void VncConfiguration::setSecurityPlain(QStringList plainUsers)
{
    m_security = Security::Plain;
    m_plainUsers = plainUsers;
}

QString VncConfiguration::querySessionName()
{
    QString desktopSession = QProcessEnvironment::systemEnvironment().value("DESKTOP_SESSION");
    if (!desktopSession.isEmpty()) {
        // XXX: Misusing QSettings to read desktop file
        QSettings desktopFile("/usr/share/xsessions/" + desktopSession + ".desktop", QSettings::IniFormat);
        if (desktopFile.contains("Desktop Entry/Name")) {
            return desktopFile.value("Desktop Entry/Name").toString();
        }
    }

    QString xdg_current_desktop = QProcessEnvironment::systemEnvironment().value("XDG_CURRENT_DESKTOP");
    return xdg_current_desktop;
}

QString VncConfiguration::queryUserName()
{
    return QProcessEnvironment::systemEnvironment().value("USER");
}
