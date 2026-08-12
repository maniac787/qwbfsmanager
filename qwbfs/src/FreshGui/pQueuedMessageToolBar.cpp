#include "pQueuedMessageToolBar.h"

#include <QBrush>
#include <QColor>
#include <QHBoxLayout>
#include <QLabel>
#include <QPalette>
#include <QToolButton>

namespace {

const QColor kMessageBackground( 255, 236, 160 );
const QColor kMessageForeground( 32, 32, 32 );
const QColor kMessageLink( 10, 70, 160 );

void applyMessageLabelColors( QLabel* label )
{
    QPalette palette = label->palette();
    palette.setColor( QPalette::WindowText, kMessageForeground );
    palette.setColor( QPalette::Text, kMessageForeground );
    palette.setColor( QPalette::Link, kMessageLink );
    palette.setColor( QPalette::LinkVisited, kMessageLink.darker( 115 ) );
    label->setPalette( palette );
    label->setStyleSheet(
        QStringLiteral(
            "QLabel {"
            "  color: rgb(%1, %2, %3);"
            "  background: transparent;"
            "}"
            "QLabel a {"
            "  color: rgb(%4, %5, %6);"
            "}" )
            .arg( kMessageForeground.red() )
            .arg( kMessageForeground.green() )
            .arg( kMessageForeground.blue() )
            .arg( kMessageLink.red() )
            .arg( kMessageLink.green() )
            .arg( kMessageLink.blue() ) );
}

} // namespace

pQueuedMessageWidget::pQueuedMessageWidget( QWidget* parent )
    : QWidget( parent )
{
    QHBoxLayout* layout = new QHBoxLayout( this );
    layout->setContentsMargins( 0, 0, 0, 0 );
    layout->setSpacing( 8 );

    m_label = new QLabel( this );
    m_label->setWordWrap( true );
    m_label->setTextInteractionFlags( Qt::TextBrowserInteraction );
    m_label->setOpenExternalLinks( true );
    applyMessageLabelColors( m_label );
    layout->addWidget( m_label, 1 );

    m_closeButton = new QToolButton( this );
    m_closeButton->setText( QStringLiteral( "×" ) );
    m_closeButton->setAutoRaise( true );
    m_closeButton->setToolTip( tr( "Close" ) );
    m_closeButton->setCursor( Qt::PointingHandCursor );
    m_closeButton->setStyleSheet(
        QStringLiteral(
            "QToolButton {"
            "  color: rgb(%1, %2, %3);"
            "  background: transparent;"
            "  border: none;"
            "  font-size: 18px;"
            "  font-weight: bold;"
            "  padding: 0 4px;"
            "}"
            "QToolButton:hover {"
            "  color: rgb(0, 0, 0);"
            "}" )
            .arg( kMessageForeground.red() )
            .arg( kMessageForeground.green() )
            .arg( kMessageForeground.blue() ) );
    layout->addWidget( m_closeButton, 0, Qt::AlignTop );

    connect( m_closeButton, &QToolButton::clicked, this, &pQueuedMessageWidget::closeRequested );
}

void pQueuedMessageWidget::appendMessage( const QString& text )
{
    m_messages << text;
    applyMessageLabelColors( m_label );
    m_label->setText( text );
}

void pQueuedMessageWidget::clearMessages()
{
    m_messages.clear();
    m_label->clear();
}

int pQueuedMessageWidget::pendingMessageCount() const
{
    return m_messages.size();
}

void pQueuedMessageWidget::currentMessageInformations( int index, QBrush* brush, QString* text ) const
{
    if ( brush ) {
        *brush = QBrush( kMessageBackground );
    }
    if ( text ) {
        *text = ( index >= 0 && index < m_messages.size() ) ? m_messages.at( index ) : QString();
    }
}

pQueuedMessageToolBar::pQueuedMessageToolBar( QWidget* parent )
    : QToolBar( parent )
{
    m_widget = new pQueuedMessageWidget( this );
    addWidget( m_widget );
    connect( m_widget, &pQueuedMessageWidget::closeRequested, this, &pQueuedMessageToolBar::closeMessage );
}

pQueuedMessageWidget* pQueuedMessageToolBar::queuedMessageWidget() const
{
    return m_widget;
}

void pQueuedMessageToolBar::appendMessage( const QString& text )
{
    m_widget->appendMessage( text );
    setVisible( true );
}

void pQueuedMessageToolBar::closeMessage()
{
    m_widget->clearMessages();
    setVisible( false );
}
