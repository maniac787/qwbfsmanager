#ifndef FRESHGUI_PUPDATECHECKER_H
#define FRESHGUI_PUPDATECHECKER_H

#include <QObject>
#include <QDateTime>
#include <QUrl>

class QAction;

class pUpdateChecker : public QObject
{
    Q_OBJECT

public:
    explicit pUpdateChecker( QObject* parent = nullptr );

    void setDownloadsFeedUrl( const QUrl& url );
    void setVersion( const QString& version );
    void setVersionString( const QString& versionString );
    void setVersionDiscoveryPattern( const QString& pattern );
    QAction* menuAction() const;
    void silentCheck();

    void setLastUpdated( const QDateTime& value );
    void setLastChecked( const QDateTime& value );
    QDateTime lastUpdated() const;
    QDateTime lastChecked() const;

private:
    QAction* m_action;
    QUrl m_feedUrl;
    QString m_version;
    QString m_versionString;
    QString m_pattern;
    QDateTime m_lastUpdated;
    QDateTime m_lastChecked;
};

#endif // FRESHGUI_PUPDATECHECKER_H
