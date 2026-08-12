#include "pTranslationManager.h"

#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QTranslator>

namespace {

QString localeFromQmBaseName( const QString& baseName )
{
    static const QRegularExpression re( QStringLiteral( "(?:[-_])([a-z]{2,3}(?:_[A-Z]{2})?)$" ) );
    const QRegularExpressionMatch match = re.match( baseName );
    return match.hasMatch() ? match.captured( 1 ) : QString();
}

bool fileMatchesLocaleCode( const QString& fileName, const QString& code )
{
    if ( code.isEmpty() || !fileName.endsWith( QLatin1String( ".qm" ) ) ) {
        return false;
    }

    const QString baseName = fileName.left( fileName.size() - 3 );
    return baseName.endsWith( QLatin1Char( '_' ) + code )
        || baseName.endsWith( QLatin1Char( '-' ) + code );
}

} // namespace

pTranslationManager* pTranslationManager::s_instance = nullptr;

pTranslationManager* pTranslationManager::instance()
{
    if ( !s_instance ) {
        s_instance = new pTranslationManager();
    }
    return s_instance;
}

pTranslationManager::pTranslationManager()
    : m_fakeCLocaleEnabled( false )
    , m_currentLocale( QLocale::system() )
{}

void pTranslationManager::setFakeCLocaleEnabled( bool enabled )
{
    m_fakeCLocaleEnabled = enabled;
    Q_UNUSED( m_fakeCLocaleEnabled );
}

void pTranslationManager::addTranslationsMask( const QString& mask )
{
    if ( !mask.isEmpty() ) {
        m_masks.insert( mask );
    }
}

void pTranslationManager::addForbiddenTranslationsMask( const QString& mask )
{
    if ( !mask.isEmpty() ) {
        m_forbiddenMasks.insert( mask );
    }
}

void pTranslationManager::setTranslationsPaths( const QStringList& paths )
{
    m_paths = paths;
}

QStringList pTranslationManager::translationsPaths() const
{
    return m_paths;
}

void pTranslationManager::setCurrentLocale( const QString& localeName )
{
    if ( !localeName.isEmpty() ) {
        m_currentLocale = QLocale( localeName );
    }
}

QLocale pTranslationManager::currentLocale() const
{
    return m_currentLocale;
}

QStringList pTranslationManager::availableLocales() const
{
    QSet<QString> locales;
    for ( const QString& path : m_paths ) {
        QDir dir( path );
        const QFileInfoList files = dir.entryInfoList( QStringList() << QStringLiteral( "*.qm" ), QDir::Files );
        for ( const QFileInfo& fi : files ) {
            const QString fileName = fi.fileName();
            if ( QDir::match( m_forbiddenMasks.values(), fileName ) ) {
                continue;
            }

            const QString locale = localeFromQmBaseName( fi.completeBaseName() );
            // Prefer full locales (es_ES) so app files like qwbfsmanager-es_ES.qm are selectable.
            if ( locale.contains( QLatin1Char( '_' ) ) ) {
                locales.insert( locale );
            }
        }
    }
    return locales.values();
}

void pTranslationManager::reloadTranslations()
{
    QApplication* app = qobject_cast<QApplication*>( QApplication::instance() );
    if ( !app ) {
        return;
    }

    for ( QTranslator* translator : m_translators ) {
        app->removeTranslator( translator );
        delete translator;
    }
    m_translators.clear();

    const QString localeName = m_currentLocale.name();
    const QString shortName = localeName.section( QLatin1Char( '_' ), 0, 0 );
    const QStringList masks = m_masks.isEmpty() ? QStringList() << QStringLiteral( "*.qm" ) : m_masks.values();

    for ( const QString& path : m_paths ) {
        QDir dir( path );
        const QFileInfoList files = dir.entryInfoList( masks, QDir::Files );
        for ( const QFileInfo& fi : files ) {
            const QString fileName = fi.fileName();
            if ( QDir::match( m_forbiddenMasks.values(), fileName ) ) {
                continue;
            }
            if ( !fileMatchesLocaleCode( fileName, localeName )
                && !fileMatchesLocaleCode( fileName, shortName ) ) {
                continue;
            }

            QTranslator* translator = new QTranslator( app );
            if ( translator->load( fi.absoluteFilePath() ) ) {
                app->installTranslator( translator );
                m_translators << translator;
            } else {
                delete translator;
            }
        }
    }
}
