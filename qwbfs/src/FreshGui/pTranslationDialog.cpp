#include "pTranslationDialog.h"

#include "../FreshCore/pTranslationManager.h"

#include <QInputDialog>
#include <QLocale>

QString pTranslationDialog::getLocale( pTranslationManager* manager, QWidget* parent )
{
    if ( !manager ) {
        return QString();
    }

    QStringList locales = manager->availableLocales();
    if ( locales.isEmpty() ) {
        locales << QLocale::system().name() << "en_US";
    }
    locales.removeDuplicates();
    locales.sort();

    bool ok = false;
    const QString current = manager->currentLocale().name();
    const int idx = qMax( 0, locales.indexOf( current ) );
    const QString locale = QInputDialog::getItem( parent, QObject::tr( "Language" ), QObject::tr( "Choose locale" ), locales, idx, false, &ok );
    return ok ? locale : QString();
}
