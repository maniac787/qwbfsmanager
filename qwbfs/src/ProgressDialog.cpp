/****************************************************************************
**
**      Created using Monkey Studio IDE v1.8.4.0 (1.8.4.0)
** Authors   : Filipe Azevedo aka Nox P@sNox <pasnox@gmail.com>
** Project   : QWBFS Manager
** FileName  : ProgressDialog.cpp
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
#include "ProgressDialog.h"

#include <QPushButton>
#include <QTimer>
#include <QScreen>
#include <QDebug>
#include <QFont>
#include <QTime>
#include <QTextCursor>

ProgressDialog::ProgressDialog( QWidget* parent )
    : QDialog( parent )
    , mSyncHeartbeat( 0 )
    , mSyncSeconds( 0 )
{
    setupUi( this );
    setAttribute( Qt::WA_DeleteOnClose );
    cbDetails->setChecked( true );
    dbbButtons->button( QDialogButtonBox::Ok )->setEnabled( false );
    dbbButtons->button( QDialogButtonBox::Cancel )->setEnabled( false );

    QFont logFont = pteErrors->font();
    logFont.setFamily( QStringLiteral( "monospace" ) );
    logFont.setStyleHint( QFont::Monospace );
    pteErrors->setFont( logFont );
    pteErrors->setPlaceholderText( tr( "Transfer message log..." ) );

    mSyncHeartbeat = new QTimer( this );
    mSyncHeartbeat->setInterval( 1000 );
    connect( mSyncHeartbeat, SIGNAL( timeout() ), this, SLOT( syncHeartbeatTimeout() ) );
    
    mThread = new WorkerThread( this );
    doConnections();
    localeChanged();
}

ProgressDialog::~ProgressDialog()
{
    //qWarning() << Q_FUNC_INFO;
}

bool ProgressDialog::event( QEvent* event )
{
    switch ( event->type() ) {
        case QEvent::LocaleChange:
            localeChanged();
            break;
        default:
            break;
    }
    
    return QDialog::event( event );
}

void ProgressDialog::setWork( const WorkerThread::Work& work )
{
    open();
    
    if ( !mThread->setWork( work ) ) {
        deleteLater();
    }
}

void ProgressDialog::done( int r )
{
    if ( mThread->isRunning() ) {
        return;
    }
    
    QDialog::done( r );
}

void ProgressDialog::closeEvent( QCloseEvent* event )
{
    if ( mThread->isRunning() ) {
        event->ignore();
        return;
    }
    
    QDialog::closeEvent( event );
}

void ProgressDialog::localeChanged()
{
    retranslateUi( this );
}

void ProgressDialog::doConnections()
{
    connect( dbbButtons->button( QDialogButtonBox::Ok ), SIGNAL( clicked() ), this, SLOT( close() ) );
    connect( dbbButtons->button( QDialogButtonBox::Cancel ), SIGNAL( clicked() ), mThread, SLOT( stop() ) );
    connect( mThread, SIGNAL( started() ), this, SLOT( thread_started() ) );
    connect( mThread, SIGNAL( message( const QString& ) ), this, SLOT( thread_message( const QString& ) ) );
    connect( mThread, SIGNAL( log( const QString& ) ), this, SLOT( thread_log( const QString& ) ) );
    connect( mThread, SIGNAL( jobFinished( const QWBFS::Model::Disc& ) ), this, SLOT( thread_jobFinished( const QWBFS::Model::Disc& ) ) );
    connect( mThread, SIGNAL( currentProgressChanged( int, int, const QTime& ) ), this, SLOT( thread_currentProgressChanged( int, int, const QTime& ) ) );
    connect( mThread, SIGNAL( globalProgressChanged( int, int ) ), this, SLOT( thread_globalProgressChanged( int, int ) ) );
    connect( mThread, SIGNAL( canceled() ), this, SLOT( thread_canceled() ) );
    connect( mThread, SIGNAL( finished() ), this, SLOT( thread_finished() ) );
    connect( mThread, SIGNAL( syncingChanged( bool ) ), this, SLOT( thread_syncingChanged( bool ) ) );
}

void ProgressDialog::thread_started()
{
    mElapsed.restart();
    dbbButtons->button( QDialogButtonBox::Cancel )->setEnabled( true );
}

void ProgressDialog::thread_message( const QString& text )
{
    lCurrentInformations->setText( text );
    // Structured Status/Transfer lines are emitted via log(); keep the label for live status only.
}

void ProgressDialog::thread_log( const QString& text )
{
    pteErrors->appendPlainText( text );
    QTextCursor cursor = pteErrors->textCursor();
    cursor.movePosition( QTextCursor::End );
    pteErrors->setTextCursor( cursor );
    
    if ( !cbDetails->isChecked() ) {
        cbDetails->toggle();
    }
}

void ProgressDialog::thread_jobFinished( const QWBFS::Model::Disc& disc )
{
    const QString stamp = QTime::currentTime().toString( QStringLiteral( "hh:mm:ss" ) );
    const QString text = QStringLiteral( "%1 Status: %2 '%3' — %4 (%5)" )
        .arg( stamp )
        .arg( WorkerThread::taskToLabel( mThread->task() ) )
        .arg( disc.baseName() )
        .arg( QWBFS::Driver::stateToString( QWBFS::Driver::State( disc.state ) ) )
        .arg( QWBFS::Driver::errorToString( QWBFS::Driver::Error( disc.error ) ) );
    
    pteErrors->appendPlainText( text );
    
    if ( !cbDetails->isChecked() && disc.hasError() ) {
        cbDetails->toggle();
    }
    
    emit jobFinished( disc );
}

void ProgressDialog::thread_currentProgressChanged( int value, int maximum, const QTime& remaining )
{
    pbCurrent->setMaximum( qMax( 1, maximum ) );
    pbCurrent->setValue( qBound( 0, value, pbCurrent->maximum() ) );
    const int percent = maximum > 0 ? int( ( qint64( value ) * 100 ) / maximum ) : 0;
    lCurrentRemaining->setText( tr( "Time remaining: %1 (%2%)" )
        .arg( remaining.toString() )
        .arg( percent ) );
}

void ProgressDialog::thread_globalProgressChanged( int value, int maximum )
{
    pbGlobal->setMaximum( qMax( 1, maximum ) );
    pbGlobal->setValue( qBound( 0, value, pbGlobal->maximum() ) );
    const int percent = maximum > 0 ? int( ( qint64( value ) * 100 ) / maximum ) : 0;
    if ( mThread->isRunning() ) {
        lGlobalInformations->setText( tr( "Overall progress: %1%" ).arg( percent ) );
    }
}

void ProgressDialog::thread_canceled()
{
    lGlobalInformations->setText( tr( "Cancel requested, the process will stop after the current operation." ) );
}

void ProgressDialog::thread_syncingChanged( bool active )
{
    if ( active ) {
        if ( !mSyncHeartbeat->isActive() ) {
            mSyncSeconds = 0;
            mSyncElapsed.restart();
            mSyncHeartbeat->start();
        }
    }
    else {
        mSyncHeartbeat->stop();
        mSyncSeconds = 0;
    }
}

void ProgressDialog::syncHeartbeatTimeout()
{
    ++mSyncSeconds;
    const QString elapsed = QTime( 0, 0 ).addSecs( mSyncSeconds ).toString( QStringLiteral( "mm:ss" ) );
    lCurrentInformations->setText( tr( "Writing to USB... %1 elapsed" ).arg( elapsed ) );

    // Keep the label live every second, but only append to the log on longer stalls
    // so Transfer lines are not drowned by "Still writing..." spam.
    if ( mSyncSeconds >= 5 && ( mSyncSeconds % 5 == 0 ) ) {
        const QString stamp = QTime::currentTime().toString( QStringLiteral( "hh:mm:ss" ) );
        pteErrors->appendPlainText( QStringLiteral( "%1 Status: Still writing to USB... %2 elapsed" )
            .arg( stamp, elapsed ) );
        QTextCursor cursor = pteErrors->textCursor();
        cursor.movePosition( QTextCursor::End );
        pteErrors->setTextCursor( cursor );
    }
}

void ProgressDialog::thread_finished()
{
    mSyncHeartbeat->stop();

    QTime time( 0, 0, 0, 0 );
    time = time.addMSecs( mElapsed.elapsed() );
    
    lCurrentInformations->setText( tr( "Tasks finished." ) );
    lCurrentRemaining->clear();
    lGlobalInformations->setText( tr( "Completed in %1" ).arg( time.toString() ) );
    pbGlobal->setMaximum( qMax( 1, pbGlobal->maximum() ) );
    pbGlobal->setValue( pbGlobal->maximum() );

    QPushButton* ok = dbbButtons->button( QDialogButtonBox::Ok );
    QPushButton* cancel = dbbButtons->button( QDialogButtonBox::Cancel );
    if ( ok ) {
        ok->setEnabled( true );
        ok->setDefault( true );
        ok->setFocus();
    }
    if ( cancel ) {
        cancel->setEnabled( false );
    }
    pteErrors->appendPlainText( tr( "Done. You can click Accept." ) );

    // Refresh partition list after the dialog can accept input (reload can block on USB).
    QTimer::singleShot( 0, this, [this]() { emit finished(); } );
}

void ProgressDialog::on_cbDetails_toggled()
{
    QTimer::singleShot( 0, this, SLOT( updateSpace() ) );
}

void ProgressDialog::updateSpace()
{
    QWidget* widget = parentWidget();
    QRect rect = geometry();
    
    if ( !cbDetails->isChecked() ) {
        rect.setHeight( minimumSizeHint().height() );
    }

    if ( !widget ) {
        QScreen* screen = QApplication::primaryScreen();
        if ( screen ) {
            rect.moveCenter( screen->availableGeometry().center() );
        }
    } else {
        widget = widget->window();
        rect.moveCenter( widget->mapToGlobal( widget->rect().center() ) );
#if defined( Q_OS_MAC )
        rect.moveTop( widget->mapToGlobal( widget->rect().topLeft() ).y() );
#endif
    }

    resize( rect.size() );
    move( rect.topLeft() );
}
