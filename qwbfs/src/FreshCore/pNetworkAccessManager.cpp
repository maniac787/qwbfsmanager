#include "pNetworkAccessManager.h"

#include <QDir>
#include <QNetworkDiskCache>
#include <QNetworkReply>

pNetworkAccessManager* pNetworkAccessManager::s_instance = nullptr;

pNetworkAccessManager* pNetworkAccessManager::instance()
{
    if ( !s_instance ) {
        s_instance = new pNetworkAccessManager();
    }
    return s_instance;
}

pNetworkAccessManager::pNetworkAccessManager( QObject* parent )
    : QNetworkAccessManager( parent )
{
    m_cache = new QNetworkDiskCache( this );
    setCache( m_cache );
    connect( this, &QNetworkAccessManager::finished, this, &pNetworkAccessManager::onFinished );
}

void pNetworkAccessManager::setCacheDirectory( const QString& path )
{
    QDir().mkpath( path );
    m_cache->setCacheDirectory( path );
}

void pNetworkAccessManager::setMaximumCacheSize( qint64 size )
{
    m_cache->setMaximumCacheSize( size );
}

bool pNetworkAccessManager::hasCacheData( const QUrl& url ) const
{
    return m_cache->data( url ) != nullptr;
}

bool pNetworkAccessManager::hasCacheData( const QString& url ) const
{
    return hasCacheData( QUrl( url ) );
}

QIODevice* pNetworkAccessManager::cacheData( const QUrl& url ) const
{
    return m_cache->data( url );
}

QIODevice* pNetworkAccessManager::cacheData( const QString& url ) const
{
    return cacheData( QUrl( url ) );
}

void pNetworkAccessManager::clearCache()
{
    m_cache->clear();
    emit cacheCleared();
}

void pNetworkAccessManager::onFinished( QNetworkReply* reply )
{
    if ( !reply ) {
        return;
    }

    if ( reply->error() == QNetworkReply::NoError ) {
        emit cached( reply->url() );
        return;
    }

    emit error( reply->url(), reply->errorString() );
}
