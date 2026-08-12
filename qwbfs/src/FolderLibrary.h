#ifndef FOLDERLIBRARY_H
#define FOLDERLIBRARY_H

#include "models/Disc.h"
#include "qwbfsdriver/PartitionStatus.h"

class FolderLibrary
{
public:
    static QString defaultImportPattern();

    // Resolve a device path (/dev/sdb1), mount point, or folder into a writable game library path.
    // Prefers <mount>/wbfs when that directory exists.
    static QString resolveLibraryPath( const QString& selection );

    static bool isLibraryPath( const QString& path );

    static QWBFS::Model::DiscList listDiscs( const QString& libraryPath );

    static QWBFS::Partition::Status storageStatus( const QString& libraryPath );

    // Remove a .wbfs file and its empty parent game folder when applicable.
    static bool removeDiscFile( const QWBFS::Model::Disc& disc );
};

#endif // FOLDERLIBRARY_H
