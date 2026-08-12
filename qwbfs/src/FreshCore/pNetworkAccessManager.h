#ifndef FRESHCORE_PNETWORKACCESSMANAGER_H
#define FRESHCORE_PNETWORKACCESSMANAGER_H

#include <QNetworkAccessManager>

class QNetworkDiskCache;

class pNetworkAccessManager : public QNetworkAccessManager
{
    Q_OBJECT

public:
    static pNetworkAccessManager* instance();

    void setCacheDirectory( const QString& path );
    void setMaximumCacheSize( qint64 size );
    bool hasCacheData( const QUrl& url ) const;
    bool hasCacheData( const QString& url ) const;
    QIODevice* cacheData( const QUrl& url ) const;
    QIODevice* cacheData( const QString& url ) const;
    void clearCache();

signals:
    void cached( const QUrl& url );
    void error( const QUrl& url, const QString& message );
    void cacheCleared();

private slots:
    void onFinished( QNetworkReply* reply );

private:
    explicit pNetworkAccessManager( QObject* parent = nullptr );
    static pNetworkAccessManager* s_instance;
    QNetworkDiskCache* m_cache;
};

#endif // FRESHCORE_PNETWORKACCESSMANAGER_H
