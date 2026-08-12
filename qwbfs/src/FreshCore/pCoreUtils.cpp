#include "pCoreUtils.h"

#include <QDir>
#include <QDirIterator>
#include <QLocale>
#include <QRegularExpression>

QString pCoreUtils::toTitleCase( const QString& text )
{
    QString out = text.toLower();
    static const QRegularExpression wordExpr( "\\b\\w" );
    QRegularExpressionMatchIterator it = wordExpr.globalMatch( out );
    while ( it.hasNext() ) {
        const QRegularExpressionMatch match = it.next();
        const int pos = match.capturedStart();
        if ( pos >= 0 && pos < out.size() ) {
            out[pos] = out.at( pos ).toUpper();
        }
    }
    return out;
}

QString pCoreUtils::fileSizeToString( qint64 bytes )
{
    return QLocale().formattedDataSize( bytes, 1, QLocale::DataSizeIecFormat );
}

QString pCoreUtils::fileSizeAdaptString( double value )
{
    return QString::number( value, 'f', 1 ) + "%";
}

QStringList pCoreUtils::findFiles( const QString& rootPath, const QStringList& filters, bool recursive )
{
    QStringList files;
    const QDirIterator::IteratorFlag flag = recursive ? QDirIterator::Subdirectories : QDirIterator::NoIteratorFlags;
    QDirIterator it( rootPath, filters, QDir::Files | QDir::NoDotAndDotDot, flag );
    while ( it.hasNext() ) {
        files << it.next();
    }
    return files;
}

QStringList pCoreUtils::findFiles( const QDir& rootDir, const QStringList& filters, bool recursive )
{
    return findFiles( rootDir.absolutePath(), filters, recursive );
}

bool pCoreUtils::isEmptyDirectory( const QString& path )
{
    QDir dir( path );
    return dir.exists() && dir.entryList( QDir::NoDotAndDotDot | QDir::AllEntries ).isEmpty();
}
