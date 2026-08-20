#ifndef FRESHCORE_PWINHELPERS_H
#define FRESHCORE_PWINHELPERS_H

#include <QtGlobal>
#include <QString>

#ifdef UNICODE
#ifndef _UNICODE
#define _UNICODE
#endif
#define QStringToTCHAR( x )      (wchar_t*)(x).utf16()
#define PQStringToTCHAR( x )     (wchar_t*)(x)->utf16()
#define TCHARToQString( x )      QString::fromUtf16( reinterpret_cast<const char16_t*>( x ) )
#define TCHARToQStringN( x, y )  QString::fromUtf16( reinterpret_cast<const char16_t*>( x ), ( y ) )
#else
#define QStringToTCHAR( x )      (x).local8Bit().constData()
#define PQStringToTCHAR( x )     (x)->local8Bit().constData()
#define TCHARToQString( x )      QString::fromLocal8Bit( ( x ) )
#define TCHARToQStringN( x, y )  QString::fromLocal8Bit( ( x ), ( y ) )
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif

#endif // FRESHCORE_PWINHELPERS_H
