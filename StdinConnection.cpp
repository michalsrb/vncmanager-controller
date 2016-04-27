#include <QtCore/QSocketNotifier>

#include "StdinConnection.h"


StdinConnection::StdinConnection(QObject *parent)
    : QObject(parent)
    , m_stdin(stdin, QIODevice::ReadOnly)
{
    QSocketNotifier *n1 = new QSocketNotifier(0, QSocketNotifier::Read, this);
    connect(n1, SIGNAL(activated(int)), this, SLOT(read()));

    QSocketNotifier *n2 = new QSocketNotifier(0, QSocketNotifier::Exception, this);
    connect(n2, SIGNAL(activated(int)), this, SIGNAL(close()));

    QTextStream m_stdout(stdout, QIODevice::WriteOnly);
    m_stdout << "OK\n";
    m_stdout.flush();
}

StdinConnection::~StdinConnection()
{
}

void StdinConnection::read()
{
    if (m_stdin.atEnd()) {
        emit close();
        return;
    }

    QString cmd;
    m_stdin >> cmd;

    if (cmd.isEmpty() || cmd == "QUIT") {
        emit close();
        return;
    }

    if (cmd == "TOGGLE") {
        emit toggleVisible();
        return;
    }
}


#include "StdinConnection.moc"
