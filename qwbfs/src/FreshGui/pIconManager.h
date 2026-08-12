#ifndef FRESHGUI_PICONMANAGER_H
#define FRESHGUI_PICONMANAGER_H

#include <QPixmap>
#include <QString>

class pIconManager
{
public:
    static QPixmap pixmap( const QString& fileName, const QString& prefix = QString() );
};

#endif // FRESHGUI_PICONMANAGER_H
