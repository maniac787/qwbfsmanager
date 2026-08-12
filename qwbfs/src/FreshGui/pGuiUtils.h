#ifndef FRESHGUI_PGUIUTILS_H
#define FRESHGUI_PGUIUTILS_H

#include <QPixmap>
#include <QRect>
#include <QString>

class QWidget;

class pGuiUtils
{
public:
    static QString cacheKey( const QString& url, const QSize& size );
    static QPixmap scaledPixmap( const QString& resourcePath, const QSize& size );
    static QPixmap scaledPixmap( const QPixmap& pixmap, const QString& key, const QSize& size );
    static QRect saveGeometry( QWidget* widget );
    static void restoreGeometry( QWidget* widget, const QRect& geometry );
};

#endif // FRESHGUI_PGUIUTILS_H
