#ifndef FRESHCORE_PSETTINGS_H
#define FRESHCORE_PSETTINGS_H

#include <QObject>
#include <QSettings>
#include <QVariant>

class pSettings : public QObject
{
    Q_OBJECT

public:
    enum Type {
        Auto
    };

    struct Properties
    {
        Properties( const QString& organization = QString(), const QString& application = QString(), Type t = Auto )
            : organizationName( organization ), applicationName( application ), type( t )
        {}

        QString organizationName;
        QString applicationName;
        Type type;
    };

    explicit pSettings( QObject* parent = nullptr );

    QVariant value( const QString& key, const QVariant& defaultValue = QVariant() ) const;
    void setValue( const QString& key, const QVariant& value );

    static void setDefaultProperties( const Properties& properties );

private:
    static Properties s_defaultProperties;
    QSettings m_settings;
};

#endif // FRESHCORE_PSETTINGS_H
