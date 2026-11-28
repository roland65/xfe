#ifndef TEXTWINDOW_H
#define TEXTWINDOW_H

#include "DialogBox.h"

class TextWindow : public DialogBox
{
    FXDECLARE(TextWindow)
protected:
    FXText* text = NULL;
    FXFont* ownedFont = NULL;
private:
    TextWindow()
    {
    }
    TextWindow(const TextWindow&);
public:
    enum
    {
        ID_CLOSE=DialogBox::ID_LAST,
        ID_LAST
    };
public:
    TextWindow(FXWindow* owner, const FXString& name, int nblines, int nbcols);
    TextWindow(FXApp* app, const FXString& name, int nblines, int nbcols);
    virtual ~TextWindow();
    void setText(const char*);
    void appendText(const char*);
    void scrollToLastLine(void);
    void setFont(FXFont*, FXbool owned = false);
    int getLength(void);
};

#endif
