/****************************************************************************
**
**      Created using Monkey Studio IDE v1.8.4.0 (1.8.4.0)
** Authors   : Filipe Azevedo aka Nox P@sNox <pasnox@gmail.com>
** Project   : QWBFS Manager
** FileName  : WorkerThread.cpp
** Date      : 2010-06-16T14:19:29
** License   : GPL2
** Home Page : https://github.com/pasnox/qwbfsmanager
** Comment   : QWBFS Manager is a cross platform WBFS manager developed using C++/Qt4.
** It's currently working fine under Windows (XP to Seven, 32 & 64Bits), Mac OS X (10.4.x to 10.6.x), Linux & unix like.
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This package is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program. If not, see <http://www.gnu.org/licenses/>.
**
** In addition, as a special exception, the copyright holders give permission
** to link this program with the OpenSSL project's "OpenSSL" library (or with
** modified versions of it that use the same license as the "OpenSSL"
** library), and distribute the linked executables. You must obey the GNU
** General Public License in all respects for all of the code used other than
** "OpenSSL".  If you modify file(s), you may extend this exception to your
** version of the file(s), but you are not obligated to do so. If you do not
** wish to do so, delete this exception statement from your version.
**
****************************************************************************/
#include "WorkerThread.h"

#include <FreshCore/pCoreUtils>

#include <QTime>
#include <QWidget>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMetaType>
#include <QDebug>
#include <QLocale>
#include <QElapsedTimer>

#if defined(Q_OS_UNIX)
#include <unistd.h>
#endif

WorkerThread::WorkerThread( QObject* parent )
    : QThread( parent )
    , mStop( false )
    , mJobCount( 0 )
    , mJobIndex( 0 )
{
    qRegisterMetaType<QWBFS::Model::Disc>( "QWBFS::Model::Disc" );
}

WorkerThread::~WorkerThread()
{
    if ( isRunning() ) {
        qWarning() << "Waiting thread to finish...";
        stop();
        wait();
    }
    
    //qWarning() << Q_FUNC_INFO;
}

WorkerThread::Task WorkerThread::task() const
{
    QMutexLocker locker( &const_cast<WorkerThread*>( this )->mMutex );
    return mWork.task;
}

bool WorkerThread::setWork( const WorkerThread::Work& work )
{
    if ( isRunning() ) {
        Q_ASSERT( 0 );
        return false;
    }
    
    mWork = work;
    mWork.window->setWindowTitle( taskToWindowTitle( mWork.task ) );
    
    start();
    
    return true;
}

QString WorkerThread::taskToWindowTitle( WorkerThread::Task task, bool indirect )
{
    return taskToLabel( task, indirect ).append( "..." );
}

QString WorkerThread::taskToLabel( WorkerThread::Task task, bool indirect )
{
    switch ( task ) {
        case WorkerThread::ExportISO:
            return indirect ? tr( "Indirect Export to ISO" ) : tr( "Export to ISO" );
        case WorkerThread::ExportWBFS:
            return indirect ? tr( "Indirect Export to WBFS" ) : tr( "Export to WBFS" );
        case WorkerThread::ImportISO:
            return indirect ? tr( "Indirect Import to ISO" ) : tr( "Import to ISO" );
        case WorkerThread::ImportWBFS:
            return indirect ? tr( "Indirect Import to WBFS" ) : tr( "Import to WBFS" );
        case WorkerThread::ConvertISO:
            return indirect ? tr( "Indirect Convert to ISO" ) : tr( "Convert to ISO" );
        case WorkerThread::ConvertWBFS:
            return indirect ? tr( "Indirect Convert to WBFS" ) : tr( "Convert to WBFS" );
        case WorkerThread::RenameAll:
            return tr( "Rename Disc" );
    }
    
    return QString();
}

void WorkerThread::stop()
{
    QMutexLocker locker( &mMutex );
    mStop = true;
    emit canceled();
}

void WorkerThread::run()
{
    WorkerThread::Work work;
    int count = 0;
    int id = 0;
    
    {
        QMutexLocker locker( &mMutex );
        mStop = false;
        work = mWork;
    }
    
    if ( work.task == WorkerThread::RenameAll ) {
        QDir dir( work.target );
        QStringList filters;
        
        dir.setFilter( QDir::Files );
        
        if ( work.task & WorkerThread::WBFS ) {
            filters << "*.wbfs";
        }
        
        if ( work.task & WorkerThread::ISO ) {
            filters << "*.iso";
        }
        
        foreach ( const QString& filePath, pCoreUtils::findFiles( dir, filters, true ) ) {
            work.discs << QWBFS::Model::Disc( filePath );
        }
    }
    
    count = work.discs.count();
    mJobCount = qMax( 1, count );
    mJobIndex = 0;

    emit globalProgressChanged( 0, mJobCount * 10000 );
    
    foreach ( QWBFS::Model::Disc disc, work.discs ) {
        switch ( work.task ) {
            case WorkerThread::RenameAll:
                renameDisc( work.task, disc, work.target, work.pattern, work.invalidChars );
                break;
            case WorkerThread::ExportISO:
            case WorkerThread::ExportWBFS:
            case WorkerThread::ImportISO:
            case WorkerThread::ImportWBFS:
            case WorkerThread::ConvertISO:
            case WorkerThread::ConvertWBFS: {
                switch ( QWBFS::Driver::fileType( disc.origin ) ) {
                    case QWBFS::Driver::WBFSFile:
                    case QWBFS::Driver::WBFSPartitionFile: {
                        switch ( QWBFS::Driver::fileType( work.target ) ) {
                            case QWBFS::Driver::WBFSPartitionFile: {
                                wbfsToWBFS( work.task, disc, work.target, false, work.invalidChars );
                                break;
                            }
                            case QWBFS::Driver::WBFSFile:
                            case QWBFS::Driver::ISOFile:
                            case QWBFS::Driver::UnknownFile: {
                                if ( work.task & WorkerThread::WBFS ) {
                                    wbfsToWBFS( work.task, disc, work.target, true, work.invalidChars, work.pattern );
                                }
                                else if ( work.task & WorkerThread::ISO ) {
                                    wbfsToISO( work.task, disc, work.target, work.invalidChars );
                                }
                                else {
                                    if ( work.task & WorkerThread::Import ) {
                                        disc.error = QWBFS::Driver::DiscAddFailed;
                                    }
                                    else if ( work.task & WorkerThread::Export ) {
                                        disc.error = QWBFS::Driver::DiscExtractFailed;
                                    }
                                    else if ( work.task & WorkerThread::Convert ) {
                                        disc.error = QWBFS::Driver::DiscConvertFailed;
                                    }
                                    else {
                                        disc.error = QWBFS::Driver::UnknownError;
                                    }
                                }
                                
                                break;
                            }
                        }
                        
                        break;
                    }
                    case QWBFS::Driver::ISOFile: {
                        switch ( QWBFS::Driver::fileType( work.target ) ) {
                            case QWBFS::Driver::WBFSPartitionFile: {
                                isoToWBFS( work.task, disc, work.target, false, work.invalidChars );
                                break;
                            }
                            case QWBFS::Driver::WBFSFile:
                            case QWBFS::Driver::ISOFile:
                            case QWBFS::Driver::UnknownFile: {
                                if ( work.task & WorkerThread::WBFS ) {
                                    isoToWBFS( work.task, disc, work.target, true, work.invalidChars, work.pattern );
                                }
                                else if ( work.task & WorkerThread::ISO ) {
                                    isoToISO( work.task, disc, work.target, work.invalidChars );
                                }
                                else {
                                    if ( work.task & WorkerThread::Import ) {
                                        disc.error = QWBFS::Driver::DiscAddFailed;
                                    }
                                    else if ( work.task & WorkerThread::Export ) {
                                        disc.error = QWBFS::Driver::DiscExtractFailed;
                                    }
                                    else if ( work.task & WorkerThread::Convert ) {
                                        disc.error = QWBFS::Driver::DiscConvertFailed;
                                    }
                                    else {
                                        disc.error = QWBFS::Driver::UnknownError;
                                    }
                                }
                                
                                break;
                            }
                        }
                        
                        break;
                    }
                    case QWBFS::Driver::UnknownFile:
                        disc.error = QWBFS::Driver::UnknownError;
                        break;
                }
            }
        }
        
        disc.state = disc.error == QWBFS::Driver::Ok ? QWBFS::Driver::Success : QWBFS::Driver::Failed;

        ++id;
        mJobIndex = id;
        emit globalProgressChanged( mJobIndex * 10000, mJobCount * 10000 );
        emit jobFinished( disc );
        
        {
            QMutexLocker locker( &mMutex );
            
            if ( mStop ) {
                break;
            }
        }
    }
    
    emit globalProgressChanged( mJobCount * 10000, mJobCount * 10000 );
}

void WorkerThread::connectDriver( QWBFS::Driver* driver )
{
    connect( driver, SIGNAL( currentProgressChanged( int, int, const QTime& ) ), this, SIGNAL( currentProgressChanged( int, int, const QTime& ) ) );
    connect( driver, SIGNAL( globalProgressChanged( int, int ) ), this, SIGNAL( globalProgressChanged( int, int ) ) );
}

bool WorkerThread::isStopRequested()
{
    QMutexLocker locker( &mMutex );
    return mStop;
}

QString WorkerThread::transferLogStamp() const
{
    return QTime::currentTime().toString( QStringLiteral( "hh:mm:ss" ) );
}

QString WorkerThread::formatTransferSpeed( qint64 bytesPerSecond ) const
{
    if ( bytesPerSecond <= 0 ) {
        return QStringLiteral( "—" );
    }
    return tr( "%1/s" ).arg( QLocale().formattedDataSize( bytesPerSecond ) );
}

void WorkerThread::emitStatusLog( const QString& text )
{
    emit message( text );
    emit log( QStringLiteral( "%1 Status: %2" ).arg( transferLogStamp(), text ) );
}

void WorkerThread::emitTransferLog( const QString& text )
{
    emit log( QStringLiteral( "%1 Transfer: %2" ).arg( transferLogStamp(), text ) );
}

namespace {

bool commitFileToDisk( QFile& file )
{
    if ( !file.flush() ) {
        return false;
    }

#if defined(Q_OS_UNIX)
    const int fd = file.handle();
    if ( fd != -1 && ::fdatasync( fd ) != 0 ) {
        return false;
    }
#endif

    return true;
}

} // namespace

void WorkerThread::emitFileProgress( qint64 value, qint64 maximum )
{
    // ProgressDialog/QProgressBar use int; files >2GiB overflow if passed raw.
    // Scale to 0..10000 so large USB copies still show a moving bar.
    const int scale = 10000;
    int scaledValue = 0;
    int scaledMaximum = scale;

    if ( maximum > 0 ) {
        scaledValue = int( ( value * qint64( scale ) ) / maximum );
        if ( scaledValue > scale ) {
            scaledValue = scale;
        }
        else if ( value > 0 && scaledValue == 0 ) {
            scaledValue = 1;
        }
    }
    else {
        scaledMaximum = 1;
    }

    emit currentProgressChanged( scaledValue, scaledMaximum, QWBFS::Driver::estimatedTimeForTask( value, maximum ) );

    // Overall bar: finished jobs + fraction of the current file copy.
    const int jobs = qMax( 1, mJobCount );
    const int globalMaximum = jobs * scale;
    const int globalValue = qMin( globalMaximum, mJobIndex * scale + scaledValue );
    emit globalProgressChanged( globalValue, globalMaximum );
}

void WorkerThread::copyFileWithProgress( const QString& sourcePath, const QString& targetPath, QWBFS::Model::Disc& result )
{
    QFile in( sourcePath );
    QFile out( targetPath );
    const QString sourceName = QFileInfo( sourcePath ).fileName();
    const QString targetName = QFileInfo( targetPath ).fileName();

    emitStatusLog( tr( "Starting file transfer..." ) );
    emit log( QStringLiteral( "%1   Local:  %2" ).arg( transferLogStamp(), sourcePath ) );
    emit log( QStringLiteral( "%1   Remote: %2" ).arg( transferLogStamp(), targetPath ) );

    if ( !in.open( QIODevice::ReadOnly ) ) {
        emitStatusLog( tr( "Could not read local file '%1'." ).arg( sourceName ) );
        result.error = QWBFS::Driver::DiscReadFailed;
        return;
    }

    if ( !out.open( QIODevice::WriteOnly ) ) {
        emitStatusLog( tr( "Could not write remote file '%1'." ).arg( targetName ) );
        result.error = QWBFS::Driver::DiscWriteFailed;
        return;
    }

    const qint64 totalSize = in.size();
    emit log( QStringLiteral( "%1   Size:   %2" )
        .arg( transferLogStamp(), QLocale().formattedDataSize( totalSize ) ) );
    emitStatusLog( tr( "Transferring '%1' → '%2'..." ).arg( sourceName, targetName ) );
    emitStatusLog( tr( "Writing in chunks and syncing to USB (progress continues during flush)..." ) );

    // Smaller chunks: FAT/USB writes can block for a long time; check cancel more often.
    const qint64 bufferSize = 1024 * 1024; // 1 MB
    // Commit to the USB periodically so progress stays honest without flooding the log.
    const qint64 commitEvery = 32 * 1024 * 1024; // 32 MiB
    QByteArray buffer;
    buffer.resize( int( bufferSize ) );
    qint64 totalRead = 0;
    qint64 sinceCommit = 0;
    qint64 committedBytes = 0;
    qint64 lastLoggedBytes = 0;
    QElapsedTimer transferTimer;
    QElapsedTimer logTimer;
    transferTimer.start();
    logTimer.start();

    // Keep a tiny headroom so 100% means "fully synced", not only "buffered".
    const qint64 progressMax = totalSize > 0 ? totalSize + 1 : 1;
    emitFileProgress( totalRead, progressMax );

    auto logTransferLine = [&]( qint64 bytesForSpeed, bool forceDone ) {
        const qint64 elapsedMs = qMax<qint64>( 1, transferTimer.elapsed() );
        const qint64 bytesPerSecond = ( bytesForSpeed * 1000 ) / elapsedMs;
        const int percent = totalSize > 0 ? int( ( committedBytes * 100 ) / totalSize ) : 0;
        QString eta = QStringLiteral( "—" );
        if ( bytesPerSecond > 0 && committedBytes < totalSize ) {
            const qint64 remainingSec = ( totalSize - committedBytes ) / bytesPerSecond;
            eta = QTime( 0, 0 ).addSecs( int( qMin<qint64>( remainingSec, 23 * 3600 + 3599 ) ) ).toString( QStringLiteral( "hh:mm:ss" ) );
        }
        else if ( forceDone || committedBytes >= totalSize ) {
            eta = QStringLiteral( "00:00:00" );
        }

        emitTransferLog( tr( "%1 → %2 | %3 / %4 (%5%) | %6 | ETA %7" )
            .arg( sourceName, targetName )
            .arg( QLocale().formattedDataSize( committedBytes ) )
            .arg( QLocale().formattedDataSize( totalSize ) )
            .arg( percent )
            .arg( formatTransferSpeed( bytesPerSecond ) )
            .arg( eta ) );
        lastLoggedBytes = totalRead;
        logTimer.restart();
        Q_UNUSED( forceDone );
    };

    while ( !in.atEnd() ) {
        if ( isStopRequested() ) {
            emit syncingChanged( false );
            out.close();
            out.remove();
            emitStatusLog( tr( "File transfer aborted by user." ) );
            result.error = QWBFS::Driver::OperationCanceled;
            return;
        }

        const qint64 read = in.read( buffer.data(), bufferSize );
        if ( read == -1 ) {
            emit syncingChanged( false );
            out.close();
            out.remove();
            emitStatusLog( tr( "File transfer failed: read error." ) );
            result.error = QWBFS::Driver::DiscReadFailed;
            return;
        }

        const qint64 written = out.write( buffer.constData(), read );
        if ( written != read ) {
            emit syncingChanged( false );
            out.close();
            out.remove();
            emitStatusLog( tr( "File transfer failed: write error." ) );
            result.error = QWBFS::Driver::DiscWriteFailed;
            return;
        }

        totalRead += read;
        sinceCommit += read;
        // Progress bar follows buffered writes; log percent follows USB-committed bytes.
        emitFileProgress( totalRead, progressMax );

        if ( sinceCommit >= commitEvery ) {
            emit syncingChanged( true );
            emit message( tr( "Writing '%1' to USB... %2 / %3" )
                .arg( targetName )
                .arg( QLocale().formattedDataSize( totalRead ) )
                .arg( QLocale().formattedDataSize( totalSize ) ) );

            if ( !commitFileToDisk( out ) ) {
                emit syncingChanged( false );
                out.close();
                out.remove();
                emitStatusLog( tr( "File transfer failed: USB sync error." ) );
                result.error = QWBFS::Driver::DiscWriteFailed;
                return;
            }

            committedBytes = totalRead;
            sinceCommit = 0;
            emit syncingChanged( false );
            logTransferLine( committedBytes, false );
        }
    }

    in.close();

    // Final sync for the remaining cached data. Heartbeat logs continue via syncingChanged.
    emit syncingChanged( true );
    emitStatusLog( tr( "Final sync of '%1' to USB (%2 left in cache)..." )
        .arg( targetName )
        .arg( QLocale().formattedDataSize( sinceCommit > 0 ? sinceCommit : 0 ) ) );
    emitFileProgress( totalSize, progressMax );

    if ( !commitFileToDisk( out ) ) {
        emit syncingChanged( false );
        out.close();
        out.remove();
        emitStatusLog( tr( "File transfer failed: flush error." ) );
        result.error = QWBFS::Driver::DiscWriteFailed;
        return;
    }

    out.close();
    emit syncingChanged( false );
    committedBytes = totalSize;

    const qint64 elapsedMs = qMax<qint64>( 1, transferTimer.elapsed() );
    const qint64 avgSpeed = ( totalSize * 1000 ) / elapsedMs;
    emitFileProgress( progressMax, progressMax );
    emitTransferLog( tr( "%1 → %2 | %3 / %4 (100%) | avg %5 | USB synced | done" )
        .arg( sourceName, targetName )
        .arg( QLocale().formattedDataSize( totalSize ) )
        .arg( QLocale().formattedDataSize( totalSize ) )
        .arg( formatTransferSpeed( avgSpeed ) ) );
    emitStatusLog( tr( "File transfer successful." ) );
    result.error = QWBFS::Driver::Ok;
}

int WorkerThread::allocateFileWithProgress( const QString& filePath, qint64 size )
{
    QFile file( filePath );
    if ( file.exists() ) {
        return QWBFS::Driver::DiscFound;
    }

    if ( size <= 0 ) {
        return QWBFS::Driver::allocateFile( filePath, size );
    }

    if ( !file.open( QIODevice::WriteOnly ) ) {
        return QWBFS::Driver::DiscWriteFailed;
    }

    // Grow in chunks so cancel can interrupt long FAT/USB allocations.
    // Avoid a single QFile::resize(4GB) which blocks cancel for a long time.
    const qint64 chunkSize = 1024 * 1024; // 1 MB
    QByteArray zeros( int( chunkSize ), '\0' );
    qint64 written = 0;

    emit message( tr( "Allocating space for '%1'..." ).arg( QFileInfo( filePath ).fileName() ) );

    while ( written < size ) {
        if ( isStopRequested() ) {
            file.close();
            file.remove();
            return QWBFS::Driver::OperationCanceled;
        }

        const qint64 toWrite = qMin( chunkSize, size - written );
        if ( file.write( zeros.constData(), toWrite ) != toWrite ) {
            file.close();
            file.remove();
            return QWBFS::Driver::DiscWriteFailed;
        }

        written += toWrite;
        emitFileProgress( written, size );
    }

    file.close();
    return QWBFS::Driver::Ok;
}

QString WorkerThread::trimmedTargetPath( const QString& targetDir, const QWBFS::Model::Disc& source, const QString& invalidChars, const QString& pattern, const QString& suffix ) const
{
    QString relative;

    if ( !pattern.isEmpty() ) {
        relative = QString( pattern )
            .replace( "%title", QWBFS::Model::Disc::cleanupGameTitle( source.title, invalidChars ), Qt::CaseInsensitive )
            .replace( "%id", source.id.toUpper(), Qt::CaseInsensitive )
            .replace( "%suffix", suffix, Qt::CaseInsensitive );
    }
    else {
        relative = QString( "%1.%2" ).arg( source.baseName( invalidChars ) ).arg( suffix );
    }

    const QString target = QDir::cleanPath( QString( "%1/%2" ).arg( targetDir ).arg( relative ) );
    QDir().mkpath( QFileInfo( target ).absolutePath() );
    return target;
}

void WorkerThread::renameDisc( WorkerThread::Task task, QWBFS::Model::Disc& source, const QString& target, const QString& pattern, const QString& invalidChars )
{
    if ( !source.isValid() ) {
        source.error = QWBFS::Driver::InvalidDisc;
        return;
    }
    
    /*
        %title = Game Title
        %id = Game ID
        %suffix = File Suffix
    */
    QString filePath = QString( "%1/%2" )
        .arg( target )
        .arg( pattern )
        .replace( "%title", QWBFS::Model::Disc::cleanupGameTitle( source.title, invalidChars ), Qt::CaseInsensitive )
        .replace( "%id", source.id.toUpper(), Qt::CaseInsensitive )
        .replace( "%suffix", QFileInfo( source.origin ).suffix(), Qt::CaseInsensitive )
        ;
    
    emit message( QString( "%1 '%2'..." ).arg( taskToLabel( task ) ).arg( source.baseName( invalidChars ) ) );
    emit currentProgressChanged( 0, 1, QTime( 0, 0, 0 ) );
    
    if ( source.origin != filePath ) {
        source.error = QDir( target ).mkpath( QFileInfo( filePath ).absolutePath() ) ? QWBFS::Driver::Ok : QWBFS::Driver::UnknownError;
        
        if ( !source.hasError() ) {
            const bool exists = QFile::exists( filePath );
            
            if ( exists ) {
                const QFileInfo file( filePath );
                const QString path = file.absolutePath();
                const QString fileName = file.fileName();
                filePath = QString( "%1/Existing/%2.%3" )
                    .arg( path )
                    .arg( fileName )
                    .arg( QDateTime::currentDateTime().toString( "yyyy-MM-dd hh'h'mm" ) );
                
                QDir( path ).mkpath( "Existing" );
            }
            
            source.error = QFile::rename( source.origin, filePath ) ? QWBFS::Driver::Ok : QWBFS::Driver::DiscRenameFailed;
            
            if ( exists ) {
                source.error = QWBFS::Driver::DiscFound;
            }
            
            if ( !source.hasError() ) {
                const QString sourcePath = QFileInfo( source.origin ).absolutePath();
                
                if ( pCoreUtils::isEmptyDirectory( sourcePath ) ) {
                    QDir( sourcePath ).rmdir( sourcePath );
                }
            }
        }
    }
    else {
        source.error = QWBFS::Driver::Ok;
    }
    
    emit currentProgressChanged( 1, 1, QTime( 0, 0, 0 ) );
}

void WorkerThread::isoToWBFS( WorkerThread::Task task, QWBFS::Model::Disc& source, const QString& _target, bool trimWBFS, const QString& invalidChars, const QString& pattern )
{
    if ( !source.isValid() ) {
        source.error = QWBFS::Driver::InvalidDisc;
        return;
    }
    
    const QFileInfo file( _target );
    QString target = _target;
    bool created = false;
    QWBFS::Partition::Handle handle;
    
    if ( trimWBFS && file.isDir() ) {
        target = trimmedTargetPath( _target, source, invalidChars, pattern, QStringLiteral( "wbfs" ) );
    }
    
    if ( trimWBFS ) {
        
        if ( QFile::exists( target ) ) {
            QWBFS::Model::Disc existing;
            if ( QWBFS::Driver::wbfsFileInfo( target, existing ) != QWBFS::Driver::Ok ) {
                QFile::remove( target );
            }
            else {
                source.error = QWBFS::Driver::DiscFound;
                return;
            }
        }
        
        emit message( tr( "Initializing WBFS disc '%1'..." ).arg( source.baseName( invalidChars ) ) );
        const qint64 fatMax = ( Q_INT64_C( 4 ) * 1024 * 1024 * 1024 ) - 1;
        qint64 allocSize = source.size > 0 ? qMin<qint64>( source.size + ( 16 * 1024 * 1024 ), fatMax ) : -1;
        source.error = allocateFileWithProgress( target, allocSize );
        
        if ( source.hasError() ) {
            QFile::remove( target );
            return;
        }
        
        QWBFS::Partition::Properties properties( target );
        properties.reset = true;
        
        emit message( tr( "Formating WBFS disc '%1'..." ).arg( source.baseName( invalidChars ) ) );
        
        handle = QWBFS::Partition::Handle( properties );
    }
    else {
        handle = QWBFS::Driver::getHandle( target, &created );
    }
    
    if ( !handle.isValid() ) {
        if ( created ) {
            QWBFS::Driver::closeHandle( handle );
        }
        
        if ( trimWBFS ) {
            QFile::remove( target );
        }
        
        source.error = QWBFS::Driver::PartitionNotOpened;
        return;
    }
    
    QWBFS::Driver driver( 0, handle );
    connectDriver( &driver );
    
    emit message( QString( "%1 '%2'..." ).arg( taskToLabel( task ) ).arg( source.baseName( invalidChars ) ) );
    source.error = driver.addDiscImage( source.origin );
    
    if ( source.hasError() ) {
        if ( created ) {
            QWBFS::Driver::closeHandle( handle );
        }
        
        if ( trimWBFS ) {
            QFile::remove( target );
        }
        
        return;
    }
    
    if ( trimWBFS ) {
        source.error = driver.trim();
    }
    
    if ( created ) {
        QWBFS::Driver::closeHandle( handle );
    }
}

void WorkerThread::wbfsToISO( WorkerThread::Task task, QWBFS::Model::Disc& source, const QString& _target, const QString& invalidChars )
{
    if ( !source.isValid() ) {
        source.error = QWBFS::Driver::InvalidDisc;
        return;
    }
    
    QFileInfo file( _target );
    QString target = _target;
    
    if ( file.isDir() ) {
        target = QDir::cleanPath( target.append( QString( "/%1.iso" ).arg( source.baseName( invalidChars ) ) ) );
    }
    
    if ( QFile::exists( target ) ) {
        source.error = QWBFS::Driver::DiscFound;
        return;
    }
    
    file.setFile( target );
    
    bool created = false;
    QWBFS::Partition::Handle handle = QWBFS::Driver::getHandle( source.origin, &created );
    
    // check handle validity
    if ( !handle.isValid() ) {
        if ( created ) {
            QWBFS::Driver::closeHandle( handle );
        }
        
        source.error = QWBFS::Driver::PartitionNotOpened;
        return;
    }
    
    QWBFS::Driver driver( 0, handle );
    connectDriver( &driver );
    
    emit message( QString( "%1 '%2'..." ).arg( taskToLabel( task ) ).arg( source.baseName( invalidChars ) ) );
    source.error = driver.extractDisc( source.id, file.absolutePath(), file.fileName() );
    
    if ( source.hasError() ) {
        QFile::remove( target );
    }
    
    if ( created ) {
        QWBFS::Driver::closeHandle( handle );
    }
}

void WorkerThread::isoToISO( WorkerThread::Task task, QWBFS::Model::Disc& source, const QString& _target, const QString& invalidChars )
{
    if ( !source.isValid() ) {
        source.error = QWBFS::Driver::InvalidDisc;
        return;
    }
    
    const QFileInfo file( _target );
    QString target = _target;
    
    if ( file.isDir() ) {
        target = QDir::cleanPath( target.append( QString( "/%1.iso" ).arg( source.baseName( invalidChars ) ) ) );
    }
    
    if ( QFile::exists( target ) ) {
        source.error = QWBFS::Driver::DiscFound;
        return;
    }
    
    emit message( QString( "%1 '%2'..." ).arg( taskToLabel( task ) ).arg( source.baseName( invalidChars ) ) );
    
    // copying the file chunk by chunk instead of using QFile::copy() to be able to see progression
    QFile in( source.origin );
    QFile out( target );
    
    if ( !in.open( QIODevice::ReadOnly ) ) {
        source.error = QWBFS::Driver::DiscReadFailed;
        return;
    }
    
    if ( !out.open( QIODevice::WriteOnly ) ) {
        source.error = QWBFS::Driver::DiscWriteFailed;
        return;
    }
    
    const qint64 bufferSize = 1024 * 1024; // 1 MB
    QByteArray buffer;
    buffer.resize( int( bufferSize ) );
    qint64 totalRead = 0;
    const qint64 totalSize = in.size();

    emitFileProgress( totalRead, totalSize );

    while ( !in.atEnd() ) {
        if ( isStopRequested() ) {
            out.close();
            out.remove();
            source.error = QWBFS::Driver::OperationCanceled;
            return;
        }

        const qint64 read = in.read( buffer.data(), bufferSize );

        if ( read == -1 ) {
            out.close();
            out.remove();

            QFile::remove( target );
            source.error = QWBFS::Driver::DiscReadFailed;
            return;
        }

        const qint64 write = out.write( buffer.constData(), read );

        if ( write != read ) {
            out.close();
            out.remove();

            QFile::remove( target );
            source.error = QWBFS::Driver::DiscWriteFailed;
            return;
        }

        totalRead += read;
        emitFileProgress( totalRead, totalSize );
    }

    in.close();
    out.close();
}

void WorkerThread::wbfsToWBFS( WorkerThread::Task task, QWBFS::Model::Disc& source, const QString& _target, bool trimWBFS, const QString& invalidChars, const QString& pattern )
{
    if ( !source.isValid() ) {
        source.error = QWBFS::Driver::InvalidDisc;
        return;
    }
    
    const QFileInfo file( _target );
    QString target = _target;

    // Standalone .wbfs -> folder library: copy the file directly.
    // Avoids preallocating a 4GB container on FAT (very slow and not cancelable).
    if ( trimWBFS && file.isDir()
        && QWBFS::Driver::fileType( source.origin ) == QWBFS::Driver::WBFSFile ) {
        target = trimmedTargetPath( _target, source, invalidChars, pattern, QStringLiteral( "wbfs" ) );

        if ( QFile::exists( target ) ) {
            QWBFS::Model::Disc existing;
            if ( QWBFS::Driver::wbfsFileInfo( target, existing ) != QWBFS::Driver::Ok ) {
                QFile::remove( target );
            }
            else {
                source.error = QWBFS::Driver::DiscFound;
                return;
            }
        }

        emitStatusLog( tr( "Copying '%1'..." ).arg( source.baseName( invalidChars ) ) );
        copyFileWithProgress( source.origin, target, source );

        if ( source.hasError() && source.error != QWBFS::Driver::DiscFound ) {
            QFile::remove( target );
            const QString parentPath = QFileInfo( target ).absolutePath();
            if ( pCoreUtils::isEmptyDirectory( parentPath ) ) {
                QDir( parentPath ).rmdir( parentPath );
            }
        }

        return;
    }

    bool sourceCreated = false;
    QWBFS::Partition::Handle sourceHandle = QWBFS::Driver::getHandle( source.origin, &sourceCreated );
    bool targetCreated = false;
    QWBFS::Partition::Handle targetHandle;
    
    if ( !sourceHandle.isValid() ) {
        if ( sourceCreated ) {
            QWBFS::Driver::closeHandle( sourceHandle );
        }
        
        source.error = QWBFS::Driver::PartitionNotOpened;
        return;
    }
    
    if ( trimWBFS && file.isDir() ) {
        target = trimmedTargetPath( _target, source, invalidChars, pattern, QStringLiteral( "wbfs" ) );
    }
    
    if ( trimWBFS ) {
        
        if ( QFile::exists( target ) ) {
            if ( sourceCreated ) {
                QWBFS::Driver::closeHandle( sourceHandle );
            }
            
            source.error = QWBFS::Driver::DiscFound;
            return;
        }
        
        emit message( tr( "Initializing WBFS disc '%1'..." ).arg( source.baseName( invalidChars ) ) );
        const qint64 fatMax = ( Q_INT64_C( 4 ) * 1024 * 1024 * 1024 ) - 1;
        qint64 allocSize = source.size > 0 ? qMin<qint64>( source.size + ( 16 * 1024 * 1024 ), fatMax ) : -1;
        source.error = allocateFileWithProgress( target, allocSize );
        
        if ( source.hasError() ) {
            if ( sourceCreated ) {
                QWBFS::Driver::closeHandle( sourceHandle );
            }
            
            QFile::remove( target );
            return;
        }
        
        QWBFS::Partition::Properties properties( target );
        properties.reset = true;
        
        emit message( tr( "Formating WBFS disc '%1'..." ).arg( source.baseName( invalidChars ) ) );
        
        targetHandle = QWBFS::Partition::Handle( properties );
    }
    else {
        targetHandle = QWBFS::Driver::getHandle( target, &targetCreated );
    }
    
    if ( !targetHandle.isValid() ) {
        if ( sourceCreated ) {
            QWBFS::Driver::closeHandle( sourceHandle );
        }
        
        if ( targetCreated ) {
            QWBFS::Driver::closeHandle( targetHandle );
        }
        
        if ( trimWBFS ) {
            QFile::remove( target );
        }
        
        source.error = QWBFS::Driver::PartitionNotOpened;
        return;
    }
    
    QWBFS::Driver targetDriver( 0, targetHandle );
    connectDriver( &targetDriver );
    
    // direct drive2drive
    if ( targetDriver.canDrive2Drive( sourceHandle ) == QWBFS::Driver::Ok ) {
        emit message( QString( "%1 '%2'..." ).arg( taskToLabel( task ) ).arg( source.baseName( invalidChars ) ) );
        
        source.error = targetDriver.addDisc( source.id, sourceHandle );
    }
    // indirect drive2drive
    else {
        emit message( QString( "%1 '%2'..." ).arg( taskToLabel( task, true ) ).arg( source.baseName( invalidChars ) ) );
        
        const QFileInfo tmpFile( QString( "%1/%2.iso" ).arg( QDir::tempPath() ).arg( source.baseName( invalidChars ) ) );
        QWBFS::Driver sourceDriver( 0, sourceHandle );
        connectDriver( &sourceDriver );
        
        if ( targetDriver.hasDisc( source.id ) == QWBFS::Driver::DiscNotFound ) {
            source.error = sourceDriver.extractDisc( source.id, tmpFile.absolutePath(), tmpFile.fileName() );
            
            if ( !source.hasError() ) {
                source.error = targetDriver.addDiscImage( tmpFile.absoluteFilePath() );
            }
            
            QFile::remove( tmpFile.absoluteFilePath() );
        }
        else {
            source.error = QWBFS::Driver::DiscFound;
        }
    }
    
    if ( !source.hasError() && trimWBFS ) {
        source.error = targetDriver.trim();
    }
    
    if ( sourceCreated ) {
        QWBFS::Driver::closeHandle( sourceHandle );
    }
    
    if ( targetCreated ) {
        QWBFS::Driver::closeHandle( targetHandle );
    }
    
    if ( source.hasError() && trimWBFS ) {
        QFile::remove( target );
    }
}
