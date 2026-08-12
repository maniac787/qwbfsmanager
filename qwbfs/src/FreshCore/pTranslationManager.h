#ifndef FRESHCORE_PTRANSLATIONMANAGER_H
#define FRESHCORE_PTRANSLATIONMANAGER_H

#include <QLocale>
#include <QSet>
#include <QStringList>

class QTranslator;

class pTranslationManager
{
public:
    static pTranslationManager* instance();

    void setFakeCLocaleEnabled( bool enabled );
    void addTranslationsMask( const QString& mask );
    void addForbiddenTranslationsMask( const QString& mask );
    void setTranslationsPaths( const QStringList& paths );
    QStringList translationsPaths() const;
    void setCurrentLocale( const QString& localeName );
    QLocale currentLocale() const;
    QStringList availableLocales() const;
    void reloadTranslations();

private:
    pTranslationManager();
    static pTranslationManager* s_instance;

    bool m_fakeCLocaleEnabled;
    QStringList m_paths;
    QSet<QString> m_masks;
    QSet<QString> m_forbiddenMasks;
    QLocale m_currentLocale;
    QList<QTranslator*> m_translators;
};

#endif // FRESHCORE_PTRANSLATIONMANAGER_H
