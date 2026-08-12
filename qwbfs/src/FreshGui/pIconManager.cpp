#include "pIconManager.h"

#include <QDir>

QPixmap pIconManager::pixmap( const QString& fileName, const QString& prefix )
{
    if ( prefix.isEmpty() ) {
        return QPixmap( fileName );
    }
    const QString path = QString( "%1/%2" ).arg( prefix ).arg( fileName );
    return QPixmap( QDir::cleanPath( path ) );
}
