#include "pGuiUtils.h"

#include <QPixmapCache>
#include <QWidget>

QString pGuiUtils::cacheKey( const QString& url, const QSize& size )
{
    return QString( "%1#%2x%3" ).arg( url ).arg( size.width() ).arg( size.height() );
}

QPixmap pGuiUtils::scaledPixmap( const QString& resourcePath, const QSize& size )
{
    QPixmap pixmap( resourcePath );
    if ( pixmap.isNull() || !size.isValid() ) {
        return pixmap;
    }
    return pixmap.scaled( size, Qt::KeepAspectRatio, Qt::SmoothTransformation );
}

QPixmap pGuiUtils::scaledPixmap( const QPixmap& pixmap, const QString& key, const QSize& size )
{
    QPixmap out = pixmap;
    if ( size.isValid() ) {
        out = pixmap.scaled( size, Qt::KeepAspectRatio, Qt::SmoothTransformation );
    }
    QPixmapCache::insert( cacheKey( key, size ), out );
    return out;
}

QRect pGuiUtils::saveGeometry( QWidget* widget )
{
    return widget ? widget->geometry() : QRect();
}

void pGuiUtils::restoreGeometry( QWidget* widget, const QRect& geometry )
{
    if ( widget && geometry.isValid() ) {
        widget->setGeometry( geometry );
    }
}
