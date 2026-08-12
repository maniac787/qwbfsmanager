#ifndef FRESHGUI_PQUEUEDMESSAGETOOLBAR_H
#define FRESHGUI_PQUEUEDMESSAGETOOLBAR_H

#include <QToolBar>

class QLabel;
class QToolButton;

class pQueuedMessageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit pQueuedMessageWidget( QWidget* parent = nullptr );
    void appendMessage( const QString& text );
    void clearMessages();
    int pendingMessageCount() const;
    void currentMessageInformations( int index, QBrush* brush, QString* text ) const;

signals:
    void closeRequested();

private:
    QLabel* m_label;
    QToolButton* m_closeButton;
    QStringList m_messages;
};

class pQueuedMessageToolBar : public QToolBar
{
    Q_OBJECT

public:
    explicit pQueuedMessageToolBar( QWidget* parent = nullptr );

    pQueuedMessageWidget* queuedMessageWidget() const;
    void appendMessage( const QString& text );

public slots:
    void closeMessage();

private:
    pQueuedMessageWidget* m_widget;
};

#endif // FRESHGUI_PQUEUEDMESSAGETOOLBAR_H
