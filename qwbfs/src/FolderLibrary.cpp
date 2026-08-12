#include "FolderLibrary.h"
#include "PartitionComboBox.h"
#include "qwbfsdriver/Driver.h"

#include "models/pPartitionModel.h"
#include "models/pPartition.h"

#include <FreshCore/pCoreUtils>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStorageInfo>

QString FolderLibrary::defaultImportPattern()
{
    return QStringLiteral( "%title [%id]/%id.%suffix" );
}

QString FolderLibrary::resolveLibraryPath( const QString& selection )
{
    if ( selection.isEmpty() ) {
        return QString();
    }

    const QFileInfo selectionInfo( selection );
    QString root;

    if ( selectionInfo.isDir() ) {
        root = selectionInfo.absoluteFilePath();
    }
    else {
        // Prefer mount points known by the partition model.
        foreach ( const pPartition& partition, PartitionComboBox::partitionModel()->partitions() ) {
            if ( partition.devicePath() != selection ) {
                continue;
            }

            const QVariant mountVariant = partition.property( pPartition::MountPoints );
            QStringList mountPoints = mountVariant.toStringList();
            if ( mountPoints.isEmpty() ) {
                const QString mount = mountVariant.toString();
                if ( !mount.isEmpty() ) {
                    mountPoints << mount;
                }
            }

            if ( !mountPoints.isEmpty() ) {
                root = mountPoints.first();
                break;
            }
        }

        // Fallback: match device against mounted volumes.
        if ( root.isEmpty() ) {
            foreach ( const QStorageInfo& volume, QStorageInfo::mountedVolumes() ) {
                if ( !volume.isValid() || !volume.isReady() ) {
                    continue;
                }
                if ( QString::fromUtf8( volume.device() ) == selection ) {
                    root = volume.rootPath();
                    break;
                }
            }
        }
    }

    if ( root.isEmpty() || !QFileInfo( root ).isDir() ) {
        return QString();
    }

    const QString wbfsDir = QDir( root ).filePath( QStringLiteral( "wbfs" ) );
    if ( QFileInfo( wbfsDir ).isDir() ) {
        return QDir::cleanPath( wbfsDir );
    }

    // Selection already points at .../wbfs
    if ( QFileInfo( root ).fileName().compare( QStringLiteral( "wbfs" ), Qt::CaseInsensitive ) == 0 ) {
        return QDir::cleanPath( root );
    }

    // Device/mount without wbfs yet: create the conventional folder when writable.
    if ( !selectionInfo.isDir() ) {
        if ( QDir().mkpath( wbfsDir ) && QFileInfo( wbfsDir ).isDir() ) {
            return QDir::cleanPath( wbfsDir );
        }
        return QString();
    }

    return QDir::cleanPath( root );
}

bool FolderLibrary::isLibraryPath( const QString& path )
{
    return !path.isEmpty() && QFileInfo( path ).isDir();
}

QWBFS::Model::DiscList FolderLibrary::listDiscs( const QString& libraryPath )
{
    QWBFS::Model::DiscList discs;

    if ( !isLibraryPath( libraryPath ) ) {
        return discs;
    }

    const QStringList files = pCoreUtils::findFiles(
        libraryPath,
        QStringList() << QStringLiteral( "*.wbfs" ),
        true );

    foreach ( const QString& filePath, files ) {
        QWBFS::Model::Disc disc;
        if ( QWBFS::Driver::wbfsFileInfo( filePath, disc ) == QWBFS::Driver::Ok && disc.isValid() ) {
            discs << disc;
        }
    }

    return discs;
}

QWBFS::Partition::Status FolderLibrary::storageStatus( const QString& libraryPath )
{
    QWBFS::Partition::Status status;

    if ( !isLibraryPath( libraryPath ) ) {
        return status;
    }

    QStorageInfo info( libraryPath );
    if ( !info.isValid() || !info.isReady() ) {
        return status;
    }

    status.size = info.bytesTotal();
    status.free = info.bytesAvailable();
    status.used = status.size >= status.free ? status.size - status.free : 0;
    return status;
}

bool FolderLibrary::removeDiscFile( const QWBFS::Model::Disc& disc )
{
    if ( disc.origin.isEmpty() || !QFile::exists( disc.origin ) ) {
        return false;
    }

    if ( !QFile::remove( disc.origin ) ) {
        return false;
    }

    const QString parentPath = QFileInfo( disc.origin ).absolutePath();
    if ( pCoreUtils::isEmptyDirectory( parentPath ) ) {
        QDir( parentPath ).rmdir( parentPath );
    }

    return true;
}
