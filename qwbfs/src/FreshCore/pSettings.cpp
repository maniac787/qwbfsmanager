#include "pSettings.h"

#include <QCoreApplication>

pSettings::Properties pSettings::s_defaultProperties = pSettings::Properties();

pSettings::pSettings( QObject* parent )
    : QObject( parent )
    , m_settings(
        s_defaultProperties.organizationName.isEmpty() ? QCoreApplication::organizationName() : s_defaultProperties.organizationName,
        s_defaultProperties.applicationName.isEmpty() ? QCoreApplication::applicationName() : s_defaultProperties.applicationName
    )
{}

QVariant pSettings::value( const QString& key, const QVariant& defaultValue ) const
{
    return m_settings.value( key, defaultValue );
}

void pSettings::setValue( const QString& key, const QVariant& value )
{
    m_settings.setValue( key, value );
}

void pSettings::setDefaultProperties( const Properties& properties )
{
    s_defaultProperties = properties;
}
