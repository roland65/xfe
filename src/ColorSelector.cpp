// ColorWell, ColorSelector and ColorDialog. Taken from the FOX library and slightly modified.

#include "config.h"
#include "i18n.h"

#include <fx.h>
#include <fxkeys.h>

#include "icons.h"
#include "ColorSelector.h"


//
// ColorWell class
//


#define WELLSIZE       12  // Minimum well size
#define FOCUSBORDER    3   // Focus border


// Map
FXDEFMAP(ColorWell) ColorWellMap[] =
{
    FXMAPFUNC(SEL_PAINT, 0, ColorWell::onPaint),
    FXMAPFUNC(SEL_MOTION, 0, ColorWell::onMotion),
    FXMAPFUNC(SEL_DRAGGED, 0, ColorWell::onDragged),
    FXMAPFUNC(SEL_DND_MOTION, 0, ColorWell::onDNDMotion),
    FXMAPFUNC(SEL_DND_ENTER, 0, ColorWell::onDNDEnter),
    FXMAPFUNC(SEL_DND_LEAVE, 0, ColorWell::onDNDLeave),
    FXMAPFUNC(SEL_DND_DROP, 0, ColorWell::onDNDDrop),
    FXMAPFUNC(SEL_DND_REQUEST, 0, ColorWell::onDNDRequest),
    FXMAPFUNC(SEL_FOCUSIN, 0, ColorWell::onFocusIn),
    FXMAPFUNC(SEL_FOCUSOUT, 0, ColorWell::onFocusOut),
    FXMAPFUNC(SEL_LEFTBUTTONPRESS, 0, ColorWell::onLeftBtnPress),
    FXMAPFUNC(SEL_LEFTBUTTONRELEASE, 0, ColorWell::onLeftBtnRelease),
    FXMAPFUNC(SEL_MIDDLEBUTTONPRESS, 0, ColorWell::onMiddleBtnPress),
    FXMAPFUNC(SEL_MIDDLEBUTTONRELEASE, 0, ColorWell::onMiddleBtnRelease),
    FXMAPFUNC(SEL_CLICKED, 0, ColorWell::onClicked),
    FXMAPFUNC(SEL_DOUBLECLICKED, 0, ColorWell::onDoubleClicked),
    FXMAPFUNC(SEL_KEYPRESS, 0, ColorWell::onKeyPress),
    FXMAPFUNC(SEL_KEYRELEASE, 0, ColorWell::onKeyRelease),
    FXMAPFUNC(SEL_UNGRABBED, 0, ColorWell::onUngrabbed),
    FXMAPFUNC(SEL_BEGINDRAG, 0, ColorWell::onBeginDrag),
    FXMAPFUNC(SEL_ENDDRAG, 0, ColorWell::onEndDrag),
    FXMAPFUNC(SEL_SELECTION_LOST, 0, ColorWell::onSelectionLost),
    FXMAPFUNC(SEL_SELECTION_GAINED, 0, ColorWell::onSelectionGained),
    FXMAPFUNC(SEL_SELECTION_REQUEST, 0, ColorWell::onSelectionRequest),
    FXMAPFUNC(SEL_QUERY_TIP, 0, ColorWell::onQueryTip),
    FXMAPFUNC(SEL_QUERY_HELP, 0, ColorWell::onQueryHelp),
    FXMAPFUNC(SEL_COMMAND, ColorWell::ID_SETVALUE, ColorWell::onCmdSetValue),
    FXMAPFUNC(SEL_COMMAND, ColorWell::ID_SETINTVALUE, ColorWell::onCmdSetIntValue),
    FXMAPFUNC(SEL_COMMAND, ColorWell::ID_GETINTVALUE, ColorWell::onCmdGetIntValue),
    FXMAPFUNC(SEL_CHANGED, ColorWell::ID_COLORDIALOG, ColorWell::onChgColorWell),
    FXMAPFUNC(SEL_COMMAND, ColorWell::ID_COLORDIALOG, ColorWell::onCmdColorWell),
    FXMAPFUNC(SEL_COMMAND, ColorWell::ID_SETHELPSTRING, ColorWell::onCmdSetHelp),
    FXMAPFUNC(SEL_COMMAND, ColorWell::ID_GETHELPSTRING, ColorWell::onCmdGetHelp),
    FXMAPFUNC(SEL_COMMAND, ColorWell::ID_SETTIPSTRING, ColorWell::onCmdSetTip),
    FXMAPFUNC(SEL_COMMAND, ColorWell::ID_GETTIPSTRING, ColorWell::onCmdGetTip),
};


// Object implementation
FXIMPLEMENT(ColorWell, FXFrame, ColorWellMap, ARRAYNUMBER(ColorWellMap))



// Construct
ColorWell::ColorWell()
{
    flags |= FLAG_ENABLED | FLAG_DROPTARGET;
    rgba = 0;
    wellColor[0] = 0;
    wellColor[1] = 0;
}


// Make a color well
ColorWell::ColorWell(FXComposite* p, FXColor clr, FXObject* tgt, FXSelector sel, FXuint opts,
                     FXint x, FXint y, FXint w, FXint h, FXint pl, FXint pr, FXint pt, FXint pb) :
    FXFrame(p, opts, x, y, w, h, pl, pr, pt, pb)
{
    flags |= FLAG_ENABLED | FLAG_DROPTARGET;
    target = tgt;
    message = sel;
    rgba = clr;
    wellColor[0] = rgbaoverwhite(rgba);
    wellColor[1] = rgbaoverblack(rgba);
}


// Create window
void ColorWell::create()
{
    FXFrame::create();
    if (!colorType)
    {
        colorType = getApp()->registerDragType(colorTypeName);
    }
    if (!textType)
    {
        textType = getApp()->registerDragType(textTypeName);
    }
    if (!utf8Type)
    {
        utf8Type = getApp()->registerDragType(utf8TypeName);
    }
}


// Detach window
void ColorWell::detach()
{
    FXFrame::detach();
    colorType = 0;
    textType = 0;
}


// If window can have focus
bool ColorWell::canFocus() const
{
    return true;
}


// Into focus chain
void ColorWell::setFocus()
{
    FXFrame::setFocus();
    setDefault(TRUE);
}


// Out of focus chain
void ColorWell::killFocus()
{
    FXFrame::killFocus();
    setDefault(MAYBE);
}


// Compute color over black
FXColor ColorWell::rgbaoverblack(FXColor clr)
{
    FXint r, g, b, mul = FXALPHAVAL(clr);

    r = (FXREDVAL(clr) * mul + 127) / 255;
    g = (FXGREENVAL(clr) * mul + 127) / 255;
    b = (FXBLUEVAL(clr) * mul + 127) / 255;
    return FXRGB(r, g, b);
}


// Compute color over white
FXColor ColorWell::rgbaoverwhite(FXColor clr)
{
    FXint r, g, b, mul = FXALPHAVAL(clr), lum = (255 - mul);

    r = (lum * 255 + FXREDVAL(clr) * mul + 127) / 255;
    g = (lum * 255 + FXGREENVAL(clr) * mul + 127) / 255;
    b = (lum * 255 + FXBLUEVAL(clr) * mul + 127) / 255;
    return FXRGB(r, g, b);
}


// Get default size
FXint ColorWell::getDefaultWidth()
{
    return WELLSIZE + FOCUSBORDER + padleft + padright + 4;
}


FXint ColorWell::getDefaultHeight()
{
    return WELLSIZE + FOCUSBORDER + padtop + padbottom + 4;
}

// Handle repaint
long ColorWell::onPaint(FXObject*, FXSelector, void* ptr)
{
    FXEvent* ev = (FXEvent*)ptr;
    FXDCWindow dc(this, ev);
    FXPoint points[3];

    dc.setForeground(backColor);
    dc.fillRectangle(0, 0, width, padtop + FOCUSBORDER);
    dc.fillRectangle(0, padtop + FOCUSBORDER, padleft + FOCUSBORDER, height - padtop - padbottom - (FOCUSBORDER << 1));
    dc.fillRectangle(width - padright - FOCUSBORDER, padtop + FOCUSBORDER, padright + FOCUSBORDER,
                     height - padtop - padbottom - (FOCUSBORDER << 1));
    dc.fillRectangle(0, height - padbottom - FOCUSBORDER, width, padbottom + FOCUSBORDER);

    if (wellColor[1] == backColor)
    {
        dc.setForeground(hiliteColor);
        dc.fillRectangle(padleft + FOCUSBORDER, padtop + FOCUSBORDER, width - padright - padleft - (FOCUSBORDER << 1),
                         height - padbottom - padtop - (FOCUSBORDER << 1));
    }
    else
    {
        dc.fillRectangle(padleft + FOCUSBORDER, padtop + FOCUSBORDER, width - padright - padleft - (FOCUSBORDER << 1),
                         height - padbottom - padtop - (FOCUSBORDER << 1));
    }
    if (hasSelection())
    {
        dc.setForeground(borderColor);
        dc.drawRectangle(padleft + 1, padtop + 1, width - padright - padleft - 3, height - padbottom - padtop - 3);
    }
    dc.setForeground(wellColor[0]);
    points[0].x = points[1].x = padleft + FOCUSBORDER + 2;
    points[2].x = width - padright - FOCUSBORDER - 2;
    points[0].y = points[2].y = padtop + FOCUSBORDER + 2;
    points[1].y = height - padbottom - FOCUSBORDER - 2;
    dc.fillPolygon(points, 3);

    dc.setForeground(wellColor[1]);
    points[0].x = padleft + FOCUSBORDER + 2;
    points[1].x = points[2].x = width - padright - FOCUSBORDER - 2;
    points[0].y = points[1].y = height - padbottom - FOCUSBORDER - 2;
    points[2].y = padtop + FOCUSBORDER + 2;
    dc.fillPolygon(points, 3);

    drawGrooveRectangle(dc, padleft + FOCUSBORDER, padtop + FOCUSBORDER,
                        width - padright - padleft - (FOCUSBORDER << 1),
                        height - padbottom - padtop - (FOCUSBORDER << 1));

    if (hasFocus())
    {
        dc.drawFocusRectangle(padleft, padtop, width - padright - padleft, height - padbottom - padtop);
    }
    return 1;
}


// Gained focus
long ColorWell::onFocusIn(FXObject* sender, FXSelector sel, void* ptr)
{
    FXFrame::onFocusIn(sender, sel, ptr);
    update();
    return 1;
}


// Lost focus
long ColorWell::onFocusOut(FXObject* sender, FXSelector sel, void* ptr)
{
    FXFrame::onFocusOut(sender, sel, ptr);
    update();
    return 1;
}


// Dragging something over well; save old color and block GUI updates
long ColorWell::onDNDEnter(FXObject* sender, FXSelector sel, void* ptr)
{
    if (FXFrame::onDNDEnter(sender, sel, ptr))
    {
        return 1;
    }
    flags &= ~FLAG_UPDATE;
    return 1;
}


// Dragged out of well, so restore old color and reenable GUI updates
long ColorWell::onDNDLeave(FXObject* sender, FXSelector sel, void* ptr)
{
    if (FXFrame::onDNDLeave(sender, sel, ptr))
    {
        return 1;
    }
    flags |= FLAG_UPDATE;
    return 1;
}


// Handle drag-and-drop motion
long ColorWell::onDNDMotion(FXObject* sender, FXSelector sel, void* ptr)
{
    // Handle base class first
    if (FXFrame::onDNDMotion(sender, sel, ptr))
    {
        return 1;
    }

    // No more messages while inside
    setDragRectangle(0, 0, width, height, FALSE);

    // Is it a color being dropped?
    if (offeredDNDType(FROM_DRAGNDROP, colorType))
    {
        acceptDrop(DRAG_ACCEPT);
        return 1;
    }

    // Is it a name of a color being dropped?
    if (offeredDNDType(FROM_DRAGNDROP, textType))
    {
        acceptDrop(DRAG_ACCEPT);
        return 1;
    }
    return 0;
}


// Handle drag-and-drop drop
long ColorWell::onDNDDrop(FXObject* sender, FXSelector sel, void* ptr)
{
    FXuchar* pointer;
    FXuint length;
    FXColor color;

    // Enable updating
    flags |= FLAG_UPDATE;

    // Try handling it in base class first
    if (FXFrame::onDNDDrop(sender, sel, ptr))
    {
        return 1;
    }

    // Try and obtain the color first
    if (getDNDData(FROM_DRAGNDROP, colorType, pointer, length))
    {
        color = FXRGBA((((FXushort*)pointer)[0] + 128) / 257, (((FXushort*)pointer)[1] + 128) / 257,
                       (((FXushort*)pointer)[2] + 128) / 257, (((FXushort*)pointer)[3] + 128) / 257);
        FXFREE(&pointer);
        setRGBA(color, TRUE);
        return 1;
    }

    // Maybe its the name of a color
    if (getDNDData(FROM_DRAGNDROP, textType, pointer, length))
    {
        FXRESIZE(&pointer, FXuchar, length + 1); pointer[length] = '\0';
        color = fxcolorfromname((const FXchar*)pointer);
        FXFREE(&pointer);

        // Accept the drop only if it was a valid color name
        if (color != FXRGBA(0, 0, 0, 0))
        {
            setRGBA(color, TRUE);
            return 1;
        }
    }

    return 0;
}


// Service requested DND data
long ColorWell::onDNDRequest(FXObject* sender, FXSelector sel, void* ptr)
{
    FXEvent* event = (FXEvent*)ptr; FXushort* clr; FXchar* str;

    // Try handling it in base class first
    if (FXFrame::onDNDRequest(sender, sel, ptr))
    {
        return 1;
    }

    // Requested as a color
    if (event->target == colorType)
    {
        FXMALLOC(&clr, FXushort, 4);
        clr[0] = 257 * FXREDVAL(rgba);
        clr[1] = 257 * FXGREENVAL(rgba);
        clr[2] = 257 * FXBLUEVAL(rgba);
        clr[3] = 257 * FXALPHAVAL(rgba);
        setDNDData(FROM_DRAGNDROP, colorType, (FXuchar*)clr, sizeof(FXushort) * 4);
        return 1;
    }

    // Requested as a color name
    if (event->target == textType)
    {
        FXMALLOC(&str, FXchar, 50);
        fxnamefromcolor(str, rgba);
        setDNDData(FROM_DRAGNDROP, textType, (FXuchar*)str, strlen(str));
        return 1;
    }
    return 0;
}


// We now really do have the selection; repaint the text field
long ColorWell::onSelectionGained(FXObject* sender, FXSelector sel, void* ptr)
{
    FXFrame::onSelectionGained(sender, sel, ptr);
    update();
    return 1;
}


// We lost the selection somehow; repaint the text field
long ColorWell::onSelectionLost(FXObject* sender, FXSelector sel, void* ptr)
{
    FXFrame::onSelectionLost(sender, sel, ptr);
    update();
    return 1;
}


// Somebody wants our selection
long ColorWell::onSelectionRequest(FXObject* sender, FXSelector sel, void* ptr)
{
    FXEvent* event = (FXEvent*)ptr;

    // Try handling it in base class first
    if (FXFrame::onSelectionRequest(sender, sel, ptr))
    {
        return 1;
    }

    // Requested as a color
    if (event->target == colorType)
    {
        FXushort* color;
        FXMALLOC(&color, FXushort, 4);
        color[0] = 257 * FXREDVAL(rgba);
        color[1] = 257 * FXGREENVAL(rgba);
        color[2] = 257 * FXBLUEVAL(rgba);
        color[3] = 257 * FXALPHAVAL(rgba);
        setDNDData(FROM_SELECTION, colorType, (FXuchar*)color, sizeof(FXushort) * 4);
        return 1;
    }

    // Requested as a color name
    if (event->target == stringType || event->target == textType)
    {
        FXchar* data;
        FXCALLOC(&data, FXchar, 50);
        fxnamefromcolor(data, rgba);
        setDNDData(FROM_SELECTION, event->target, (FXuchar*)data, strlen(data));
        return 1;
    }
    return 0;
}


// Start a drag operation
long ColorWell::onBeginDrag(FXObject* sender, FXSelector sel, void* ptr)
{
    FXDragType types[2];

    if (FXFrame::onBeginDrag(sender, sel, ptr))
    {
        return 1;
    }
    types[0] = colorType;
    types[1] = textType;
    beginDrag(types, 2);
    setDragCursor(getApp()->getDefaultCursor(DEF_SWATCH_CURSOR));
    return 1;
}


// End drag operation
long ColorWell::onEndDrag(FXObject* sender, FXSelector sel, void* ptr)
{
    if (FXFrame::onEndDrag(sender, sel, ptr))
    {
        return 1;
    }
    endDrag(didAccept() == DRAG_COPY);
    setDragCursor(getApp()->getDefaultCursor(DEF_ARROW_CURSOR));
    return 1;
}


// Dragged stuff around
long ColorWell::onDragged(FXObject* sender, FXSelector sel, void* ptr)
{
    if (FXFrame::onDragged(sender, sel, ptr))
    {
        return 1;
    }
    handleDrag(((FXEvent*)ptr)->root_x, ((FXEvent*)ptr)->root_y, DRAG_COPY);
    if (didAccept() == DRAG_COPY)
    {
        setDragCursor(getApp()->getDefaultCursor(DEF_SWATCH_CURSOR));
    }
    else
    {
        setDragCursor(getApp()->getDefaultCursor(DEF_DNDSTOP_CURSOR));
    }
    return 1;
}


// Moving
long ColorWell::onMotion(FXObject*, FXSelector, void* ptr)
{
    if (flags & FLAG_DODRAG)
    {
        handle(this, FXSEL(SEL_DRAGGED, 0), ptr);
        return 1;
    }
    if ((flags & FLAG_TRYDRAG) && ((FXEvent*)ptr)->moved)
    {
        if (handle(this, FXSEL(SEL_BEGINDRAG, 0), ptr))
        {
            flags |= FLAG_DODRAG;
        }
        flags &= ~FLAG_TRYDRAG;
        return 1;
    }
    return 0;
}


// Drag start
long ColorWell::onLeftBtnPress(FXObject*, FXSelector, void* ptr)
{
    FXEvent* event = (FXEvent*)ptr;

    flags &= ~FLAG_TIP;
    handle(this, FXSEL(SEL_FOCUS_SELF, 0), ptr);
    if (isEnabled())
    {
        grab();
        if (target && target->tryHandle(this, FXSEL(SEL_LEFTBUTTONPRESS, message), ptr))
        {
            return 1;
        }
        if (event->click_count == 1)
        {
            flags &= ~FLAG_UPDATE;
            flags |= FLAG_TRYDRAG;
        }
    }
    return 1;
}


// Drop
long ColorWell::onLeftBtnRelease(FXObject*, FXSelector, void* ptr)
{
    FXEvent* event = (FXEvent*)ptr;
    FXuint flgs = flags;

    if (isEnabled())
    {
        ungrab();
        flags |= FLAG_UPDATE;
        flags &= ~(FLAG_TRYDRAG | FLAG_DODRAG);
        if (target && target->tryHandle(this, FXSEL(SEL_LEFTBUTTONRELEASE, message), ptr))
        {
            return 1;
        }
        if (flgs & FLAG_DODRAG)
        {
            handle(this, FXSEL(SEL_ENDDRAG, 0), ptr);
        }
        if (event->click_count == 1)
        {
            handle(this, FXSEL(SEL_CLICKED, 0), (void*)(FXuval)rgba);
            if (!event->moved && target)
            {
                target->tryHandle(this, FXSEL(SEL_COMMAND, message), (void*)(FXuval)rgba);
            }
        }
        else if (event->click_count == 2)
        {
            handle(this, FXSEL(SEL_DOUBLECLICKED, 0), (void*)(FXuval)rgba);
        }
        return 1;
    }
    return 1;
}


// Pressed middle button to paste
long ColorWell::onMiddleBtnPress(FXObject*, FXSelector, void* ptr)
{
    flags &= ~FLAG_TIP;
    handle(this, FXSEL(SEL_FOCUS_SELF, 0), ptr);
    if (isEnabled())
    {
        grab();
        if (target && target->tryHandle(this, FXSEL(SEL_MIDDLEBUTTONPRESS, message), ptr))
        {
            return 1;
        }
    }
    return 0;
}


// Released middle button causes paste of selection
long ColorWell::onMiddleBtnRelease(FXObject*, FXSelector, void* ptr)
{
    FXuchar* pointer;
    FXuint length;
    FXColor color;

    if (isEnabled())
    {
        ungrab();
        if (target && target->tryHandle(this, FXSEL(SEL_MIDDLEBUTTONRELEASE, message), ptr))
        {
            return 1;
        }
        if (getDNDData(FROM_SELECTION, colorType, pointer, length))
        {
            color = FXRGBA((((FXushort*)pointer)[0] + 128) / 257, (((FXushort*)pointer)[1] + 128) / 257,
                           (((FXushort*)pointer)[2] + 128) / 257, (((FXushort*)pointer)[3] + 128) / 257);
            FXFREE(&color);
            setRGBA(color, TRUE);
            return 1;
        }
        if (getDNDData(FROM_SELECTION, stringType, pointer, length))
        {
            FXRESIZE(&pointer, FXuchar, length + 1); pointer[length] = '\0';
            color = fxcolorfromname((const FXchar*)pointer);
            FXFREE(&pointer);
            setRGBA(color, TRUE);
            return 1;
        }
    }
    return 0;
}


// Key Press
long ColorWell::onKeyPress(FXObject*, FXSelector, void* ptr)
{
    FXEvent* event = (FXEvent*)ptr;

    flags &= ~FLAG_TIP;
    if (isEnabled())
    {
        if (target && target->tryHandle(this, FXSEL(SEL_KEYPRESS, message), ptr))
        {
            return 1;
        }
        switch (event->code)
        {
        case KEY_space:
        case KEY_KP_Enter:
        case KEY_Return:
            flags &= ~FLAG_UPDATE;
            return 1;
        }
    }
    return 0;
}


// Key Release
long ColorWell::onKeyRelease(FXObject*, FXSelector, void* ptr)
{
    FXEvent* event = (FXEvent*)ptr;

    if (isEnabled())
    {
        flags |= FLAG_UPDATE;
        if (target && target->tryHandle(this, FXSEL(SEL_KEYRELEASE, message), ptr))
        {
            return 1;
        }
        switch (event->code)
        {
        case KEY_space:
            handle(this, FXSEL(SEL_CLICKED, 0), (void*)(FXuval)rgba);
            if (target)
            {
                target->tryHandle(this, FXSEL(SEL_COMMAND, message), (void*)(FXuval)rgba);
            }
            return 1;
        case KEY_KP_Enter:
        case KEY_Return:
            handle(this, FXSEL(SEL_DOUBLECLICKED, 0), (void*)(FXuval)rgba);
            return 1;
        }
    }
    return 0;
}


// Clicked in the well
long ColorWell::onClicked(FXObject*, FXSelector, void*)
{
    FXDragType types[3];

    if (target && target->tryHandle(this, FXSEL(SEL_CLICKED, message), (void*)(FXuval)rgba))
    {
        return 1;
    }
    if (!hasSelection())
    {
        types[0] = stringType;
        types[1] = colorType;
        types[2] = textType;
        acquireSelection(types, 3);
    }
    return 1;
}


// Double clicked in well; normally pops the color dialog
// except when _COLORWELL_SOURCEONLY is passed in which case
// editing by the dialog is not allowed (used for wells inside the
// color dialog itself for example).
// The well follows the editing via the dialog; when the dialog
// is closed by cancelling it it will revert to the old color
long ColorWell::onDoubleClicked(FXObject*, FXSelector, void*)
{
    if (target && target->tryHandle(this, FXSEL(SEL_DOUBLECLICKED, message), (void*)(FXuval)rgba))
    {
        return 1;
    }
    if (options & _COLORWELL_SOURCEONLY)
    {
        return 1;
    }
    ColorDialog colordialog(this, _("Color Dialog"));
    FXColor oldcolor = getRGBA();
    colordialog.setTarget(this);
    colordialog.setSelector(ID_COLORDIALOG);
    colordialog.setRGBA(oldcolor);
    colordialog.setOpaqueOnly(isOpaqueOnly());
    if (!colordialog.execute())
    {
        setRGBA(oldcolor, TRUE);
    }
    return 1;
}


// The widget lost the grab for some reason
long ColorWell::onUngrabbed(FXObject* sender, FXSelector sel, void* ptr)
{
    FXFrame::onUngrabbed(sender, sel, ptr);
    flags &= ~(FLAG_TRYDRAG | FLAG_DODRAG);
    flags |= FLAG_UPDATE;
    endDrag(FALSE);
    return 1;
}


// Change from another Color Well
long ColorWell::onChgColorWell(FXObject*, FXSelector, void* ptr)
{
    flags &= ~FLAG_UPDATE;
    setRGBA((FXColor)(FXuval)ptr);
    if (target)
    {
        target->tryHandle(this, FXSEL(SEL_CHANGED, message), (void*)(FXuval)rgba);
    }
    return 1;
}


// Command from another Color Well
long ColorWell::onCmdColorWell(FXObject*, FXSelector, void* ptr)
{
    flags |= FLAG_UPDATE;
    setRGBA((FXColor)(FXuval)ptr);
    if (target)
    {
        target->tryHandle(this, FXSEL(SEL_COMMAND, message), (void*)(FXuval)rgba);
    }
    return 1;
}


// Set help using a message
long ColorWell::onCmdSetHelp(FXObject*, FXSelector, void* ptr)
{
    setHelpText(*((FXString*)ptr));
    return 1;
}


// Get help using a message
long ColorWell::onCmdGetHelp(FXObject*, FXSelector, void* ptr)
{
    *((FXString*)ptr) = getHelpText();
    return 1;
}


// Set tip using a message
long ColorWell::onCmdSetTip(FXObject*, FXSelector, void* ptr)
{
    setTipText(*((FXString*)ptr));
    return 1;
}


// Get tip using a message
long ColorWell::onCmdGetTip(FXObject*, FXSelector, void* ptr)
{
    *((FXString*)ptr) = getTipText();
    return 1;
}


// We were asked about tip text
long ColorWell::onQueryTip(FXObject* sender, FXSelector sel, void* ptr)
{
    if (FXWindow::onQueryTip(sender, sel, ptr))
    {
        return 1;
    }
    if ((flags & FLAG_TIP) && !tip.empty())
    {
        sender->handle(this, FXSEL(SEL_COMMAND, ID_SETSTRINGVALUE), (void*)&tip);
        return 1;
    }
    return 0;
}


// We were asked about status text
long ColorWell::onQueryHelp(FXObject* sender, FXSelector sel, void* ptr)
{
    if (FXWindow::onQueryHelp(sender, sel, ptr))
    {
        return 1;
    }
    if ((flags & FLAG_HELP) && !help.empty())
    {
        sender->handle(this, FXSEL(SEL_COMMAND, ID_SETSTRINGVALUE), (void*)&help);
        return 1;
    }
    return 0;
}


// Change RGBA color
void ColorWell::setRGBA(FXColor clr, FXbool notify)
{
    if (options & _COLORWELL_OPAQUEONLY)
    {
        clr |= FXRGBA(0, 0, 0, 255);
    }
    if (clr != rgba)
    {
        rgba = clr;
        wellColor[0] = rgbaoverwhite(rgba);
        wellColor[1] = rgbaoverblack(rgba);
        update();
        if (notify && target)
        {
            target->tryHandle(this, FXSEL(SEL_COMMAND, message), (void*)(FXuval)rgba);
        }
    }
}


// Set color
long ColorWell::onCmdSetValue(FXObject*, FXSelector, void* ptr)
{
    setRGBA((FXColor)(FXuval)ptr);
    return 1;
}


// Update well from a message
long ColorWell::onCmdSetIntValue(FXObject*, FXSelector, void* ptr)
{
    setRGBA(*((FXColor*)ptr));
    return 1;
}


// Obtain value from well
long ColorWell::onCmdGetIntValue(FXObject*, FXSelector, void* ptr)
{
    *((FXColor*)ptr) = getRGBA();
    return 1;
}


// Return true if only opaque colors allowed
FXbool ColorWell::isOpaqueOnly() const
{
    return (options & _COLORWELL_OPAQUEONLY) != 0;
}


// Change opaque only mode
void ColorWell::setOpaqueOnly(FXbool opaque)
{
    if (opaque)
    {
        options |= _COLORWELL_OPAQUEONLY;
        setRGBA(rgba);
    }
    else
    {
        options &= ~_COLORWELL_OPAQUEONLY;
    }
}


// Save data
void ColorWell::save(FXStream& store) const
{
    FXFrame::save(store);
    store << wellColor[0] << wellColor[1];
    store << rgba;
    store << tip;
    store << help;
}


// Load data
void ColorWell::load(FXStream& store)
{
    FXFrame::load(store);
    store >> wellColor[0] >> wellColor[1];
    store >> rgba;
    store >> tip;
    store >> help;
}


// Destroy
ColorWell::~ColorWell()
{
}




//
// ColorSelector class
//


// Map
FXDEFMAP(ColorSelector) ColorSelectorMap[] =
{
    FXMAPFUNC(SEL_UPDATE, ColorSelector::ID_ALPHA_TEXT, ColorSelector::onUpdAlphaText),
    FXMAPFUNC(SEL_COMMAND, ColorSelector::ID_ALPHA_TEXT, ColorSelector::onCmdAlphaText),
    FXMAPFUNC(SEL_UPDATE, ColorSelector::ID_ALPHA_LABEL, ColorSelector::onUpdAlphaLabel),
    FXMAPFUNC(SEL_UPDATE, ColorSelector::ID_ALPHA_SLIDER, ColorSelector::onUpdAlphaSlider),
    FXMAPFUNC(SEL_CHANGED, ColorSelector::ID_ALPHA_SLIDER, ColorSelector::onCmdAlphaSlider),
    FXMAPFUNC(SEL_COMMAND, ColorSelector::ID_ALPHA_SLIDER, ColorSelector::onCmdAlphaSlider),
    FXMAPFUNCS(SEL_UPDATE, ColorSelector::ID_RGB_RED_TEXT, ColorSelector::ID_RGB_BLUE_TEXT,
               ColorSelector::onUpdRGBText),
    FXMAPFUNCS(SEL_UPDATE, ColorSelector::ID_HSV_HUE_TEXT, ColorSelector::ID_HSV_VALUE_TEXT,
               ColorSelector::onUpdHSVText),
    FXMAPFUNCS(SEL_UPDATE, ColorSelector::ID_CMY_CYAN_TEXT, ColorSelector::ID_CMY_YELLOW_TEXT,
               ColorSelector::onUpdCMYText),
    FXMAPFUNCS(SEL_COMMAND, ColorSelector::ID_RGB_RED_TEXT, ColorSelector::ID_RGB_BLUE_TEXT,
               ColorSelector::onCmdRGBText),
    FXMAPFUNCS(SEL_COMMAND, ColorSelector::ID_HSV_HUE_TEXT, ColorSelector::ID_HSV_VALUE_TEXT,
               ColorSelector::onCmdHSVText),
    FXMAPFUNCS(SEL_COMMAND, ColorSelector::ID_CMY_CYAN_TEXT, ColorSelector::ID_CMY_YELLOW_TEXT,
               ColorSelector::onCmdCMYText),
    FXMAPFUNCS(SEL_UPDATE, ColorSelector::ID_RGB_RED_SLIDER, ColorSelector::ID_RGB_BLUE_SLIDER,
               ColorSelector::onUpdRGBSlider),
    FXMAPFUNCS(SEL_UPDATE, ColorSelector::ID_HSV_HUE_SLIDER, ColorSelector::ID_HSV_VALUE_SLIDER,
               ColorSelector::onUpdHSVSlider),
    FXMAPFUNCS(SEL_UPDATE, ColorSelector::ID_CMY_CYAN_SLIDER, ColorSelector::ID_CMY_YELLOW_SLIDER,
               ColorSelector::onUpdCMYSlider),
    FXMAPFUNCS(SEL_CHANGED, ColorSelector::ID_RGB_RED_SLIDER, ColorSelector::ID_RGB_BLUE_SLIDER,
               ColorSelector::onCmdRGBSlider),
    FXMAPFUNCS(SEL_COMMAND, ColorSelector::ID_RGB_RED_SLIDER, ColorSelector::ID_RGB_BLUE_SLIDER,
               ColorSelector::onCmdRGBSlider),
    FXMAPFUNCS(SEL_CHANGED, ColorSelector::ID_HSV_HUE_SLIDER, ColorSelector::ID_HSV_VALUE_SLIDER,
               ColorSelector::onCmdHSVSlider),
    FXMAPFUNCS(SEL_COMMAND, ColorSelector::ID_HSV_HUE_SLIDER, ColorSelector::ID_HSV_VALUE_SLIDER,
               ColorSelector::onCmdHSVSlider),
    FXMAPFUNCS(SEL_CHANGED, ColorSelector::ID_CMY_CYAN_SLIDER, ColorSelector::ID_CMY_YELLOW_SLIDER,
               ColorSelector::onCmdCMYSlider),
    FXMAPFUNCS(SEL_COMMAND, ColorSelector::ID_CMY_CYAN_SLIDER, ColorSelector::ID_CMY_YELLOW_SLIDER,
               ColorSelector::onCmdCMYSlider),
    FXMAPFUNC(SEL_COMMAND, ColorSelector::ID_WELL_CHANGED, ColorSelector::onCmdWell),
    FXMAPFUNC(SEL_CHANGED, ColorSelector::ID_WELL_CHANGED, ColorSelector::onChgWell),
    FXMAPFUNCS(SEL_COMMAND, ColorSelector::ID_CUSTOM_FIRST, ColorSelector::ID_CUSTOM_LAST,
               ColorSelector::onCmdCustomWell),
    FXMAPFUNCS(SEL_CHANGED, ColorSelector::ID_CUSTOM_FIRST, ColorSelector::ID_CUSTOM_LAST,
               ColorSelector::onChgCustomWell),
    FXMAPFUNC(SEL_COMMAND, ColorSelector::ID_COLOR_LIST, ColorSelector::onCmdList),
    FXMAPFUNC(SEL_COMMAND, FXWindow::ID_SETVALUE, ColorSelector::onCmdSetValue),
    FXMAPFUNC(SEL_COMMAND, ColorSelector::ID_ACTIVEPANE, ColorSelector::onCmdActivePane),
    FXMAPFUNC(SEL_UPDATE, ColorSelector::ID_DIAL_WHEEL, ColorSelector::onUpdWheel),
    FXMAPFUNC(SEL_COMMAND, ColorSelector::ID_DIAL_WHEEL, ColorSelector::onCmdWheel),
    FXMAPFUNC(SEL_CHANGED, ColorSelector::ID_DIAL_WHEEL, ColorSelector::onCmdWheel),
    FXMAPFUNC(SEL_COMMAND, ColorSelector::ID_COLORPICK, ColorSelector::onCmdColorPick),
};


// Object implementation
FXIMPLEMENT(ColorSelector, FXPacker, ColorSelectorMap, ARRAYNUMBER(ColorSelectorMap))


// Well names
const FXchar* ColorSelector::wellname[24] =
{
    "wella", "wellb", "wellc", "welld",
    "welle", "wellf", "wellg", "wellh",
    "welli", "wellj", "wellk", "welll",
    "wellm", "welln", "wello", "wellp",
    "wellq", "wellr", "wells", "wellt",
    "wellu", "wellv", "wellw", "wellx"
};


// Separator item
ColorSelector::ColorSelector(FXComposite* p, FXObject* tgt, FXSelector sel, FXuint opts, FXint x, FXint y, FXint w,
                             FXint h) :
    FXPacker(p, opts, x, y, w, h)
{
    FXLabel* label;

    target = tgt;
    message = sel;

    // Buttons
    FXHorizontalFrame* buttons = new FXHorizontalFrame(this, LAYOUT_SIDE_BOTTOM | LAYOUT_FILL_X | PACK_UNIFORM_WIDTH);
    accept = new FXButton(buttons, _("&Accept"), NULL, NULL, 0,
                          BUTTON_INITIAL | BUTTON_DEFAULT | FRAME_GROOVE | LAYOUT_RIGHT,
                          0, 0, 0, 0, 20, 20);
    cancel = new FXButton(buttons, _("&Cancel"), NULL, NULL, 0, BUTTON_DEFAULT | FRAME_GROOVE | LAYOUT_RIGHT,
                          0, 0, 0, 0, 20, 20);

    // Separator
    //~ new FXHorizontalSeparator(this, SEPARATOR_RIDGE | LAYOUT_SIDE_BOTTOM | LAYOUT_FILL_X);

    // Wells with custom colors
    FXHorizontalFrame* colors = new FXHorizontalFrame(this, LAYOUT_SIDE_BOTTOM | LAYOUT_FILL_X, 0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0);
    colorwells[0] = new ColorWell(colors, FXRGBA(255, 255, 255, 255), this, ID_CUSTOM_FIRST + 0, _COLORWELL_SOURCEONLY |
                                  LAYOUT_CENTER_Y | LAYOUT_CENTER_X, 0, 0, 0, 0, 0, 0, 0, 0);
    colorwells[1] = new ColorWell(colors, FXRGBA(0, 0, 0, 255), this, ID_CUSTOM_FIRST + 1, _COLORWELL_SOURCEONLY |
                                  LAYOUT_CENTER_Y | LAYOUT_CENTER_X, 0, 0, 0, 0, 0, 0, 0, 0);
    colorwells[2] = new ColorWell(colors, FXRGBA(255, 0, 0, 255), this, ID_CUSTOM_FIRST + 2, _COLORWELL_SOURCEONLY |
                                  LAYOUT_CENTER_Y | LAYOUT_CENTER_X, 0, 0, 0, 0, 0, 0, 0, 0);
    colorwells[3] = new ColorWell(colors, FXRGBA(0, 255, 0, 255), this, ID_CUSTOM_FIRST + 3, _COLORWELL_SOURCEONLY |
                                  LAYOUT_CENTER_Y | LAYOUT_CENTER_X, 0, 0, 0, 0, 0, 0, 0, 0);

    colorwells[4] = new ColorWell(colors, FXRGBA(0, 0, 255, 255), this, ID_CUSTOM_FIRST + 4, _COLORWELL_SOURCEONLY |
                                  LAYOUT_CENTER_Y | LAYOUT_CENTER_X, 0, 0, 0, 0, 0, 0, 0, 0);
    colorwells[5] = new ColorWell(colors, FXRGBA(0, 0, 255, 255), this, ID_CUSTOM_FIRST + 5, _COLORWELL_SOURCEONLY |
                                  LAYOUT_CENTER_Y | LAYOUT_CENTER_X, 0, 0, 0, 0, 0, 0, 0, 0);
    colorwells[6] = new ColorWell(colors, FXRGBA(255, 255, 0, 255), this, ID_CUSTOM_FIRST + 6, _COLORWELL_SOURCEONLY |
                                  LAYOUT_CENTER_Y | LAYOUT_CENTER_X, 0, 0, 0, 0, 0, 0, 0, 0);
    colorwells[7] = new ColorWell(colors, FXRGBA(0, 255, 255, 255), this, ID_CUSTOM_FIRST + 7, _COLORWELL_SOURCEONLY |
                                  LAYOUT_CENTER_Y | LAYOUT_CENTER_X, 0, 0, 0, 0, 0, 0, 0, 0);

    colorwells[8] = new ColorWell(colors, FXRGBA(255, 0, 255, 255), this, ID_CUSTOM_FIRST + 8, _COLORWELL_SOURCEONLY |
                                  LAYOUT_CENTER_Y | LAYOUT_CENTER_X, 0, 0, 0, 0, 0, 0, 0, 0);
    colorwells[9] = new ColorWell(colors, FXRGBA(128, 0, 0, 255), this, ID_CUSTOM_FIRST + 9, _COLORWELL_SOURCEONLY |
                                  LAYOUT_CENTER_Y | LAYOUT_CENTER_X, 0, 0, 0, 0, 0, 0, 0, 0);
    colorwells[10] = new ColorWell(colors, FXRGBA(0, 128, 0, 255), this, ID_CUSTOM_FIRST + 10, _COLORWELL_SOURCEONLY |
                                   LAYOUT_CENTER_Y | LAYOUT_CENTER_X, 0, 0, 0, 0, 0, 0, 0, 0);
    colorwells[11] = new ColorWell(colors, FXRGBA(0, 0, 128, 255), this, ID_CUSTOM_FIRST + 11, _COLORWELL_SOURCEONLY |
                                   LAYOUT_CENTER_Y | LAYOUT_CENTER_X, 0, 0, 0, 0, 0, 0, 0, 0);

    colorwells[12] = new ColorWell(colors, FXRGBA(128, 128, 0, 255), this, ID_CUSTOM_FIRST + 12, _COLORWELL_SOURCEONLY |
                                   LAYOUT_CENTER_Y | LAYOUT_CENTER_X, 0, 0, 0, 0, 0, 0, 0, 0);
    colorwells[13] = new ColorWell(colors, FXRGBA(128, 0, 128, 255), this, ID_CUSTOM_FIRST + 13, _COLORWELL_SOURCEONLY |
                                   LAYOUT_CENTER_Y | LAYOUT_CENTER_X, 0, 0, 0, 0, 0, 0, 0, 0);
    colorwells[14] = new ColorWell(colors, FXRGBA(0, 128, 128, 255), this, ID_CUSTOM_FIRST + 14, _COLORWELL_SOURCEONLY |
                                   LAYOUT_CENTER_Y | LAYOUT_CENTER_X, 0, 0, 0, 0, 0, 0, 0, 0);
    colorwells[15] = new ColorWell(colors, FXRGBA(0, 128, 128, 255), this, ID_CUSTOM_FIRST + 15, _COLORWELL_SOURCEONLY |
                                   LAYOUT_CENTER_Y | LAYOUT_CENTER_X, 0, 0, 0, 0, 0, 0, 0, 0);

    colorwells[16] = new ColorWell(colors, FXRGBA(255, 0, 255, 255), this, ID_CUSTOM_FIRST + 16, _COLORWELL_SOURCEONLY |
                                   LAYOUT_CENTER_Y | LAYOUT_CENTER_X, 0, 0, 0, 0, 0, 0, 0, 0);
    colorwells[17] = new ColorWell(colors, FXRGBA(128, 0, 0, 255), this, ID_CUSTOM_FIRST + 17, _COLORWELL_SOURCEONLY |
                                   LAYOUT_CENTER_Y | LAYOUT_CENTER_X, 0, 0, 0, 0, 0, 0, 0, 0);
    colorwells[18] = new ColorWell(colors, FXRGBA(0, 128, 0, 255), this, ID_CUSTOM_FIRST + 18, _COLORWELL_SOURCEONLY |
                                   LAYOUT_CENTER_Y | LAYOUT_CENTER_X, 0, 0, 0, 0, 0, 0, 0, 0);
    colorwells[19] = new ColorWell(colors, FXRGBA(0, 0, 128, 255), this, ID_CUSTOM_FIRST + 19, _COLORWELL_SOURCEONLY |
                                   LAYOUT_CENTER_Y | LAYOUT_CENTER_X, 0, 0, 0, 0, 0, 0, 0, 0);

    colorwells[20] = new ColorWell(colors, FXRGBA(128, 128, 0, 255), this, ID_CUSTOM_FIRST + 20, _COLORWELL_SOURCEONLY |
                                   LAYOUT_CENTER_Y | LAYOUT_CENTER_X, 0, 0, 0, 0, 0, 0, 0, 0);
    colorwells[21] = new ColorWell(colors, FXRGBA(128, 0, 128, 255), this, ID_CUSTOM_FIRST + 21, _COLORWELL_SOURCEONLY |
                                   LAYOUT_CENTER_Y | LAYOUT_CENTER_X, 0, 0, 0, 0, 0, 0, 0, 0);
    colorwells[22] = new ColorWell(colors, FXRGBA(0, 128, 128, 255), this, ID_CUSTOM_FIRST + 22, _COLORWELL_SOURCEONLY |
                                   LAYOUT_CENTER_Y | LAYOUT_CENTER_X, 0, 0, 0, 0, 0, 0, 0, 0);
    colorwells[23] = new ColorWell(colors, FXRGBA(0, 128, 128, 255), this, ID_CUSTOM_FIRST + 23, _COLORWELL_SOURCEONLY |
                                   LAYOUT_CENTER_Y | LAYOUT_CENTER_X, 0, 0, 0, 0, 0, 0, 0, 0);

    // Main part
    FXHorizontalFrame* main = new FXHorizontalFrame(this, LAYOUT_SIDE_TOP | FRAME_NONE | LAYOUT_FILL_X | LAYOUT_FILL_Y);

    FXVerticalFrame* wellframe = new FXVerticalFrame(main, LAYOUT_FILL_Y, 0, 0, 0, 0, 0, 0, 10, 0, 0, 2);

    // Color sucker
    new FXPicker(wellframe, _("\tPick Color"), eyedropicon, this, ID_COLORPICK, JUSTIFY_CENTER_X | JUSTIFY_CENTER_Y |
                 FRAME_GROOVE | LAYOUT_CENTER_X | LAYOUT_FIX_WIDTH | LAYOUT_FIX_HEIGHT, 0, 0, 56, 32, 0, 0, 0, 0);

    // Main color well
    well = new ColorWell(wellframe, FXRGBA(255, 255, 255, 255), this, ID_WELL_CHANGED, _COLORWELL_SOURCEONLY |
                         LAYOUT_TOP | LAYOUT_LEFT | LAYOUT_FILL_Y | LAYOUT_FIX_WIDTH, 0, 0, 64, 0, 0, 0, 0, 0);

    // Tab book with switchable panels
    panels = new FXTabBook(main, this, ID_ACTIVEPANE, TABBOOK_TOPTABS | LAYOUT_FILL_Y | LAYOUT_FILL_X);

    // HSV Dial Mode
    new FXTabItem(panels, _("\tHue, Saturation, Value"), dialmodeicon, TAB_TOP_NORMAL, 0, 0, 0, 0, 6, 6, 0, 0);

    // Color wheel
    FXHorizontalFrame* dialblock = new FXHorizontalFrame(panels, FRAME_GROOVE | LAYOUT_FILL_Y | LAYOUT_FILL_X |
                                                         LAYOUT_TOP | LAYOUT_LEFT, 0, 0, 0, 0, 15, 15, 5, 5, 5, 8);
    wheel = new FXColorRing(dialblock, this, ID_DIAL_WHEEL, LAYOUT_CENTER_Y | LAYOUT_FIX_WIDTH | LAYOUT_FIX_HEIGHT,
                            0, 0, 120, 120, 1, 1, 1, 1);

    // RGB Mode
    new FXTabItem(panels, _("\tRed, Green, Blue"), rgbmodeicon, TAB_TOP_NORMAL, 0, 0, 0, 0, 6, 6, 0, 0);

    // RGB Sliders
    FXMatrix* rgbblock = new FXMatrix(panels, 3, FRAME_GROOVE | LAYOUT_FILL_Y | LAYOUT_FILL_X | LAYOUT_TOP |
                                      LAYOUT_LEFT | MATRIX_BY_COLUMNS, 0, 0, 0, 0, 10, 10, 10, 10, 5, 8);

    // Red
    new FXLabel(rgbblock, _("&Red:"), NULL, LAYOUT_FILL_ROW | LAYOUT_CENTER_Y | LAYOUT_RIGHT);
    rgbatext[0] = new FXTextField(rgbblock, 5, this, ColorSelector::ID_RGB_RED_TEXT, JUSTIFY_RIGHT | LAYOUT_FILL_ROW |
                                  LAYOUT_CENTER_Y | TEXTFIELD_NORMAL, 0, 0, 0, 0, DEFAULT_PAD, DEFAULT_PAD, 0, 0);
    rgbaslider[0] = new FXSlider(rgbblock, this, ColorSelector::ID_RGB_RED_SLIDER,
                                 LAYOUT_FILL_ROW | LAYOUT_FILL_COLUMN |
                                 LAYOUT_CENTER_Y | LAYOUT_FILL_X | LAYOUT_FIX_HEIGHT | SLIDER_HORIZONTAL |
                                 SLIDER_INSIDE_BAR, 0, 0, 0, 15);

    // Green slider
    new FXLabel(rgbblock, _("&Green:"), NULL, LAYOUT_FILL_ROW | LAYOUT_CENTER_Y | LAYOUT_RIGHT);
    rgbatext[1] = new FXTextField(rgbblock, 5, this, ColorSelector::ID_RGB_GREEN_TEXT, JUSTIFY_RIGHT | LAYOUT_FILL_ROW |
                                  LAYOUT_CENTER_Y | TEXTFIELD_NORMAL, 0, 0, 0, 0, DEFAULT_PAD, DEFAULT_PAD, 0, 0);
    rgbaslider[1] = new FXSlider(rgbblock, this, ColorSelector::ID_RGB_GREEN_SLIDER,
                                 LAYOUT_FILL_ROW | LAYOUT_FILL_COLUMN |
                                 LAYOUT_CENTER_Y | LAYOUT_FILL_X | LAYOUT_FIX_HEIGHT | SLIDER_HORIZONTAL |
                                 SLIDER_INSIDE_BAR, 0, 0, 0, 15);

    // Blue slider
    new FXLabel(rgbblock, _("&Blue:"), NULL, LAYOUT_FILL_ROW | LAYOUT_CENTER_Y | LAYOUT_RIGHT);
    rgbatext[2] = new FXTextField(rgbblock, 5, this, ColorSelector::ID_RGB_BLUE_TEXT, JUSTIFY_RIGHT | LAYOUT_FILL_ROW |
                                  LAYOUT_CENTER_Y | TEXTFIELD_NORMAL, 0, 0, 0, 0, DEFAULT_PAD, DEFAULT_PAD, 0, 0);
    rgbaslider[2] = new FXSlider(rgbblock, this, ColorSelector::ID_RGB_BLUE_SLIDER,
                                 LAYOUT_FILL_ROW | LAYOUT_FILL_COLUMN |
                                 LAYOUT_CENTER_Y | LAYOUT_FILL_X | LAYOUT_FIX_HEIGHT | SLIDER_HORIZONTAL |
                                 SLIDER_INSIDE_BAR, 0, 0, 0, 15);

    // Alpha slider
    label = new FXLabel(rgbblock, _("&Alpha:"), NULL, LAYOUT_FILL_ROW | LAYOUT_CENTER_Y | LAYOUT_RIGHT);
    rgbatext[3] = new FXTextField(rgbblock, 5, this, ColorSelector::ID_ALPHA_TEXT, JUSTIFY_RIGHT | LAYOUT_FILL_ROW |
                                  LAYOUT_CENTER_Y | TEXTFIELD_NORMAL, 0, 0, 0, 0, DEFAULT_PAD, DEFAULT_PAD, 0, 0);
    rgbaslider[3] = new FXSlider(rgbblock, this, ColorSelector::ID_ALPHA_SLIDER, LAYOUT_FILL_ROW | LAYOUT_FILL_COLUMN |
                                 LAYOUT_CENTER_Y | LAYOUT_FILL_X | LAYOUT_FIX_HEIGHT | SLIDER_HORIZONTAL |
                                 SLIDER_INSIDE_BAR, 0, 0, 0, 15);

    label->setTarget(this);
    label->setSelector(ID_ALPHA_LABEL);

    // Set ranges and increment
    rgbaslider[0]->setRange(0, 255);
    rgbaslider[1]->setRange(0, 255);
    rgbaslider[2]->setRange(0, 255);
    rgbaslider[3]->setRange(0, 255);

    // HSV Mode
    new FXTabItem(panels, _("\tHue, Saturation, Value"), hsvmodeicon, TAB_TOP_NORMAL, 0, 0, 0, 0, 6, 6, 0, 0);

    // RGB Sliders
    FXMatrix* hsvblock = new FXMatrix(panels, 3, FRAME_GROOVE | LAYOUT_FILL_Y | LAYOUT_FILL_X | LAYOUT_TOP |
                                      LAYOUT_LEFT | MATRIX_BY_COLUMNS, 0, 0, 0, 0, 10, 10, 10, 10, 5, 8);

    // Hue Slider
    new FXLabel(hsvblock, _("Hue:"), NULL, LAYOUT_FILL_ROW | LAYOUT_CENTER_Y | LAYOUT_RIGHT);
    hsvatext[0] = new FXTextField(hsvblock, 5, this, ColorSelector::ID_HSV_HUE_TEXT, JUSTIFY_RIGHT | LAYOUT_FILL_ROW |
                                  LAYOUT_CENTER_Y | TEXTFIELD_NORMAL, 0, 0, 0, 0, DEFAULT_PAD, DEFAULT_PAD, 0, 0);
    hsvaslider[0] = new FXSlider(hsvblock, this, ColorSelector::ID_HSV_HUE_SLIDER,
                                 LAYOUT_FILL_ROW | LAYOUT_FILL_COLUMN |
                                 LAYOUT_CENTER_Y | LAYOUT_FILL_X | LAYOUT_FIX_HEIGHT | SLIDER_HORIZONTAL |
                                 SLIDER_INSIDE_BAR, 0, 0, 0, 15);

    // Saturation slider
    new FXLabel(hsvblock, _("Saturation:"), NULL, LAYOUT_FILL_ROW | LAYOUT_CENTER_Y | LAYOUT_RIGHT);
    hsvatext[1] = new FXTextField(hsvblock, 5, this, ColorSelector::ID_HSV_SATURATION_TEXT,
                                  JUSTIFY_RIGHT | LAYOUT_FILL_ROW |
                                  LAYOUT_CENTER_Y | TEXTFIELD_NORMAL, 0, 0, 0, 0, DEFAULT_PAD, DEFAULT_PAD, 0, 0);
    hsvaslider[1] = new FXSlider(hsvblock, this, ColorSelector::ID_HSV_SATURATION_SLIDER,
                                 LAYOUT_FILL_ROW | LAYOUT_FILL_COLUMN |
                                 LAYOUT_CENTER_Y | LAYOUT_FILL_X | LAYOUT_FIX_HEIGHT | SLIDER_HORIZONTAL |
                                 SLIDER_INSIDE_BAR, 0, 0, 0, 15);

    // Value slider
    new FXLabel(hsvblock, _("Value:"), NULL, LAYOUT_FILL_ROW | LAYOUT_CENTER_Y | LAYOUT_RIGHT);
    hsvatext[2] = new FXTextField(hsvblock, 5, this, ColorSelector::ID_HSV_VALUE_TEXT, JUSTIFY_RIGHT | LAYOUT_FILL_ROW |
                                  LAYOUT_CENTER_Y | TEXTFIELD_NORMAL, 0, 0, 0, 0, DEFAULT_PAD, DEFAULT_PAD, 0, 0);
    hsvaslider[2] = new FXSlider(hsvblock, this, ColorSelector::ID_HSV_VALUE_SLIDER,
                                 LAYOUT_FILL_ROW | LAYOUT_FILL_COLUMN |
                                 LAYOUT_CENTER_Y | LAYOUT_FILL_X | LAYOUT_FIX_HEIGHT | SLIDER_HORIZONTAL |
                                 SLIDER_INSIDE_BAR, 0, 0, 0, 15);

    // Alpha slider
    label = new FXLabel(hsvblock, _("Alpha:"), NULL, LAYOUT_FILL_ROW | LAYOUT_CENTER_Y | LAYOUT_RIGHT);
    hsvatext[3] = new FXTextField(hsvblock, 5, this, ColorSelector::ID_ALPHA_TEXT, JUSTIFY_RIGHT | LAYOUT_FILL_ROW |
                                  LAYOUT_CENTER_Y | TEXTFIELD_NORMAL, 0, 0, 0, 0, DEFAULT_PAD, DEFAULT_PAD, 0, 0);
    hsvaslider[3] = new FXSlider(hsvblock, this, ColorSelector::ID_ALPHA_SLIDER, LAYOUT_FILL_ROW | LAYOUT_FILL_COLUMN |
                                 LAYOUT_CENTER_Y | LAYOUT_FILL_X | LAYOUT_FIX_HEIGHT | SLIDER_HORIZONTAL |
                                 SLIDER_INSIDE_BAR, 0, 0, 0, 15);

    label->setTarget(this);
    label->setSelector(ID_ALPHA_LABEL);

    // Set ranges and increment
    hsvaslider[0]->setRange(0, 360);
    hsvaslider[1]->setRange(0, 1000);
    hsvaslider[2]->setRange(0, 1000);
    hsvaslider[3]->setRange(0, 255);

    // CMY Mode
    new FXTabItem(panels, _("\tCyan, Magenta, Yellow"), cmymodeicon, TAB_TOP_NORMAL, 0, 0, 0, 0, 6, 6, 0, 0);

    // RGB Sliders
    FXMatrix* cmyblock = new FXMatrix(panels, 3, FRAME_GROOVE | LAYOUT_FILL_Y | LAYOUT_FILL_X | LAYOUT_TOP |
                                      LAYOUT_LEFT | MATRIX_BY_COLUMNS, 0, 0, 0, 0, 10, 10, 10, 10, 5, 8);

    // Cyan Slider
    new FXLabel(cmyblock, _("Cyan:"), NULL, LAYOUT_FILL_ROW | LAYOUT_CENTER_Y | LAYOUT_RIGHT);
    cmytext[0] = new FXTextField(cmyblock, 5, this, ColorSelector::ID_CMY_CYAN_TEXT, JUSTIFY_RIGHT | LAYOUT_FILL_ROW |
                                 LAYOUT_CENTER_Y | TEXTFIELD_NORMAL, 0, 0, 0, 0, DEFAULT_PAD, DEFAULT_PAD, 0, 0);
    cmyslider[0] = new FXSlider(cmyblock, this, ColorSelector::ID_CMY_CYAN_SLIDER,
                                LAYOUT_FILL_ROW | LAYOUT_FILL_COLUMN |
                                LAYOUT_CENTER_Y | LAYOUT_FILL_X | LAYOUT_FIX_HEIGHT | SLIDER_HORIZONTAL |
                                SLIDER_INSIDE_BAR, 0, 0, 0, 15);

    // Magenta slider
    new FXLabel(cmyblock, _("Magenta:"), NULL, LAYOUT_FILL_ROW | LAYOUT_CENTER_Y | LAYOUT_RIGHT);
    cmytext[1] = new FXTextField(cmyblock, 5, this, ColorSelector::ID_CMY_MAGENTA_TEXT,
                                 JUSTIFY_RIGHT | LAYOUT_FILL_ROW |
                                 LAYOUT_CENTER_Y | TEXTFIELD_NORMAL, 0, 0, 0, 0, DEFAULT_PAD, DEFAULT_PAD, 0, 0);
    cmyslider[1] = new FXSlider(cmyblock, this, ColorSelector::ID_CMY_MAGENTA_SLIDER,
                                LAYOUT_FILL_ROW | LAYOUT_FILL_COLUMN |
                                LAYOUT_CENTER_Y | LAYOUT_FILL_X | LAYOUT_FIX_HEIGHT | SLIDER_HORIZONTAL |
                                SLIDER_INSIDE_BAR, 0, 0, 0, 15);

    // Yellow slider
    new FXLabel(cmyblock, _("Yellow:"), NULL, LAYOUT_FILL_ROW | LAYOUT_CENTER_Y | LAYOUT_RIGHT);
    cmytext[2] = new FXTextField(cmyblock, 5, this, ColorSelector::ID_CMY_YELLOW_TEXT, JUSTIFY_RIGHT | LAYOUT_FILL_ROW |
                                 LAYOUT_CENTER_Y | TEXTFIELD_NORMAL, 0, 0, 0, 0, DEFAULT_PAD, DEFAULT_PAD, 0, 0);
    cmyslider[2] = new FXSlider(cmyblock, this, ColorSelector::ID_CMY_YELLOW_SLIDER,
                                LAYOUT_FILL_ROW | LAYOUT_FILL_COLUMN |
                                LAYOUT_CENTER_Y | LAYOUT_FILL_X | LAYOUT_FIX_HEIGHT | SLIDER_HORIZONTAL |
                                SLIDER_INSIDE_BAR, 0, 0, 0, 15);

    // Alpha slider
    label = new FXLabel(cmyblock, _("Alpha:"), NULL, LAYOUT_FILL_ROW | LAYOUT_CENTER_Y | LAYOUT_RIGHT);
    cmytext[3] = new FXTextField(cmyblock, 5, this, ColorSelector::ID_ALPHA_TEXT, JUSTIFY_RIGHT | LAYOUT_FILL_ROW |
                                 LAYOUT_CENTER_Y | TEXTFIELD_NORMAL, 0, 0, 0, 0, DEFAULT_PAD, DEFAULT_PAD, 0, 0);
    cmyslider[3] = new FXSlider(cmyblock, this, ColorSelector::ID_ALPHA_SLIDER, LAYOUT_FILL_ROW | LAYOUT_FILL_COLUMN |
                                LAYOUT_CENTER_Y | LAYOUT_FILL_X | LAYOUT_FIX_HEIGHT | SLIDER_HORIZONTAL |
                                SLIDER_INSIDE_BAR, 0, 0, 0, 15);

    label->setTarget(this);
    label->setSelector(ID_ALPHA_LABEL);

    // Set ranges and increment
    cmyslider[0]->setRange(0, 255);
    cmyslider[1]->setRange(0, 255);
    cmyslider[2]->setRange(0, 255);
    cmyslider[3]->setRange(0, 255);

    // Named Color Mode
    new FXTabItem(panels, _("\tBy Name"), listmodeicon, TAB_TOP_NORMAL, 0, 0, 0, 0, 6, 6, 0, 0);

    // Name list
    FXHorizontalFrame* outer = new FXHorizontalFrame(panels, FRAME_GROOVE | LAYOUT_FILL_Y | LAYOUT_FILL_X);
    FXHorizontalFrame* frame = new FXHorizontalFrame(outer, LAYOUT_FILL_Y | LAYOUT_FILL_X | FRAME_NONE, 0, 0, 0, 0, 0,
                                                     0, 0, 0);
    list = new FXColorList(frame, this, ID_COLOR_LIST, LAYOUT_FILL_Y | LAYOUT_FILL_X | LIST_BROWSESELECT);
    list->setNumVisible(6);

    // Add color names
    for (FXuint i = 0; i < fxnumcolornames; i++)
    {
        list->appendItem(tr(fxcolornames[i].name), fxcolornames[i].color);
    }

    // Init RGBA
    rgba[0] = 0.0;
    rgba[1] = 0.0;
    rgba[2] = 0.0;
    rgba[3] = 1.0;

    // Init HSVA
    hsva[0] = 360.0;
    hsva[1] = 0.0;
    hsva[2] = 0.0;
    hsva[3] = 1.0;

    // Reflect color in well
    updateWell();

    // Initial focus on accept button
    accept->setFocus();
}


// Init the panel
void ColorSelector::create()
{
    FXPacker::create();

    // Get custom well colors from defaults database
    colorwells[0]->setRGBA(getApp()->reg().readColorEntry("SETTINGS", wellname[0], FXRGBA(255, 255, 255, 255)));
    colorwells[1]->setRGBA(getApp()->reg().readColorEntry("SETTINGS", wellname[1], FXRGBA(0, 0, 0, 255)));
    colorwells[2]->setRGBA(getApp()->reg().readColorEntry("SETTINGS", wellname[2], FXRGBA(255, 0, 0, 255)));
    colorwells[3]->setRGBA(getApp()->reg().readColorEntry("SETTINGS", wellname[3], FXRGBA(0, 255, 0, 255)));
    colorwells[4]->setRGBA(getApp()->reg().readColorEntry("SETTINGS", wellname[4], FXRGBA(0, 0, 255, 255)));
    colorwells[5]->setRGBA(getApp()->reg().readColorEntry("SETTINGS", wellname[5], FXRGBA(0, 0, 255, 255)));
    colorwells[6]->setRGBA(getApp()->reg().readColorEntry("SETTINGS", wellname[6], FXRGBA(255, 255, 0, 255)));
    colorwells[7]->setRGBA(getApp()->reg().readColorEntry("SETTINGS", wellname[7], FXRGBA(0, 255, 255, 255)));
    colorwells[8]->setRGBA(getApp()->reg().readColorEntry("SETTINGS", wellname[8], FXRGBA(255, 0, 255, 255)));
    colorwells[9]->setRGBA(getApp()->reg().readColorEntry("SETTINGS", wellname[9], FXRGBA(128, 0, 0, 255)));
    colorwells[10]->setRGBA(getApp()->reg().readColorEntry("SETTINGS", wellname[10], FXRGBA(0, 128, 0, 255)));
    colorwells[11]->setRGBA(getApp()->reg().readColorEntry("SETTINGS", wellname[11], FXRGBA(0, 0, 128, 255)));
    colorwells[12]->setRGBA(getApp()->reg().readColorEntry("SETTINGS", wellname[12], FXRGBA(128, 128, 0, 255)));
    colorwells[13]->setRGBA(getApp()->reg().readColorEntry("SETTINGS", wellname[13], FXRGBA(128, 0, 128, 255)));
    colorwells[14]->setRGBA(getApp()->reg().readColorEntry("SETTINGS", wellname[14], FXRGBA(0, 128, 128, 255)));
    colorwells[15]->setRGBA(getApp()->reg().readColorEntry("SETTINGS", wellname[15], FXRGBA(0, 128, 128, 255)));
    colorwells[16]->setRGBA(getApp()->reg().readColorEntry("SETTINGS", wellname[16], FXRGBA(255, 0, 255, 255)));
    colorwells[17]->setRGBA(getApp()->reg().readColorEntry("SETTINGS", wellname[17], FXRGBA(128, 0, 0, 255)));
    colorwells[18]->setRGBA(getApp()->reg().readColorEntry("SETTINGS", wellname[18], FXRGBA(0, 128, 0, 255)));
    colorwells[19]->setRGBA(getApp()->reg().readColorEntry("SETTINGS", wellname[19], FXRGBA(0, 0, 128, 255)));
    colorwells[20]->setRGBA(getApp()->reg().readColorEntry("SETTINGS", wellname[20], FXRGBA(128, 128, 0, 255)));
    colorwells[21]->setRGBA(getApp()->reg().readColorEntry("SETTINGS", wellname[21], FXRGBA(128, 0, 128, 255)));
    colorwells[22]->setRGBA(getApp()->reg().readColorEntry("SETTINGS", wellname[22], FXRGBA(0, 128, 128, 255)));
    colorwells[23]->setRGBA(getApp()->reg().readColorEntry("SETTINGS", wellname[23], FXRGBA(0, 128, 128, 255)));

    // Switch to correct pane
    panels->setCurrent(getApp()->reg().readIntEntry("SETTINGS", "activecolorpane", 0));
}


// ALPHA

// Update well from Alpha slider
long ColorSelector::onCmdAlphaSlider(FXObject* sender, FXSelector sel, void*)
{
    FXint value;

    sender->handle(this, FXSEL(SEL_COMMAND, ID_GETINTVALUE), (void*)&value);
    hsva[3] = rgba[3] = 0.003921568627f * value;
    updateWell();
    if (target)
    {
        target->tryHandle(this, FXSEL(FXSELTYPE(sel), message), (void*)(FXuval)well->getRGBA());
    }
    return 1;
}


// Update well from Alpha text fields
long ColorSelector::onCmdAlphaText(FXObject* sender, FXSelector, void*)
{
    FXdouble value;

    sender->handle(this, FXSEL(SEL_COMMAND, ID_GETREALVALUE), (void*)&value);
    hsva[3] = rgba[3] = 0.003921568627f * (FXfloat)value;
    updateWell();
    if (target)
    {
        target->tryHandle(this, FXSEL(SEL_COMMAND, message), (void*)(FXuval)well->getRGBA());
    }
    return 1;
}


// Update Alpha text fields
long ColorSelector::onUpdAlphaText(FXObject* sender, FXSelector, void*)
{
    if (isOpaqueOnly())
    {
        sender->handle(this, FXSEL(SEL_COMMAND, ID_HIDE), NULL);
    }
    else
    {
        FXString value = FXStringVal(255.0 * rgba[3], 1, FALSE);
        sender->handle(this, FXSEL(SEL_COMMAND, ID_SETSTRINGVALUE), (void*)&value);
        sender->handle(this, FXSEL(SEL_COMMAND, ID_SHOW), NULL);
    }
    return 1;
}


// Update Alpha sliders
long ColorSelector::onUpdAlphaSlider(FXObject* sender, FXSelector, void*)
{
    if (isOpaqueOnly())
    {
        sender->handle(this, FXSEL(SEL_COMMAND, ID_HIDE), NULL);
    }
    else
    {
        FXint value = (FXint)(255.0 * rgba[3]);
        sender->handle(this, FXSEL(SEL_COMMAND, ID_SETINTVALUE), (void*)&value);
        sender->handle(this, FXSEL(SEL_COMMAND, ID_SHOW), NULL);
    }
    return 1;
}



// Update Alpha Labels
long ColorSelector::onUpdAlphaLabel(FXObject* sender, FXSelector, void*)
{
    sender->handle(this, isOpaqueOnly()?FXSEL(SEL_COMMAND, ID_HIDE):FXSEL(SEL_COMMAND, ID_SHOW), NULL);
    return 1;
}



// Wheel

// Update well from wheel
long ColorSelector::onCmdWheel(FXObject*, FXSelector sel, void*)
{
    hsva[0] = wheel->getHue();
    hsva[1] = wheel->getSat();
    hsva[2] = wheel->getVal();
    fxhsv_to_rgb(rgba[0], rgba[1], rgba[2], hsva[0], hsva[1], hsva[2]);
    updateWell();
    if (target)
    {
        target->tryHandle(this, FXSEL(FXSELTYPE(sel), message), (void*)(FXuval)well->getRGBA());
    }
    return 1;
}


// Update wheel
long ColorSelector::onUpdWheel(FXObject*, FXSelector, void*)
{
    wheel->setHue(hsva[0]);
    wheel->setSat(hsva[1]);
    wheel->setVal(hsva[2]);
    return 1;
}



// RGB

// Update well from RGB slider
long ColorSelector::onCmdRGBSlider(FXObject*, FXSelector sel, void*)
{
    FXint which = FXSELID(sel) - ID_RGB_RED_SLIDER;

    rgba[which] = 0.003921568627f * rgbaslider[which]->getValue();
    fxrgb_to_hsv(hsva[0], hsva[1], hsva[2], rgba[0], rgba[1], rgba[2]);
    updateWell();
    if (target)
    {
        target->tryHandle(this, FXSEL(FXSELTYPE(sel), message), (void*)(FXuval)well->getRGBA());
    }
    return 1;
}


// Update well from RGB text fields
long ColorSelector::onCmdRGBText(FXObject*, FXSelector sel, void*)
{
    FXint which = FXSELID(sel) - ID_RGB_RED_TEXT;

    rgba[which] = 0.003921568627f * FXFloatVal(rgbatext[which]->getText());
    fxrgb_to_hsv(hsva[0], hsva[1], hsva[2], rgba[0], rgba[1], rgba[2]);
    updateWell();
    if (target)
    {
        target->tryHandle(this, FXSEL(SEL_COMMAND, message), (void*)(FXuval)well->getRGBA());
    }
    return 1;
}


// Update RGB text fields
long ColorSelector::onUpdRGBText(FXObject*, FXSelector sel, void*)
{
    FXint which = FXSELID(sel) - ID_RGB_RED_TEXT;

    rgbatext[which]->setText(FXStringVal(255.0f * rgba[which], 1, FALSE));
    return 1;
}


// Update RGB sliders
long ColorSelector::onUpdRGBSlider(FXObject*, FXSelector sel, void*)
{
    FXint which = FXSELID(sel) - ID_RGB_RED_SLIDER;

    rgbaslider[which]->setValue((FXint)(255.0f * rgba[which]));
    return 1;
}



// HSV

// Update well from HSV sliders
long ColorSelector::onCmdHSVSlider(FXObject*, FXSelector sel, void*)
{
    const FXfloat factor[3] = { 1.0f, 0.001f, 0.001f };
    FXint which = FXSELID(sel) - ID_HSV_HUE_SLIDER;

    hsva[which] = factor[which] * hsvaslider[which]->getValue();
    fxhsv_to_rgb(rgba[0], rgba[1], rgba[2], hsva[0], hsva[1], hsva[2]);
    updateWell();
    if (target)
    {
        target->tryHandle(this, FXSEL(FXSELTYPE(sel), message), (void*)(FXuval)well->getRGBA());
    }
    return 1;
}


// Update well from HSV text fields
long ColorSelector::onCmdHSVText(FXObject*, FXSelector sel, void*)
{
    const FXfloat factor[3] = { 1.0f, 0.01f, 0.01f };
    FXint which = FXSELID(sel) - ID_HSV_HUE_TEXT;

    hsva[which] = factor[which] * FXFloatVal(hsvatext[which]->getText());
    fxhsv_to_rgb(rgba[0], rgba[1], rgba[2], hsva[0], hsva[1], hsva[2]);
    updateWell();
    if (target)
    {
        target->tryHandle(this, FXSEL(SEL_COMMAND, message), (void*)(FXuval)well->getRGBA());
    }
    return 1;
}


// Update HSV text fields
long ColorSelector::onUpdHSVText(FXObject*, FXSelector sel, void*)
{
    const FXfloat factor[3] = { 1.0f, 100.0f, 100.0f };
    FXint which = FXSELID(sel) - ID_HSV_HUE_TEXT;

    hsvatext[which]->setText(FXStringVal(hsva[which] * factor[which], 1, FALSE));
    return 1;
}


// Update HSV sliders
long ColorSelector::onUpdHSVSlider(FXObject*, FXSelector sel, void*)
{
    const FXfloat factor[3] = { 1.0f, 1000.0f, 1000.0f };
    FXint which = FXSELID(sel) - ID_HSV_HUE_SLIDER;

    hsvaslider[which]->setValue((FXint)(hsva[which] * factor[which]));
    return 1;
}


// CMY

long ColorSelector::onCmdCMYSlider(FXObject*, FXSelector sel, void*)
{
    FXint which = FXSELID(sel) - ID_CMY_CYAN_SLIDER;
    FXfloat val = 0.003921568627f * cmyslider[which]->getValue();

    rgba[which] = 1.0f - val;
    fxrgb_to_hsv(hsva[0], hsva[1], hsva[2], rgba[0], rgba[1], rgba[2]);
    hsva[3] = rgba[3];
    updateWell();
    if (target)
    {
        target->tryHandle(this, FXSEL(FXSELTYPE(sel), message), (void*)(FXuval)well->getRGBA());
    }
    return 1;
}


long ColorSelector::onCmdCMYText(FXObject*, FXSelector sel, void*)
{
    FXint which = FXSELID(sel) - ID_CMY_CYAN_TEXT;
    FXfloat val = 0.003921568627f * FXFloatVal(cmytext[which]->getText());

    rgba[which] = 1.0f - val;
    fxrgb_to_hsv(hsva[0], hsva[1], hsva[2], rgba[0], rgba[1], rgba[2]);
    hsva[3] = rgba[3];
    updateWell();
    if (target)
    {
        target->tryHandle(this, FXSEL(SEL_COMMAND, message), (void*)(FXuval)well->getRGBA());
    }
    return 1;
}


long ColorSelector::onUpdCMYText(FXObject*, FXSelector sel, void*)
{
    FXint which = FXSELID(sel) - ID_CMY_CYAN_TEXT;
    FXfloat val = 255.0f - 255.0f * rgba[which];

    cmytext[which]->setText(FXStringVal(val, 1, FALSE));
    return 1;
}


long ColorSelector::onUpdCMYSlider(FXObject*, FXSelector sel, void*)
{
    FXint which = FXSELID(sel) - ID_CMY_CYAN_SLIDER;
    FXint val = (FXint)(255.0f - 255.0f * rgba[which]);

    cmyslider[which]->setValue(val);
    return 1;
}


// Color picker
long ColorSelector::onCmdColorPick(FXObject*, FXSelector, void* ptr)
{
    FXPoint* point = (FXPoint*)ptr;
    FXDCWindow dc(getRoot());

    setRGBA(dc.readPixel(point->x, point->y));
    if (target)
    {
        target->tryHandle(this, FXSEL(SEL_COMMAND, message), (void*)(FXuval)well->getRGBA());
    }
    return 1;
}



// Dropped color in main well
long ColorSelector::onChgWell(FXObject*, FXSelector, void* ptr)
{
    FXColor color = (FXColor)(FXuval)ptr;

    if (isOpaqueOnly())
    {
        color |= FXRGBA(0, 0, 0, 255);
    }
    rgba[0] = 0.003921568627f * FXREDVAL(color);
    rgba[1] = 0.003921568627f * FXGREENVAL(color);
    rgba[2] = 0.003921568627f * FXBLUEVAL(color);
    rgba[3] = 0.003921568627f * FXALPHAVAL(color);
    fxrgb_to_hsv(hsva[0], hsva[1], hsva[2], rgba[0], rgba[1], rgba[2]);
    hsva[3] = rgba[3];
    return 1;
}


// Command from main well
long ColorSelector::onCmdWell(FXObject*, FXSelector, void*)
{
    if (target)
    {
        target->tryHandle(this, FXSEL(SEL_COMMAND, message), (void*)(FXuval)well->getRGBA());
    }
    return 1;
}


// Update main well
void ColorSelector::updateWell()
{
    well->setRGBA(FXRGBA((int)(rgba[0] * 255.0), (int)(rgba[1] * 255.0), (int)(rgba[2] * 255.0),
                         (int)(rgba[3] * 255.0)));
}



// Clicked on color in list
long ColorSelector::onCmdList(FXObject*, FXSelector, void* ptr)
{
    FXint index = (FXint)(FXuval)ptr;
    FXColor clr = fxcolornames[index].color;

    if (isOpaqueOnly())
    {
        clr |= FXRGBA(0, 0, 0, 255);
    }
    rgba[0] = 0.003921568627f * FXREDVAL(clr);
    rgba[1] = 0.003921568627f * FXGREENVAL(clr);
    rgba[2] = 0.003921568627f * FXBLUEVAL(clr);
    rgba[3] = 0.003921568627f * FXALPHAVAL(clr);
    fxrgb_to_hsv(hsva[0], hsva[1], hsva[2], rgba[0], rgba[1], rgba[2]);
    hsva[3] = rgba[3];
    updateWell();
    if (target)
    {
        target->tryHandle(this, FXSEL(SEL_COMMAND, message), (void*)(FXuval)well->getRGBA());
    }
    return 1;
}



// Custom well changed
long ColorSelector::onChgCustomWell(FXObject*, FXSelector sel, void* ptr)
{
    FXColor color = (FXColor)(FXuval)ptr;
    FXuint which = FXSELID(sel) - ID_CUSTOM_FIRST;

    FXASSERT(which < 24);
    getApp()->reg().writeColorEntry("SETTINGS", wellname[which], color);
    return 1;
}


// Custom well clicked
long ColorSelector::onCmdCustomWell(FXObject*, FXSelector, void* ptr)
{
    FXColor color = (FXColor)(FXuval)ptr;

    if (isOpaqueOnly())
    {
        color |= FXRGBA(0, 0, 0, 255);
    }
    setRGBA(color);
    if (target)
    {
        target->tryHandle(this, FXSEL(SEL_COMMAND, message), (void*)(FXuval)well->getRGBA());
    }
    return 1;
}


// Switching active pane
long ColorSelector::onCmdActivePane(FXObject*, FXSelector, void* ptr)
{
    getApp()->reg().writeIntEntry("SETTINGS", "activecolorpane", (FXint)(FXival)ptr);
    return 1;
}



// Set color
long ColorSelector::onCmdSetValue(FXObject*, FXSelector, void* ptr)
{
    setRGBA((FXColor)(FXuval)ptr);
    return 1;
}


// Change RGBA color
void ColorSelector::setRGBA(FXColor clr)
{
    if (clr != well->getRGBA())
    {
        rgba[0] = 0.003921568627f * FXREDVAL(clr);
        rgba[1] = 0.003921568627f * FXGREENVAL(clr);
        rgba[2] = 0.003921568627f * FXBLUEVAL(clr);
        rgba[3] = 0.003921568627f * FXALPHAVAL(clr);
        fxrgb_to_hsv(hsva[0], hsva[1], hsva[2], rgba[0], rgba[1], rgba[2]);
        hsva[3] = rgba[3];
        well->setRGBA(clr);
    }
}


// Retrieve RGBA color
FXColor ColorSelector::getRGBA() const
{
    return well->getRGBA();
}


// Return true if only opaque colors allowed
FXbool ColorSelector::isOpaqueOnly() const
{
    return well->isOpaqueOnly();
}


// Change opaque only mode
void ColorSelector::setOpaqueOnly(FXbool opaque)
{
    if (opaque)
    {
        well->setOpaqueOnly(TRUE);
        setRGBA(well->getRGBA() | FXRGBA(0, 0, 0, 255));
    }
    else
    {
        well->setOpaqueOnly(FALSE);
    }
}



// Save data
void ColorSelector::save(FXStream& store) const
{
    FXPacker::save(store);
    store << panels;
    store << well;
    store << list;
    store << accept;
    store << cancel;
    store << wheel;
    store << rgbaslider[0] << rgbaslider[1] << rgbaslider[2] << rgbaslider[3];
    store << hsvaslider[0] << hsvaslider[1] << hsvaslider[2] << hsvaslider[3];
    store << cmyslider[0] << cmyslider[1] << cmyslider[2] << cmyslider[3];
    store << rgbatext[0] << rgbatext[1] << rgbatext[2] << rgbatext[3];
    store << hsvatext[0] << hsvatext[1] << hsvatext[2] << hsvatext[3];
    store << cmytext[0] << cmytext[1] << cmytext[2] << cmytext[3];
    store << colorwells[0] << colorwells[1] << colorwells[2] << colorwells[3];
    store << colorwells[4] << colorwells[5] << colorwells[6] << colorwells[7];
    store << colorwells[8] << colorwells[9] << colorwells[10] << colorwells[11];
    store << colorwells[12] << colorwells[13] << colorwells[14] << colorwells[15];
    store << colorwells[16] << colorwells[17] << colorwells[18] << colorwells[19];
    store << colorwells[20] << colorwells[21] << colorwells[22] << colorwells[23];
    store.save(rgba, 4);
    store.save(hsva, 4);
}


// Load data
void ColorSelector::load(FXStream& store)
{
    FXPacker::load(store);
    store >> panels;
    store >> well;
    store >> list;
    store >> accept;
    store >> cancel;
    store >> wheel;
    store >> rgbaslider[0] >> rgbaslider[1] >> rgbaslider[2] >> rgbaslider[3];
    store >> hsvaslider[0] >> hsvaslider[1] >> hsvaslider[2] >> hsvaslider[3];
    store >> cmyslider[0] >> cmyslider[1] >> cmyslider[2] >> cmyslider[3];
    store >> rgbatext[0] >> rgbatext[1] >> rgbatext[2] >> rgbatext[3];
    store >> hsvatext[0] >> hsvatext[1] >> hsvatext[2] >> hsvatext[3];
    store >> cmytext[0] >> cmytext[1] >> cmytext[2] >> cmytext[3];
    store >> colorwells[0] >> colorwells[1] >> colorwells[2] >> colorwells[3];
    store >> colorwells[4] >> colorwells[5] >> colorwells[6] >> colorwells[7];
    store >> colorwells[8] >> colorwells[9] >> colorwells[10] >> colorwells[11];
    store >> colorwells[12] >> colorwells[13] >> colorwells[14] >> colorwells[15];
    store >> colorwells[16] >> colorwells[17] >> colorwells[18] >> colorwells[19];
    store >> colorwells[20] >> colorwells[21] >> colorwells[22] >> colorwells[23];
    store.load(rgba, 4);
    store.load(hsva, 4);
}


// Cleanup; icons must be explicitly deleted
ColorSelector::~ColorSelector()
{
    wheel = (FXColorRing*)-1L;
    panels = (FXTabBook*)-1L;
    well = (ColorWell*)-1L;
    accept = (FXButton*)-1L;
    cancel = (FXButton*)-1L;
}



//
// ColorDialog class
//



// Map
FXDEFMAP(ColorDialog) ColorDialogMap[] =
{
    FXMAPFUNC(SEL_CHANGED, ColorDialog::ID_COLORSELECTOR, ColorDialog::onChgColor),
    FXMAPFUNC(SEL_COMMAND, ColorDialog::ID_COLORSELECTOR, ColorDialog::onCmdColor),
};


// Object implementation
FXIMPLEMENT(ColorDialog, FXDialogBox, ColorDialogMap, ARRAYNUMBER(ColorDialogMap))



// Color dialog
ColorDialog::ColorDialog(FXWindow* owner, const FXString& name, FXuint opts, FXint x, FXint y, FXint w, FXint h) :
    FXDialogBox(owner, name, opts | DECOR_TITLE | DECOR_BORDER | DECOR_STRETCHABLE | DECOR_CLOSE,
                x, y, w, h, 0, 0, 0, 0, 4, 4)
{
    colorbox = new ColorSelector(this, this, ID_COLORSELECTOR, LAYOUT_FILL_X | LAYOUT_FILL_Y);
    colorbox->acceptButton()->setTarget(this);
    colorbox->acceptButton()->setSelector(FXDialogBox::ID_ACCEPT);
    colorbox->cancelButton()->setTarget(this);
    colorbox->cancelButton()->setSelector(FXDialogBox::ID_CANCEL);
}


// Change RGBA color
void ColorDialog::setRGBA(FXColor clr)
{
    colorbox->setRGBA(clr);
}


// Retrieve RGBA color
FXColor ColorDialog::getRGBA() const
{
    return colorbox->getRGBA();
}


// Forward ColorSelector color change to target [a color well]
long ColorDialog::onChgColor(FXObject*, FXSelector, void* ptr)
{
    return target && target->tryHandle(this, FXSEL(SEL_CHANGED, message), ptr);
}


// Forward ColorSelector color command to target [a color well]
long ColorDialog::onCmdColor(FXObject*, FXSelector, void* ptr)
{
    return target && target->tryHandle(this, FXSEL(SEL_COMMAND, message), ptr);
}


// Return true if only opaque colors allowed
FXbool ColorDialog::isOpaqueOnly() const
{
    return colorbox->isOpaqueOnly();
}


// Change opaque only mode
void ColorDialog::setOpaqueOnly(FXbool forceopaque)
{
    colorbox->setOpaqueOnly(forceopaque);
}


// Save data
void ColorDialog::save(FXStream& store) const
{
    FXDialogBox::save(store);
    store << colorbox;
}


// Load data
void ColorDialog::load(FXStream& store)
{
    FXDialogBox::load(store);
    store >> colorbox;
}


// Cleanup
ColorDialog::~ColorDialog()
{
    colorbox = (ColorSelector*)-1L;
}
