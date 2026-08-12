#ifndef FRESHGUI_PTRANSLATIONDIALOG_H
#define FRESHGUI_PTRANSLATIONDIALOG_H

#include <QString>

class QWidget;
class pTranslationManager;

class pTranslationDialog
{
public:
    static QString getLocale( pTranslationManager* manager, QWidget* parent = nullptr );
};

#endif // FRESHGUI_PTRANSLATIONDIALOG_H
