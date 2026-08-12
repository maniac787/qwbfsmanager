#ifndef FRESHCORE_PCOREUTILS_H
#define FRESHCORE_PCOREUTILS_H

#include <QString>
#include <QStringList>
#include <QDir>

class pCoreUtils
{
public:
    static QString toTitleCase( const QString& text );
    static QString fileSizeToString( qint64 bytes );
    static QString fileSizeAdaptString( double value );
    static QStringList findFiles( const QString& rootPath, const QStringList& filters, bool recursive );
    static QStringList findFiles( const QDir& rootDir, const QStringList& filters, bool recursive );
    static bool isEmptyDirectory( const QString& path );
};

#endif // FRESHCORE_PCOREUTILS_H
