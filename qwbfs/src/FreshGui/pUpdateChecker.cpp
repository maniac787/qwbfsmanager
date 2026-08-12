#include "pUpdateChecker.h"

#include <QAction>

pUpdateChecker::pUpdateChecker( QObject* parent )
    : QObject( parent )
{
    m_action = new QAction( tr( "Check updates" ), this );
    connect( m_action, &QAction::triggered, this, &pUpdateChecker::silentCheck );
}

void pUpdateChecker::setDownloadsFeedUrl( const QUrl& url )
{
    m_feedUrl = url;
}

void pUpdateChecker::setVersion( const QString& version )
{
    m_version = version;
}

void pUpdateChecker::setVersionString( const QString& versionString )
{
    m_versionString = versionString;
    m_action->setText( tr( "Updates (%1)" ).arg( m_versionString ) );
}

void pUpdateChecker::setVersionDiscoveryPattern( const QString& pattern )
{
    m_pattern = pattern;
}

QAction* pUpdateChecker::menuAction() const
{
    return m_action;
}

void pUpdateChecker::silentCheck()
{
    m_lastChecked = QDateTime::currentDateTime();
    Q_UNUSED( m_feedUrl );
    Q_UNUSED( m_version );
    Q_UNUSED( m_pattern );
}

void pUpdateChecker::setLastUpdated( const QDateTime& value )
{
    m_lastUpdated = value;
}

void pUpdateChecker::setLastChecked( const QDateTime& value )
{
    m_lastChecked = value;
}

QDateTime pUpdateChecker::lastUpdated() const
{
    return m_lastUpdated;
}

QDateTime pUpdateChecker::lastChecked() const
{
    return m_lastChecked;
}
