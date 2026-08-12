#ifndef FRESHGUI_PDOCKWIDGET_H
#define FRESHGUI_PDOCKWIDGET_H

#include <QDockWidget>

class pDockWidget : public QDockWidget
{
    Q_OBJECT
public:
    explicit pDockWidget( QWidget* parent = nullptr )
        : QDockWidget( parent )
    {}
};

#endif // FRESHGUI_PDOCKWIDGET_H
