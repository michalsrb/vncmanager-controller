#ifndef DATA_H
#define DATA_H

#include <QtCore/QStringList>

#include "ManagerConnection.h"


typedef struct _XDisplay Display;

class VncConfiguration
{
public:
    enum class Persistence
    {
        Unknown, NonPersistent, Persistent
    };

    enum class Security
    {
        Unknown, None, VncAuth, Plain
    };

    enum class Sharing
    {
        Unknown, OneClient, MultipleClients
    };

public:
    VncConfiguration(Display *display);

    /// Connect to vncmanager's control socket
    bool connect();

    /// Read the current settings from X server
    void read();

    /// Change current settings to reasonable default
    void normalizeSettings();

    /// Apply the settings to X server
    void apply();

    QString getSessionName() const { return m_sessionName; }
    void setSessionName(QString sessionName);

    Persistence getPersistence() const { return m_persistence; }
    void setPersistence(Persistence persistence);

    Sharing getSharing() const { return m_sharing; }
    void setSharing(Sharing sharing);

    Security getSecurity() const { return m_security; }

    void setSecurityNone();

    void setSecurityVncAuth(QString password, QString viewonlyPassword);
    void setSecurityVncAuth(QString authFilename);

    QStringList getPlainUsers() const { return m_plainUsers; }
    void setSecurityPlain();
    void setSecurityPlain(QStringList plainUsers);

private:
    QString querySessionName();
    QString queryUserName();

private:
    Display *m_display;

    ManagerConnection m_managerConnection;

    QString m_sessionName;

    Persistence m_persistence = Persistence::Unknown;
    Sharing m_sharing = Sharing::Unknown;
    Security m_security = Security::Unknown;

    QString m_passwordFile;

    QStringList m_plainUsers;
};

#endif // DATA_H
