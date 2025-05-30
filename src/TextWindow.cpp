// Text widget with a close button
#include "config.h"
#include "i18n.h"

#include <stdio.h>
#include <stdlib.h>

#include <fx.h>

#include "icons.h"
#include "xfeutils.h"
#include "TextWindow.h"


// Map
FXDEFMAP(TextWindow) TextWindowMap[] = {};


// Object implementation
FXIMPLEMENT(TextWindow, DialogBox, TextWindowMap, ARRAYNUMBER(TextWindowMap))


// Construct
TextWindow::TextWindow(FXWindow* owner, const FXString& name, int nblines, int nbcols) :
    DialogBox(owner, name, DECOR_TITLE | DECOR_BORDER | DECOR_STRETCHABLE | DECOR_MAXIMIZE | DECOR_CLOSE,
              0, 0, 0, 0, 6, 6, 6, 6, 4, 4)
{
    // Bottom part
    FXHorizontalFrame* closebox = new FXHorizontalFrame(this, LAYOUT_SIDE_BOTTOM | LAYOUT_FILL_X | PACK_UNIFORM_WIDTH);
    FXButton* button = new FXButton(closebox, _("&Close"), NULL, this, DialogBox::ID_ACCEPT,
                                    BUTTON_INITIAL | BUTTON_DEFAULT | LAYOUT_RIGHT | FRAME_GROOVE, 0, 0, 0, 0, 20, 20,
                                    5, 5);

    // Add shortcut for closing window
    FXString key = getApp()->reg().readStringEntry("KEYBINDINGS", "close", "Ctrl-W");
    FXHotKey hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, this, FXSEL(SEL_COMMAND, DialogBox::ID_ACCEPT));

    // Text part
    FXHorizontalFrame* textbox = new FXHorizontalFrame(this,
                                                       LAYOUT_SIDE_TOP | LAYOUT_FILL_X | LAYOUT_FILL_Y | FRAME_NONE,
                                                       0, 0, 0, 0, 0, 0, 0, 0);

    text = new FXText(textbox, NULL, 0, TEXT_READONLY | TEXT_WORDWRAP | LAYOUT_FILL_X | LAYOUT_FILL_Y);
    text->setVisibleRows(nblines);
    text->setVisibleColumns(nbcols);

    button->setFocus();
}


// Construct Text dialog box
TextWindow::TextWindow(FXApp* app, const FXString& name, int nblines, int nbcols) :
    DialogBox(app, name, DECOR_TITLE | DECOR_BORDER | DECOR_STRETCHABLE | DECOR_MINIMIZE | DECOR_MAXIMIZE | DECOR_CLOSE,
              0, 0, 0, 0, 6, 6, 6, 6, 4, 4)
{
    // Bottom part
    FXHorizontalFrame* closebox = new FXHorizontalFrame(this, LAYOUT_SIDE_BOTTOM | LAYOUT_FILL_X | PACK_UNIFORM_WIDTH);
    FXButton* button = new FXButton(closebox, _("&Close"), NULL, this, DialogBox::ID_ACCEPT,
                                    BUTTON_INITIAL | BUTTON_DEFAULT | LAYOUT_RIGHT | FRAME_GROOVE, 0, 0, 0, 0, 20, 20,
                                    5, 5);
    
    // Add shortcut for closing window
    FXString key = getApp()->reg().readStringEntry("KEYBINDINGS", "close", "Ctrl-W");
    FXHotKey hotkey = xf_parseaccel(key);
    getAccelTable()->addAccel(hotkey, this, FXSEL(SEL_COMMAND, DialogBox::ID_ACCEPT));

    // Text part
    FXHorizontalFrame* textbox = new FXHorizontalFrame(this,
                                                       LAYOUT_SIDE_TOP | LAYOUT_FILL_X | LAYOUT_FILL_Y | FRAME_NONE,
                                                       0, 0, 0, 0, 0, 0, 0, 0);

    text = new FXText(textbox, NULL, 0, TEXT_READONLY | TEXT_WORDWRAP | LAYOUT_FILL_X | LAYOUT_FILL_Y);
    text->setVisibleRows(nblines);
    text->setVisibleColumns(nbcols);

    button->setFocus();
}


// Change the text in the buffer to new text
void TextWindow::setText(const char* str)
{
    text->setText(str, strlen(str));
    getApp()->repaint();
}


// Append new text at the end of the buffer
void TextWindow::appendText(const char* str)
{
    text->appendText(str, strlen(str));
    getApp()->repaint();
}


// Change the text in the buffer to new text
void TextWindow::setFont(FXFont* font)
{
    text->setFont(font);
    getApp()->repaint();
}


// Scroll to the last line
void TextWindow::scrollToLastLine(void)
{
    text->makePositionVisible(text->getLength());
    getApp()->repaint();
}


// Get text length
int TextWindow::getLength(void)
{
    return text->getLength();
}


// Clean up
TextWindow::~TextWindow()
{
    text = (FXText*)-1;
}
