#include "ConfigWindow.h"

#include <QtWidgets/QLabel>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QAction>
#include <QtGui/QShowEvent>

ConfigWindow::ConfigWindow(VncConfiguration &data, bool dontClose)
    : m_data(data)
    , m_dontClose(dontClose)
{
    ui.setupUi(this);

    // Disabled for now:
    ui.chkSecurityVncAuth->setVisible(false);
    ui.widgetVncAuth->setVisible(false);

    // Not supposed to be visible:
    ui.chkSecurityUnknown->setVisible(false);
    ui.chkSharingUnknown->setVisible(false);

    adjustSize();

    connect(ui.buttonBox, SIGNAL(clicked(QAbstractButton *)), this, SLOT(buttonBarClicked(QAbstractButton *)));
}

ConfigWindow::~ConfigWindow()
{}

void ConfigWindow::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    m_data.read();

    ui.editSessionName->setText(m_data.getSessionName());

    ui.chkPersistencePersistent->setChecked(m_data.getPersistence() == VncConfiguration::Persistence::Persistent);
    ui.chkPersistenceNonPersistent->setChecked(m_data.getPersistence() == VncConfiguration::Persistence::NonPersistent);

    ui.chkSecurityUnknown->setChecked(m_data.getSecurity() == VncConfiguration::Security::Unknown);
    ui.chkSecurityNone->   setChecked(m_data.getSecurity() == VncConfiguration::Security::None);
    ui.chkSecurityVncAuth->setChecked(m_data.getSecurity() == VncConfiguration::Security::VncAuth);
    ui.chkSecurityPlain->  setChecked(m_data.getSecurity() == VncConfiguration::Security::Plain);

    ui.chkSharingUnknown->        setChecked(m_data.getSharing() == VncConfiguration::Sharing::Unknown);
    ui.chkSharingOneClient->      setChecked(m_data.getSharing() == VncConfiguration::Sharing::OneClient);
    ui.chkSharingMultipleClients->setChecked(m_data.getSharing() == VncConfiguration::Sharing::MultipleClients);

    ui.editAllowedUsers->setText(m_data.getPlainUsers().join(","));
}

void ConfigWindow::closeEvent(QCloseEvent *event)
{
    if (m_dontClose) {
        event->ignore();
        hide();
    } else {
        event->accept();
    }
}

void ConfigWindow::apply()
{
    m_data.setSessionName(ui.editSessionName->text());

    if (ui.chkPersistencePersistent->isChecked()) {
        m_data.setPersistence(VncConfiguration::Persistence::Persistent);
    } else if (ui.chkPersistenceNonPersistent->isChecked()) {
        m_data.setPersistence(VncConfiguration::Persistence::NonPersistent);
    }

    if (ui.chkSecurityNone->isChecked()) {
        m_data.setSecurityNone();
    } else if (ui.chkSecurityVncAuth->isChecked()) {
        m_data.setSecurityVncAuth(ui.editPassword->text(), ui.editViewonlyPassword->text());
    } else if (ui.chkSecurityPlain->isChecked()) {
        m_data.setSecurityPlain(ui.editAllowedUsers->text().split(','));
    }

    if (ui.chkSharingOneClient->isChecked()) {
        m_data.setSharing(VncConfiguration::Sharing::OneClient);
    } else if (ui.chkSharingMultipleClients->isChecked()) {
        m_data.setSharing(VncConfiguration::Sharing::MultipleClients);
    }

    m_data.apply();
}

void ConfigWindow::done()
{
    if (m_dontClose) {
        hide();
    } else {
        close();
    }
}

void ConfigWindow::buttonBarClicked(QAbstractButton *button)
{
    switch (ui.buttonBox->buttonRole(button)) {
    case QDialogButtonBox::AcceptRole:
        apply();
        done();
        break;

    case QDialogButtonBox::ApplyRole:
        apply();
        break;

    case QDialogButtonBox::RejectRole:
        done();
        break;
    }
}

void ConfigWindow::trayActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::Trigger:
        toggleVisible();

        break;
    }
}

void ConfigWindow::toggleVisible()
{
    if (isVisible()) {
        done();
    } else {
        show();
    }
}


#include "ConfigWindow.moc"
