// This file contains FOX functions redefinitions
// These are used to implement modern controls (rounded buttons with gradients and other effects)
// Modern controls are obtained mainly by redefining the onPaint() functions for various widgets


// Integer UI scaling factor
extern FXint scaleint;


// Threshold used to check if dark theme
#define DARK_COLOR_THRESHOLD    125


//
// Specific functions and macros
//

// Draw rectangle with gradient effect
// Default is vertical gradient
static void drawGradientRectangle(FXDC& dc, FXColor upper, FXColor lower, int x, int y, int w, int h,
                                  FXbool vert = true)
{
    int rr, gg, bb, dr, dg, db, r1, g1, b1, r2, g2, b2, yl, yh, yy, dy, n, t, ww;
    const int MAXSTEPS = 128;

    if ((0 < w) && (0 < h))
    {
        // Horizontal gradient : exchange w and h
        if (!vert)
        {
            ww = w;
            w = h;
            h = ww;
        }

        dc.setStipple(STIPPLE_NONE);
        dc.setFillStyle(FILL_SOLID);

        r1 = FXREDVAL(upper);
        r2 = FXREDVAL(lower);
        dr = r2 - r1;
        g1 = FXGREENVAL(upper);
        g2 = FXGREENVAL(lower);
        dg = g2 - g1;
        b1 = FXBLUEVAL(upper);
        b2 = FXBLUEVAL(lower);
        db = b2 - b1;

        n = FXABS(dr);
        if ((t = FXABS(dg)) > n)
        {
            n = t;
        }
        if ((t = FXABS(db)) > n)
        {
            n = t;
        }
        n++;
        if (n > h)
        {
            n = h;
        }
        if (n > MAXSTEPS)
        {
            n = MAXSTEPS;
        }
        rr = (r1 << 16) + 32767;
        gg = (g1 << 16) + 32767;
        bb = (b1 << 16) + 32767;
        yy = 32767;

        dr = (dr << 16) / n;
        dg = (dg << 16) / n;
        db = (db << 16) / n;
        dy = (h << 16) / n;

        do
        {
            yl = yy >> 16;
            yy += dy;
            yh = yy >> 16;
            dc.setForeground(FXRGB(rr >> 16, gg >> 16, bb >> 16));

            // Vertical gradient
            if (vert)
            {
                dc.fillRectangle(x, y + yl, w, yh - yl);
            }
            // Horizontal gradient
            else
            {
                dc.fillRectangle(x + yl, y, yh - yl, w);
            }

            rr += dr;
            gg += dg;
            bb += db;
        }
        while (yh < h);
    }
}


// These macros are used to simplify the code
// They draw a button with gradient effect in up or down state

#define DRAW_MODERN_BUTTON_UP                                                                              \
        dc.setForeground(backColor);                                                                       \
        dc.drawPoints(basebackground, 4);                                                                  \
                                                                                                           \
        dc.setForeground(bordercolor);                                                                     \
        dc.drawRectangle(2, 0, width - 5, 0);                                                              \
        dc.drawRectangle(2, height - 1, width - 5, height - 1);                                            \
        dc.drawRectangle(0, 2, 0, height - 5);                                                             \
        dc.drawRectangle(width - 1, 2, 0, height - 5);                                                     \
        dc.drawPoints(bordercorners, 4);                                                                   \
        dc.setForeground(shadecolor);                                                                      \
        dc.drawPoints(bordershade, 16);                                                                    \
                                                                                                           \
        drawGradientRectangle(dc, topcolor, bottomcolor, 2, 1, width - 4, height - 2);                     \
        dc.setForeground(topcolor);                                                                        \
        dc.drawRectangle(1, 3, 0, height - 7);                                                             \
        dc.setForeground(bottomcolor);                                                                     \
        dc.drawRectangle(width - 2, 3, 0, height - 7);


#define DRAW_MODERN_BUTTON_DOWN                                                                            \
        dc.setForeground(shadecolor);                                                                      \
        dc.fillRectangle(0, 0, width, height);                                                             \
                                                                                                           \
        dc.setForeground(backColor);                                                                       \
        dc.drawPoints(basebackground, 4);                                                                  \
                                                                                                           \
        dc.setForeground(bordercolor);                                                                     \
        dc.drawRectangle(2, 0, width - 5, 0);                                                              \
        dc.drawRectangle(2, height - 1, width - 5, height - 1);                                            \
        dc.drawRectangle(0, 2, 0, height - 5);                                                             \
        dc.drawRectangle(width - 1, 2, 0, height - 5);                                                     \
        dc.drawPoints(bordercorners, 4);                                                                   \
        dc.setForeground(shadecolor);                                                                      \
        dc.drawPoints(bordershade, 16);


#define DRAW_MODERN_BUTTON_DOWN_NO_BORDER                                                                  \
        FXuint rk = FXREDVAL(backColor);                                                                   \
        FXuint gk = FXGREENVAL(backColor);                                                                 \
        FXuint bk = FXBLUEVAL(backColor);                                                                  \
                                                                                                           \
        if ((rk + gk + bk) / 3 < DARK_COLOR_THRESHOLD)                                                     \
        {                                                                                                  \
            shadecolor = FXRGB(FXMIN(1.3 * rk, 255), FXMIN(1.3 * gk, 255), FXMIN(1.3 * bk, 255));          \
        }                                                                                                  \
        dc.setForeground(shadecolor);                                                                      \
        dc.fillRectangle(0, 0, width, height);


#define INIT_MODERN                                                                                        \
        static FXbool init = true;                                                                         \
        static FXColor topcolor, bottomcolor, shadecolor, bordercolor;                                     \
                                                                                                           \
        FXPoint basebackground[4] = { FXPoint(0, 0), FXPoint(width - 1, 0), FXPoint(0, height - 1),        \
                                      FXPoint(width - 1, height - 1) };                                    \
        FXPoint bordershade[16] = { FXPoint(0, 1), FXPoint(1, 0), FXPoint(1, 2), FXPoint(2, 1),            \
                                    FXPoint(width - 2, 0), FXPoint(width - 1, 1), FXPoint(width - 3, 1),   \
                                    FXPoint(width - 2, 2), FXPoint(0, height - 2), FXPoint(1, height - 1), \
                                    FXPoint(1, height - 3), FXPoint(2, height - 2),                        \
                                    FXPoint(width - 1, height - 2), FXPoint(width - 2, height - 1),        \
                                    FXPoint(width - 2, height - 3), FXPoint(width - 3, height - 2)         \
        };                                                                                                 \
        FXPoint bordercorners[4] = { FXPoint(1, 1), FXPoint(1, height - 2), FXPoint(width - 2, 1),         \
                                     FXPoint(width - 2, height - 2) };                                     \
                                                                                                           \
        if (init)                                                                                          \
        {                                                                                                  \
            FXuint r = FXREDVAL(baseColor);                                                                \
            FXuint g = FXGREENVAL(baseColor);                                                              \
            FXuint b = FXBLUEVAL(baseColor);                                                               \
                                                                                                           \
            topcolor = FXRGB(FXMIN(1.1 * r, 255), FXMIN(1.1 * g, 255), FXMIN(1.1 * b, 255));               \
            (void)topcolor;             /* Trick to avoid unused variable compiler warning */              \
            bottomcolor = FXRGB(0.9 * r, 0.9 * g, 0.9 * b);                                                \
            (void)bottomcolor;          /* Trick to avoid unused variable compiler warning */              \
            shadecolor = FXRGB(0.9 * r, 0.9 * g, 0.9 * b);                                                 \
            bordercolor = FXRGB(0.5 * r, 0.5 * g, 0.5 * b);                                                \
                                                                                                           \
            init = false;                                                                                  \
        }



//
// Hack of FXButton (button with gradient effect and rounded corners)
// Original author : Sander Jansen <sander@knology.net>
//


// Handle repaint
long FXButton::onPaint(FXObject*, FXSelector, void* ptr)
{
    // Initialize modern controls
    INIT_MODERN

    FXEvent* ev = (FXEvent*)ptr;
    FXDCWindow dc(this, ev);
    int tw = 0, th = 0, iw = 0, ih = 0, tx, ty, ix, iy;

    // Button with nice gradient effect and rounded corners
    // Toolbar style
    if (options & BUTTON_TOOLBAR)
    {
        // Enabled and cursor inside, and up
        if (isEnabled() && underCursor() && (state == STATE_UP))
        {
            DRAW_MODERN_BUTTON_UP
        }
        // Enabled and cursor inside and down
        else if (isEnabled() && underCursor() && (state == STATE_DOWN))
        {
            DRAW_MODERN_BUTTON_DOWN
        }
        // Enabled and checked
        else if (isEnabled() && (state == STATE_ENGAGED))
        {
            DRAW_MODERN_BUTTON_UP
        }
        // Disabled or unchecked or not under cursor
        else
        {
            // Down
            if (state == STATE_DOWN)
            {
                DRAW_MODERN_BUTTON_DOWN_NO_BORDER
            }
            else
            {
                dc.setForeground(backColor);
                dc.fillRectangle(0, 0, width, height);
            }
        }
    }
    // Normal style
    else
    {
        // Draw in up state if disabled or up
        if (!isEnabled() || (state == STATE_UP))
        {
            DRAW_MODERN_BUTTON_UP
        }
        // Draw in down state if enabled and either checked or pressed
        else
        {
            DRAW_MODERN_BUTTON_DOWN
        }
    }

    // Place text & icon
    if (!label.empty())
    {
        tw = labelWidth(label);
        th = labelHeight(label);
    }
    if (icon)
    {
        iw = icon->getWidth();
        ih = icon->getHeight();
    }

    just_x(tx, ix, tw, iw);
    just_y(ty, iy, th, ih);

    // Shift a bit when pressed
    if (state && (options & (FRAME_RAISED | FRAME_SUNKEN)))
    {
        ++tx;
        ++ty;
        ++ix;
        ++iy;
    }

    // Draw enabled state
    if (isEnabled())
    {
        if (icon)
        {
            dc.drawIcon(icon, ix, iy);
        }
        if (!label.empty())
        {
            dc.setFont(font);
            dc.setForeground(textColor);
            drawLabel(dc, label, hotoff, tx, ty, tw, th);
        }
        if (hasFocus())
        {
            dc.drawFocusRectangle(border + 1, border + 1, width - 2 * border - 2, height - 2 * border - 2);
        }
    }
    // Draw grayed-out state
    else
    {
        if (icon)
        {
            dc.drawIconSunken(icon, ix, iy);
        }
        if (!label.empty())
        {
            dc.setFont(font);

            // !!! Hack here !!!
            FXColor backcolor = getApp()->getBackColor();
            FXuint rk = FXREDVAL(backcolor);
            FXuint gk = FXREDVAL(backcolor);
            FXuint bk = FXREDVAL(backcolor);

            FXuint r = FXREDVAL(borderColor);
            FXuint g = FXGREENVAL(borderColor);
            FXuint b = FXBLUEVAL(borderColor);

            FXColor color;
            if ((rk + gk + bk) / 3 < DARK_COLOR_THRESHOLD)
            {
                color = FXRGB(FXMIN(1.1 * r, 255), FXMIN(1.1 * g, 255), FXMIN(1.1 * b, 255));
            }
            else
            {
                color = borderColor;
            }

            dc.setForeground(color);
            drawLabel(dc, label, hotoff, tx, ty, tw, th);
        }
    }

    return 1;
}


//
// Hack of FXCheckButton
//

// Includes HiDPI scaling


// Handle repaint
long FXCheckButton::onPaint(FXObject*, FXSelector, void* ptr)
{
    // Initialize modern controls (here we don't use the macro)
    static FXbool init = true;
    static FXColor shadecolor, bordercolor;

    if (init)
    {
        FXuint r = FXREDVAL(baseColor);
        FXuint g = FXGREENVAL(baseColor);
        FXuint b = FXBLUEVAL(baseColor);

        shadecolor = FXRGB(0.9 * r, 0.9 * g, 0.9 * b);
        bordercolor = FXRGB(0.5 * r, 0.5 * g, 0.5 * b);

        init = false;
    }

    FXEvent* ev = (FXEvent*)ptr;
    FXint tw = 0, th = 0, tx, ty, ix, iy;

    FXDCWindow dc(this, ev);

    // Figure text size
    if (!label.empty())
    {
        tw = labelWidth(label);
        th = labelHeight(label);
    }

    // Placement
    just_x(tx, ix, tw, scaleint * 13);
    just_y(ty, iy, th, scaleint * 13);
    ix = FXMAX(ix, 0);
    iy = FXMAX(iy, 0);

    // Button with nice gradient effect and rounded corners
    // Widget background
    dc.setForeground(backColor);
    dc.fillRectangle(ev->rect.x, ev->rect.y, scaleint * ev->rect.w, scaleint * ev->rect.h);

    FXRectangle recs[4];

    // Check background
    recs[0].x = ix + 2 * scaleint;
    recs[0].y = iy + 2 * scaleint;
    recs[0].w = 9 * scaleint;
    recs[0].h = 9 * scaleint;
    if ((check == MAYBE) || !isEnabled())
    {
        dc.setForeground(baseColor);
    }
    else
    {
        dc.setForeground(boxColor);
    }
    dc.fillRectangles(recs, 1);

    // Check border
    recs[0].x = ix + 2 * scaleint;
    recs[0].y = iy + 1 * scaleint;
    recs[0].w = 9 * scaleint;
    recs[0].h = 1 * scaleint;
    recs[1].x = ix + 2 * scaleint;
    recs[1].y = iy + 11 * scaleint;
    recs[1].w = 9 * scaleint;
    recs[1].h = 1 * scaleint;
    recs[2].x = ix + 1 * scaleint;
    recs[2].y = iy + 2 * scaleint;
    recs[2].w = 1 * scaleint;
    recs[2].h = 9 * scaleint;
    recs[3].x = ix + 11 * scaleint;
    recs[3].y = iy + 2 * scaleint;
    recs[3].w = 1 * scaleint;
    recs[3].h = 9 * scaleint;
    dc.setForeground(bordercolor);
    dc.fillRectangles(recs, 4);

    // Check border corners
    recs[0].x = ix + 1 * scaleint;
    recs[0].y = iy + 1 * scaleint;
    recs[0].w = 1 * scaleint;
    recs[0].h = 1 * scaleint;
    recs[1].x = ix + 11 * scaleint;
    recs[1].y = iy + 1 * scaleint;
    recs[1].w = 1 * scaleint;
    recs[1].h = 1 * scaleint;
    recs[2].x = ix + 1 * scaleint;
    recs[2].y = iy + 11 * scaleint;
    recs[2].w = 1 * scaleint;
    recs[2].h = 1 * scaleint;
    recs[3].x = ix + 11 * scaleint;
    recs[3].y = iy + 11 * scaleint;
    recs[3].w = 1 * scaleint;
    recs[3].h = 1 * scaleint;
    dc.setForeground(shadecolor);
    dc.fillRectangles(recs, 4);

    // Check color
    if ((check == MAYBE) || !isEnabled())
    {
        dc.setForeground(shadowColor);
    }
    else
    {
        dc.setForeground(checkColor);
    }

    // Show as +
    if (options & CHECKBUTTON_PLUS)
    {
        if (check != true)
        {
            dc.fillRectangle(ix + scaleint * 6, iy + scaleint * 4, 1 * scaleint, 5 * scaleint);
        }
        dc.fillRectangle(ix + scaleint * 4, iy + scaleint * 6, 5 * scaleint, 1 * scaleint);
    }
    // Show as v
    else
    {
        if (check != false)
        {
            FXRectangle recs[7];
            recs[0].x = ix + 3 * scaleint;
            recs[0].y = iy + 5 * scaleint;
            recs[0].w = 1 * scaleint;
            recs[0].h = 3 * scaleint;
            recs[1].x = ix + 4 * scaleint;
            recs[1].y = iy + 6 * scaleint;
            recs[1].w = 1 * scaleint;
            recs[1].h = 3 * scaleint;
            recs[2].x = ix + 5 * scaleint;
            recs[2].y = iy + 7 * scaleint;
            recs[2].w = 1 * scaleint;
            recs[2].h = 3 * scaleint;
            recs[3].x = ix + 6 * scaleint;
            recs[3].y = iy + 6 * scaleint;
            recs[3].w = 1 * scaleint;
            recs[3].h = 3 * scaleint;
            recs[4].x = ix + 7 * scaleint;
            recs[4].y = iy + 5 * scaleint;
            recs[4].w = 1 * scaleint;
            recs[4].h = 3 * scaleint;
            recs[5].x = ix + 8 * scaleint;
            recs[5].y = iy + 4 * scaleint;
            recs[5].w = 1 * scaleint;
            recs[5].h = 3 * scaleint;
            recs[6].x = ix + 9 * scaleint;
            recs[6].y = iy + 3 * scaleint;
            recs[6].w = 1 * scaleint;
            recs[6].h = 3 * scaleint;
            dc.fillRectangles(recs, 7);
        }
    }

    // Text
    if (!label.empty() && (label != " "))
    {
        dc.setFont(font);
        if (isEnabled())
        {
            dc.setForeground(textColor);
            drawLabel(dc, label, hotoff, tx, ty, tw, th);
            if (hasFocus())
            {
                dc.drawFocusRectangle(tx - 1, ty - 1, tw + 2, th + 2);
            }
        }
        else
        {
            // !!! Hack here !!!
            FXColor backcolor = getApp()->getBackColor();
            FXuint rk = FXREDVAL(backcolor);
            FXuint gk = FXREDVAL(backcolor);
            FXuint bk = FXREDVAL(backcolor);

            FXuint r = FXREDVAL(borderColor);
            FXuint g = FXGREENVAL(borderColor);
            FXuint b = FXBLUEVAL(borderColor);

            FXColor color;
            if ((rk + gk + bk) / 3 < DARK_COLOR_THRESHOLD)
            {
                color = FXRGB(FXMIN(1.1 * r, 255), FXMIN(1.1 * g, 255), FXMIN(1.1 * b, 255));
            }
            else
            {
                color = borderColor;
            }

            dc.setForeground(color);
            drawLabel(dc, label, hotoff, tx, ty, tw, th);
        }
    }

    // Frame
    drawFrame(dc, 0, 0, width, height);

    return 1;
}


//
// Hack of FXTextField
//

// Handle repaint
long FXTextField::onPaint(FXObject*, FXSelector, void* ptr)
{
    // Initialize modern controls
    INIT_MODERN

    FXEvent* ev = (FXEvent*)ptr;
    FXDCWindow dc(this, ev);

    // Draw frame
    drawFrame(dc, 0, 0, width, height);

    // Draw background
    dc.setForeground(backColor);
    dc.fillRectangle(border, border, width - (border << 1), height - (border << 1));

    // !!! Hack to get a rounded rectangle shape only if _TEXTFIELD_NOFRAME is not specified !!!
    if (!(options & _TEXTFIELD_NOFRAME))
    {
        // Outside Background
        dc.setForeground(baseColor);
        dc.fillRectangle(0, 0, width, height);
        dc.drawPoints(basebackground, 4);

        // Border
        dc.setForeground(bordercolor);
        dc.drawRectangle(2, 0, width - 5, 0);
        dc.drawRectangle(2, height - 1, width - 5, height - 1);
        dc.drawRectangle(0, 2, 0, height - 5);
        dc.drawRectangle(width - 1, 2, 0, height - 5);
        dc.drawPoints(bordercorners, 4);
        dc.setForeground(shadecolor);
        dc.drawPoints(bordershade, 16);
        dc.setForeground(backColor);
        dc.fillRectangle(2, 1, width - 4, height - 2);
    }
    // !!! End of hack

    // Draw text, clipped against frame interior
    dc.setClipRectangle(border, border, width - (border << 1), height - (border << 1));
    drawTextRange(dc, 0, contents.length());

    // Draw caret
    if (flags & FLAG_CARET)
    {
        int xx = coord(cursor) - 1;
        dc.setForeground(cursorColor);
        dc.fillRectangle(xx, padtop + border, 1, height - padbottom - padtop - (border << 1));
        dc.fillRectangle(xx - 2, padtop + border, 5, 1);
        dc.fillRectangle(xx - 2, height - border - padbottom - 1, 5, 1);
    }

    return 1;
}


//
// Hack of FXToggleButton
//

// Handle repaint
long FXToggleButton::onPaint(FXObject*, FXSelector, void* ptr)
{
    // Initialize modern controls
    INIT_MODERN

    int tw = 0, th = 0, iw = 0, ih = 0, tx, ty, ix, iy;
    FXEvent* ev = (FXEvent*)ptr;
    FXDCWindow dc(this, ev);

    // Button with nice gradient effect and rounded corners
    // Button style is toolbar
    if (options & TOGGLEBUTTON_TOOLBAR)
    {
        // Enabled and cursor inside and button down
        if (down || ((options & TOGGLEBUTTON_KEEPSTATE) && state))
        {
            DRAW_MODERN_BUTTON_DOWN
        }
        // Enabled and cursor inside but button not down
        else if (isEnabled() && underCursor())
        {
            DRAW_MODERN_BUTTON_UP
        }
        // Disabled or unchecked or not under cursor
        else
        {
            dc.setForeground(backColor);
            dc.fillRectangle(0, 0, width, height);
        }
    }
    // Button style is normal
    else
    {
        // Button down
        if (down || ((options & TOGGLEBUTTON_KEEPSTATE) && state))
        {
            DRAW_MODERN_BUTTON_DOWN
        }
        // Button up
        else
        {
            DRAW_MODERN_BUTTON_UP
        }
    }

    // Place text & icon
    if (state && !altlabel.empty())
    {
        tw = labelWidth(altlabel);
        th = labelHeight(altlabel);
    }
    else if (!label.empty())
    {
        tw = labelWidth(label);
        th = labelHeight(label);
    }
    if (state && alticon)
    {
        iw = alticon->getWidth();
        ih = alticon->getHeight();
    }
    else if (icon)
    {
        iw = icon->getWidth();
        ih = icon->getHeight();
    }

    just_x(tx, ix, tw, iw);
    just_y(ty, iy, th, ih);

    // Shift a bit when pressed
    if ((down || ((options & TOGGLEBUTTON_KEEPSTATE) && state)) && (options & (FRAME_RAISED | FRAME_SUNKEN)))
    {
        ++tx;
        ++ty;
        ++ix;
        ++iy;
    }

    // Draw enabled state
    if (isEnabled())
    {
        if (state && alticon)
        {
            dc.drawIcon(alticon, ix, iy);
        }
        else if (icon)
        {
            dc.drawIcon(icon, ix, iy);
        }
        if (state && !altlabel.empty())
        {
            dc.setFont(font);
            dc.setForeground(textColor);
            drawLabel(dc, altlabel, althotoff, tx, ty, tw, th);
        }
        else if (!label.empty())
        {
            dc.setFont(font);
            dc.setForeground(textColor);
            drawLabel(dc, label, hotoff, tx, ty, tw, th);
        }
        if (hasFocus())
        {
            dc.drawFocusRectangle(border + 1, border + 1, width - 2 * border - 2, height - 2 * border - 2);
        }
    }
    // Draw grayed-out state
    else
    {
        if (state && alticon)
        {
            dc.drawIconSunken(alticon, ix, iy);
        }
        else if (icon)
        {
            dc.drawIconSunken(icon, ix, iy);
        }
        if (state && !altlabel.empty())
        {
            dc.setFont(font);
            dc.setForeground(hiliteColor);
            drawLabel(dc, altlabel, althotoff, tx + 1, ty + 1, tw, th);
            dc.setForeground(shadowColor);
            drawLabel(dc, altlabel, althotoff, tx, ty, tw, th);
        }
        else if (!label.empty())
        {
            dc.setFont(font);
            dc.setForeground(hiliteColor);
            drawLabel(dc, label, hotoff, tx + 1, ty + 1, tw, th);
            dc.setForeground(shadowColor);
            drawLabel(dc, label, hotoff, tx, ty, tw, th);
        }
    }

    return 1;
}


//
// Hack of FXScrollBar
//


// Draw scrollbar button with gradient effect and grip
static void drawGradientScrollButton(FXDCWindow& dc, FXColor topcolor, FXColor bottomcolor, FXColor shadecolor,
                                     FXColor lightcolor, FXColor gripcolor,
                                     FXuint options, int x, int y, int w, int h)
{
    // Fill rectangle with gradient in the right direction (vertical or horizontal)
    FXbool vertical = ((options & SCROLLBAR_HORIZONTAL) ? true : false);

    drawGradientRectangle(dc, topcolor, bottomcolor, x, y, w, h, vertical);

    // Draw button borders
    dc.setForeground(shadecolor);
    dc.fillRectangle(x, y, w, 1);
    dc.fillRectangle(x, y, 1, h - 1);
    dc.fillRectangle(x, y + h - 1, w, 1);
    dc.fillRectangle(x + w - 1, y, 1, h);

    // Draw grip lines for horizontal scrollbar
    if ((options & SCROLLBAR_HORIZONTAL))
    {
        dc.setForeground(gripcolor);
        dc.fillRectangle(x + w / 2 - 3, y + 4, 1, h - 7);
        dc.fillRectangle(x + w / 2, y + 4, 1, h - 7);
        dc.fillRectangle(x + w / 2 + 3, y + 4, 1, h - 7);
    }
    // Draw grip lines for vertical scrollbar
    else
    {
        dc.setForeground(gripcolor);
        dc.fillRectangle(x + 4, y + h / 2 - 3, w - 7, 1);
        dc.fillRectangle(x + 4, y + h / 2, w - 7, 1);
        dc.fillRectangle(x + 4, y + h / 2 + 3, w - 7, 1);
    }
}


// !!! Small hack to set the minimum length of the scrollbar button to barsize*2 instead of barsize/2 !!!
void FXScrollBar::setPosition(int p)
{
    int total, travel, lo, hi, l, h;

    pos = p;
    if (pos < 0)
    {
        pos = 0;
    }
    if (pos > (range - page))
    {
        pos = range - page;
    }
    lo = thumbpos;
    hi = thumbpos + thumbsize;
    if (options & SCROLLBAR_HORIZONTAL)
    {
        total = width - height - height;
        thumbsize = (total * page) / range;
        // !!! Hack to change the minimum button size !!!
        if (thumbsize < (barsize << 1))
        {
            thumbsize = (barsize << 1);
        }
        // !!! End of hack !!!
        travel = total - thumbsize;
        if (range > page)
        {
            thumbpos = height + (int)((((double)pos) * travel) / (range - page));
        }
        else
        {
            thumbpos = height;
        }
        l = thumbpos;
        h = thumbpos + thumbsize;
        if ((l != lo) || (h != hi))
        {
            update(FXMIN(l, lo), 0, FXMAX(h, hi) - FXMIN(l, lo), height);
        }
    }
    else
    {
        total = height - width - width;
        thumbsize = (total * page) / range;
        // !!! Hack to change the minimum button size !!!
        if (thumbsize < (barsize << 1))
        {
            thumbsize = (barsize << 1);
        }
        // !!! End of hack !!!
        travel = total - thumbsize;
        if (range > page)
        {
            thumbpos = width + (int)((((double)pos) * travel) / (range - page));
        }
        else
        {
            thumbpos = width;
        }
        l = thumbpos;
        h = thumbpos + thumbsize;
        if ((l != lo) || (h != hi))
        {
            update(0, FXMIN(l, lo), width, FXMAX(h, hi) - FXMIN(l, lo));
        }
    }
}


// Arrow directions
enum
{
    _ARROW_LEFT,
    _ARROW_RIGHT,
    _ARROW_UP,
    _ARROW_DOWN
};


// Draw arrow button in scrollbar with gradient effect and rounded corners
static void drawGradientArrowButton(FXDCWindow& dc, FXColor backcolor, FXColor topcolor, FXColor bottomcolor,
                                    FXColor shadecolor,
                                    FXColor bordercolor, FXColor arrowcolor,
                                    FXuint options, int x, int y, int w, int h, FXbool down, FXuint direction)
{
    FXPoint arrowpoints[3];
    int xx, yy, ah, ab;

    FXPoint basebackground[2];
    FXPoint bordershade[8];
    FXPoint bordercorners[2];

    // Rounded corner and arrow point coordinates depend on the button direction
    if (direction == _ARROW_UP)
    {
        // Rounded corners
        basebackground[0] = FXPoint(0, 0);
        basebackground[1] = FXPoint(w - 1, 0);
        bordercorners[0] = FXPoint(1, 1);
        bordercorners[1] = FXPoint(w - 2, 1);
        bordershade[0] = FXPoint(0, 1);
        bordershade[1] = FXPoint(1, 0);
        bordershade[2] = FXPoint(1, 2);
        bordershade[3] = FXPoint(2, 1);
        bordershade[4] = FXPoint(w - 2, 0);
        bordershade[5] = FXPoint(w - 1, 1);
        bordershade[6] = FXPoint(w - 3, 1);
        bordershade[7] = FXPoint(w - 2, 2);

        // Arrow points
        ab = (w - 7) | 1;
        ah = ab >> 1;
        xx = x + ((w - ab) >> 1);
        yy = y + ((h - ah) >> 1);
        if (down)
        {
            ++xx;
            ++yy;
        }
        arrowpoints[0] = FXPoint(xx + (ab >> 1), yy - 1);
        arrowpoints[1] = FXPoint(xx, yy + ah);
        arrowpoints[2] = FXPoint(xx + ab, yy + ah);
    }
    else if (direction == _ARROW_DOWN)
    {
        // Rounded corners
        basebackground[0] = FXPoint(x, y + h - 1);
        basebackground[1] = FXPoint(x + w - 1, y + h - 1);
        bordercorners[0] = FXPoint(x + 1, y + h - 2);
        bordercorners[1] = FXPoint(x + w - 2, y + h - 2);
        bordershade[0] = FXPoint(x, y + h - 2);
        bordershade[1] = FXPoint(x + 1, y + h - 1);
        bordershade[2] = FXPoint(x + 1, y + h - 3);
        bordershade[3] = FXPoint(x + 2, y + h - 2);
        bordershade[4] = FXPoint(x + w - 1, y + h - 2);
        bordershade[5] = FXPoint(x + w - 2, y + h - 1);
        bordershade[6] = FXPoint(x + w - 2, y + h - 3);
        bordershade[7] = FXPoint(x + w - 3, y + h - 2);

        // Arrow points
        ab = (w - 7) | 1;
        ah = ab >> 1;
        xx = x + ((w - ab) >> 1);
        yy = y + ((h - ah) >> 1);
        if (down)
        {
            ++xx;
            ++yy;
        }
        arrowpoints[0] = FXPoint(xx + 1, yy);
        arrowpoints[1] = FXPoint(xx + ab - 1, yy);
        arrowpoints[2] = FXPoint(xx + (ab >> 1), yy + ah);
    }
    else if (direction == _ARROW_LEFT)
    {
        // Rounded corners
        basebackground[0] = FXPoint(0, 0);
        basebackground[1] = FXPoint(0, h - 1);
        bordercorners[0] = FXPoint(1, 1);
        bordercorners[1] = FXPoint(1, h - 2);
        bordershade[0] = FXPoint(0, 1);
        bordershade[1] = FXPoint(1, 0);
        bordershade[2] = FXPoint(1, 2);
        bordershade[3] = FXPoint(2, 1);
        bordershade[4] = FXPoint(0, h - 2);
        bordershade[5] = FXPoint(1, h - 1);
        bordershade[6] = FXPoint(1, h - 3);
        bordershade[7] = FXPoint(2, h - 2);

        // Arrow points
        ab = (h - 7) | 1;
        ah = ab >> 1;
        xx = x + ((w - ah) >> 1);
        yy = y + ((h - ab) >> 1);
        if (down)
        {
            ++xx;
            ++yy;
        }
        arrowpoints[0] = FXPoint(xx + ah, yy);
        arrowpoints[1] = FXPoint(xx + ah, yy + ab - 1);
        arrowpoints[2] = FXPoint(xx, yy + (ab >> 1));
    }
    else // _ARROW_RIGHT
    {
        // Rounded corners
        basebackground[0] = FXPoint(x + w - 1, y);
        basebackground[1] = FXPoint(x + w - 1, y + h - 1);
        bordercorners[0] = FXPoint(x + w - 2, y + 1);
        bordercorners[1] = FXPoint(x + w - 2, y + h - 2);
        bordershade[0] = FXPoint(x + w - 2, y);
        bordershade[1] = FXPoint(x + w - 1, y + 1);
        bordershade[2] = FXPoint(x + w - 3, y + 1);
        bordershade[3] = FXPoint(x + w - 2, y + 2);
        bordershade[4] = FXPoint(x + w - 1, y + h - 2);
        bordershade[5] = FXPoint(x + w - 2, y + h - 1);
        bordershade[6] = FXPoint(x + w - 2, y + h - 3);
        bordershade[7] = FXPoint(x + w - 3, y + h - 2);

        // Arrow points
        ab = (h - 7) | 1;
        ah = ab >> 1;
        xx = x + ((w - ah) >> 1);
        yy = y + ((h - ab) >> 1);
        if (down)
        {
            ++xx;
            ++yy;
        }
        arrowpoints[0] = FXPoint(xx, yy);
        arrowpoints[1] = FXPoint(xx, yy + ab - 1);
        arrowpoints[2] = FXPoint(xx + ah, yy + (ab >> 1));
    }

    // Draw button when up
    if (!down)
    {
        // Fill rectangle with gradient in the right direction (vertical or horizontal)
        FXbool vertical = ((options & SCROLLBAR_HORIZONTAL) ? true : false);
        drawGradientRectangle(dc, topcolor, bottomcolor, x, y, w, h, vertical);

        // Button borders
        dc.setForeground(shadecolor);
        dc.fillRectangle(x, y, w, 1);
        dc.fillRectangle(x, y, 1, h - 1);
        dc.fillRectangle(x, y + h - 1, w, 1);
        dc.fillRectangle(x + w - 1, y, 1, h);

        // Rounded corners
        dc.setForeground(backcolor);
        dc.drawPoints(basebackground, 2);
        dc.setForeground(shadecolor);
        dc.drawPoints(bordercorners, 2);
        dc.setForeground(bordercolor);
        dc.drawPoints(bordershade, 8);

        // Arrow
        dc.setForeground(arrowcolor);
        dc.fillPolygon(arrowpoints, 3);
    }
    // Draw button when down (pressed)
    else
    {
        // Dark background
        dc.setForeground(bordercolor);
        dc.fillRectangle(x, y, w, h);

        // Button borders
        dc.setForeground(shadecolor);
        dc.fillRectangle(x, y, w, 1);
        dc.fillRectangle(x, y, 1, h - 1);
        dc.fillRectangle(x, y + h - 1, w, 1);
        dc.fillRectangle(x + w - 1, y, 1, h);

        // Rounded corners
        dc.setForeground(backcolor);
        dc.drawPoints(basebackground, 2);
        dc.setForeground(shadecolor);
        dc.drawPoints(bordercorners, 2);
        dc.setForeground(bordercolor);
        dc.drawPoints(bordershade, 8);

        // Arrow
        dc.setForeground(arrowcolor);
        dc.fillPolygon(arrowpoints, 3);
    }
}


// Support dark theme

// Handle repaint
long FXScrollBar::onPaint(FXObject*, FXSelector, void* ptr)
{
    // Caution, don't use the macro here

    static FXbool init = true;
    static FXColor bg_topcolor, bg_bottomcolor, bg_shadecolor, bg_bordercolor;
    static FXColor sb_topcolor, sb_bottomcolor, sb_shadecolor, sb_bordercolor;
    static FXColor sb_lightcolor, scrollbarcolor, gripcolor;

    FXEvent* ev = (FXEvent*)ptr;
    int total;
    FXDCWindow dc(this, ev);

    // At first run, select the scrollbar style and color
    if (init)
    {
        scrollbarcolor = getApp()->reg().readColorEntry("SETTINGS", "scrollbarcolor", FXRGB(237, 233, 227));

        // Compute gradient colors from the base color
        // Decompose the base color
        FXuint r = FXREDVAL(backColor);
        FXuint g = FXGREENVAL(backColor);
        FXuint b = FXBLUEVAL(backColor);

        // Compute the gradient colors from the base color (background)
        bg_topcolor = FXRGB(FXMIN(1.1 * r, 255), FXMIN(1.1 * g, 255), FXMIN(1.1 * b, 255));
        bg_bottomcolor = FXRGB(0.9 * r, 0.9 * g, 0.9 * b);
        bg_shadecolor = FXRGB(0.8 * r, 0.8 * g, 0.8 * b);
        bg_bordercolor = FXRGB(0.9 * r, 0.9 * g, 0.9 * b);

        // Compute the gradient colors from the base color (scrollbar)
        r = FXREDVAL(scrollbarcolor);
        g = FXGREENVAL(scrollbarcolor);
        b = FXBLUEVAL(scrollbarcolor);
        sb_topcolor = FXRGB(FXMIN(1.1 * r, 255), FXMIN(1.1 * g, 255), FXMIN(1.1 * b, 255));
        sb_bottomcolor = FXRGB(0.9 * r, 0.9 * g, 0.9 * b);
        sb_shadecolor = FXRGB(0.8 * r, 0.8 * g, 0.8 * b);
        sb_bordercolor = FXRGB(0.9 * r, 0.9 * g, 0.9 * b);
        (void)sb_bordercolor; // Hack to avoid unused variable compiler warning
        sb_lightcolor = FXRGB(FXMIN(1.3 * r, 255), FXMIN(1.3 * g, 255), FXMIN(1.3 * b, 255));

        // Compute scrollbar grip color, supporting dark theme
        FXuint rs = FXREDVAL(scrollbarcolor);
        FXuint gs = FXREDVAL(scrollbarcolor);
        FXuint bs = FXREDVAL(scrollbarcolor);

        r = FXREDVAL(borderColor);
        g = FXGREENVAL(borderColor);
        b = FXBLUEVAL(borderColor);

        if ((rs + gs + bs) / 3 < DARK_COLOR_THRESHOLD)
        {
            gripcolor = FXRGB(FXMIN(1.5 * r, 255), FXMIN(1.5 * g, 255), FXMIN(1.5 * b, 255));
        }
        else
        {
            gripcolor = FXRGB(0.5 * r, 0.5 * g, 0.5 * b);
        }

        init = false;
    }

    // Nice scrollbar with gradient and rounded corners
    if (options & SCROLLBAR_HORIZONTAL)
    {
        total = width - height - height;
        if (thumbsize < total)                            // Scrollable
        {
            drawGradientScrollButton(dc, sb_topcolor, sb_bottomcolor, sb_shadecolor, sb_lightcolor, gripcolor, options,
                                     thumbpos, 0, thumbsize, height);
            dc.setForeground(bg_bordercolor);
            dc.setBackground(backColor);
            dc.fillRectangle(height, 0, thumbpos - height, height);
            dc.fillRectangle(thumbpos + thumbsize, 0, width - height - thumbpos - thumbsize, height);
        }
        else                                            // Non-scrollable
        {
            dc.setForeground(bg_bordercolor);
            dc.setBackground(backColor);
            dc.fillRectangle(height, 0, total, height);
        }
        drawGradientArrowButton(dc, backColor, bg_topcolor, bg_bottomcolor, bg_shadecolor, bg_bordercolor, arrowColor,
                                options, width - height, 0, height, height, (mode == MODE_INC), _ARROW_RIGHT);
        drawGradientArrowButton(dc, backColor, bg_topcolor, bg_bottomcolor, bg_shadecolor, bg_bordercolor, arrowColor,
                                options, 0, 0, height, height, (mode == MODE_DEC), _ARROW_LEFT);
    }
    // Vertical
    else
    {
        total = height - width - width;
        if (thumbsize < total)                            // Scrollable
        {
            drawGradientScrollButton(dc, sb_topcolor, sb_bottomcolor, sb_shadecolor, sb_lightcolor, gripcolor, options,
                                     0, thumbpos, width, thumbsize);
            dc.setForeground(bg_bordercolor);
            dc.setBackground(backColor);
            dc.fillRectangle(0, width, width, thumbpos - width);
            dc.fillRectangle(0, thumbpos + thumbsize, width, height - width - thumbpos - thumbsize);
        }
        else                                            // Non-scrollable
        {
            dc.setForeground(bg_bordercolor);
            dc.setBackground(backColor);
            dc.fillRectangle(0, width, width, total);
        }
        drawGradientArrowButton(dc, backColor, bg_topcolor, bg_bottomcolor, bg_shadecolor, bg_bordercolor, arrowColor,
                                options, 0, height - width, width, width, (mode == MODE_INC), _ARROW_DOWN);
        drawGradientArrowButton(dc, backColor, bg_topcolor, bg_bottomcolor, bg_shadecolor, bg_bordercolor, arrowColor,
                                options, 0, 0, width, width, (mode == MODE_DEC), _ARROW_UP);
    }

    return 1;
}


#define MENUBUTTONARROW_WIDTH     11
#define MENUBUTTONARROW_HEIGHT    5


//
// Hack of FXMenuTitle
//

// This hack adds a gradient with rounded corner theme to the menu title

// Handle repaint
long FXMenuTitle::onPaint(FXObject*, FXSelector, void* ptr)
{
    // Initialize modern controls
    FXColor baseColor = backColor;
    INIT_MODERN

    FXEvent* ev = (FXEvent*)ptr;
    FXDCWindow dc(this, ev);
    FXint xx, yy;

    dc.setFont(font);
    xx = 6;
    yy = 0;

    if (isEnabled())
    {
        if (isActive())
        {
            // Button with nice gradient effect and rounded corners
            dc.setForeground(selbackColor);
            dc.fillRectangle(0, 0, width, height);
            dc.setForeground(backColor);
            dc.drawPoints(basebackground, 4);
            dc.setForeground(bordercolor);
            dc.drawRectangle(2, 0, width - 5, 0);
            dc.drawRectangle(2, height - 1, width - 5, height - 1);
            dc.drawRectangle(0, 2, 0, height - 5);
            dc.drawRectangle(width - 1, 2, 0, height - 5);
            dc.drawPoints(bordercorners, 4);
            dc.setForeground(selbackColor);
            dc.drawPoints(bordershade, 16);

            xx++;
            yy++;
        }
        else if (underCursor())
        {
            // Button with nice gradient effect and rounded corners
            DRAW_MODERN_BUTTON_UP
        }
        else
        {
            dc.setForeground(backColor);
            dc.fillRectangle(0, 0, width, height);
        }

        if (icon)
        {
            dc.drawIcon(icon, xx, yy + (height - icon->getHeight()) / 2);
            xx += 5 + icon->getWidth();
        }

        if (!label.empty())
        {
            yy += font->getFontAscent() + (height - font->getFontHeight()) / 2;
            dc.setForeground(isActive() ? seltextColor : textColor);
            dc.drawText(xx, yy, label);
            if (0 <= hotoff)
            {
                dc.fillRectangle(xx + font->getTextWidth(&label[0], hotoff), yy + 1,
                                 font->getTextWidth(&label[hotoff], wclen(&label[hotoff])), 1);
            }
        }
    }
    else
    {
        dc.setForeground(backColor);
        dc.fillRectangle(0, 0, width, height);
        if (icon)
        {
            dc.drawIconSunken(icon, xx, yy + (height - icon->getHeight()) / 2);
            xx += 5 + icon->getWidth();
        }
        if (!label.empty())
        {
            yy += font->getFontAscent() + (height - font->getFontHeight()) / 2;
            dc.setForeground(hiliteColor);
            dc.drawText(xx + 1, yy + 1, label);
            if (0 <= hotoff)
            {
                dc.fillRectangle(xx + font->getTextWidth(&label[0], hotoff), yy + 1,
                                 font->getTextWidth(&label[hotoff], wclen(&label[hotoff])), 1);
            }
            dc.setForeground(shadowColor);
            dc.drawText(xx, yy, label);
            if (0 <= hotoff)
            {
                dc.fillRectangle(xx + font->getTextWidth(&label[0], hotoff), yy + 1,
                                 font->getTextWidth(&label[hotoff], wclen(&label[hotoff])), 1);
            }
        }
    }

    return 1;
}


//
// Hack of FXRadioButton
//

// For HiDPI scaling, remove highlight part of grayed text and support dark theme


// Handle repaint
long FXRadioButton::onPaint(FXObject*, FXSelector, void* ptr)
{
    FXEvent* ev = (FXEvent*)ptr;
    FXint tw = 0, th = 0, tx, ty, ix, iy;
    FXRectangle recs[6];
    FXDCWindow dc(this, ev);

    dc.setForeground(backColor);
    dc.fillRectangle(ev->rect.x, ev->rect.y, ev->rect.w, ev->rect.h);

    if (!label.empty())
    {
        tw = labelWidth(label);
        th = labelHeight(label);
    }

    // Placement
    just_x(tx, ix, tw, scaleint * 13);
    just_y(ty, iy, th, scaleint * 13);
    ix = FXMAX(ix, 0);
    iy = FXMAX(iy, 0);

    // Inside
    recs[0].x = ix + 4 * scaleint;
    recs[0].y = iy + 2 * scaleint;
    recs[0].w = 4 * scaleint;
    recs[0].h = 1 * scaleint;
    recs[1].x = ix + 3 * scaleint;
    recs[1].y = iy + 3 * scaleint;
    recs[1].w = 6 * scaleint;
    recs[1].h = 1 * scaleint;
    recs[2].x = ix + 2 * scaleint;
    recs[2].y = iy + 4 * scaleint;
    recs[2].w = 8 * scaleint;
    recs[2].h = 4 * scaleint;
    recs[3].x = ix + 3 * scaleint;
    recs[3].y = iy + 8 * scaleint;
    recs[3].w = 6 * scaleint;
    recs[3].h = 1 * scaleint;
    recs[4].x = ix + 4 * scaleint;
    recs[4].y = iy + 9 * scaleint;
    recs[4].w = 4 * scaleint;
    recs[4].h = 1 * scaleint;

    if (!isEnabled()) // fix by Daniel Gehriger (gehriger@linkcad.com)
    {
        dc.setForeground(baseColor);
    }
    else
    {
        dc.setForeground(diskColor);
    }
    dc.fillRectangles(recs, 5);

    // Radio button with modern appearance
    // Top left inside
    recs[0].x = ix + 4 * scaleint;
    recs[0].y = iy + 1 * scaleint;
    recs[0].w = 4 * scaleint;
    recs[0].h = 1 * scaleint;
    recs[1].x = ix + 2 * scaleint;
    recs[1].y = iy + 2 * scaleint;
    recs[1].w = 2 * scaleint;
    recs[1].h = 1 * scaleint;
    recs[2].x = ix + 8 * scaleint;
    recs[2].y = iy + 2 * scaleint;
    recs[2].w = 2 * scaleint;
    recs[2].h = 1 * scaleint;
    recs[3].x = ix + 2 * scaleint;
    recs[3].y = iy + 3 * scaleint;
    recs[3].w = 1 * scaleint;
    recs[3].h = 1 * scaleint;
    recs[4].x = ix + 1 * scaleint;
    recs[4].y = iy + 4 * scaleint;
    recs[4].w = 1 * scaleint;
    recs[4].h = 4 * scaleint;
    recs[5].x = ix + 2 * scaleint;
    recs[5].y = iy + 8 * scaleint;
    recs[5].w = 1 * scaleint;
    recs[5].h = 2 * scaleint;
    dc.setForeground(borderColor);
    dc.fillRectangles(recs, 6);

    // Bottom right inside
    recs[0].x = ix + 9 * scaleint;
    recs[0].y = iy + 3 * scaleint;
    recs[0].w = 1 * scaleint;
    recs[0].h = 1 * scaleint;
    recs[1].x = ix + 10 * scaleint;
    recs[1].y = iy + 4 * scaleint;
    recs[1].w = 1 * scaleint;
    recs[1].h = 4 * scaleint;
    recs[2].x = ix + 9 * scaleint;
    recs[2].y = iy + 8 * scaleint;
    recs[2].w = 1 * scaleint;
    recs[2].h = 1 * scaleint;
    recs[3].x = ix + 8 * scaleint;
    recs[3].y = iy + 9 * scaleint;
    recs[3].w = 2 * scaleint;
    recs[3].h = 1 * scaleint;
    recs[4].x = ix + 3 * scaleint;
    recs[4].y = iy + 9 * scaleint;
    recs[4].w = 1 * scaleint;
    recs[4].h = 1 * scaleint;
    recs[5].x = ix + 4 * scaleint;
    recs[5].y = iy + 10 * scaleint;
    recs[5].w = 4 * scaleint;
    recs[5].h = 1 * scaleint;
    dc.setForeground(borderColor);
    dc.fillRectangles(recs, 6);

    // Ball inside
    if (check != false)
    {
        recs[0].x = ix + 5 * scaleint;
        recs[0].y = iy + 4 * scaleint;
        recs[0].w = 2 * scaleint;
        recs[0].h = 1 * scaleint;
        recs[1].x = ix + 4 * scaleint;
        recs[1].y = iy + 5 * scaleint;
        recs[1].w = 4 * scaleint;
        recs[1].h = 2 * scaleint;
        recs[2].x = ix + 5 * scaleint;
        recs[2].y = iy + 7 * scaleint;
        recs[2].w = 2 * scaleint;
        recs[2].h = 1 * scaleint;
        if (isEnabled())
        {
            dc.setForeground(radioColor);
        }
        else
        {
            dc.setForeground(shadowColor);
        }
        dc.fillRectangles(recs, 3);
    }

    // Label
    if (!label.empty())
    {
        dc.setFont(font);
        if (isEnabled())
        {
            dc.setForeground(textColor);
            drawLabel(dc, label, hotoff, tx, ty, tw, th);
            if (hasFocus())
            {
                dc.drawFocusRectangle(tx - 1, ty - 1, tw + 2 * scaleint, th + 2);
            }
        }
        else
        {
            // !!! Hack here !!!
            FXColor backcolor = getApp()->getBackColor();
            FXuint rk = FXREDVAL(backcolor);
            FXuint gk = FXREDVAL(backcolor);
            FXuint bk = FXREDVAL(backcolor);

            FXuint r = FXREDVAL(borderColor);
            FXuint g = FXGREENVAL(borderColor);
            FXuint b = FXBLUEVAL(borderColor);

            FXColor color;
            if ((rk + gk + bk) / 3 < DARK_COLOR_THRESHOLD)
            {
                color = FXRGB(FXMIN(1.1 * r, 255), FXMIN(1.1 * g, 255), FXMIN(1.1 * b, 255));
            }
            else
            {
                color = borderColor;
            }

            dc.setForeground(color);
            drawLabel(dc, label, hotoff, tx, ty, tw, th);
        }
    }
    drawFrame(dc, 0, 0, width, height);
    return 1;
}


//
// Hack of FXMenuButton
//


// Handle repaint
long FXMenuButton::onPaint(FXObject*, FXSelector, void* ptr)
{
    // Initialize modern controls
    INIT_MODERN

    int tw = 0, th = 0, iw = 0, ih = 0, tx, ty, ix, iy;
    FXEvent* ev = (FXEvent*)ptr;
    FXPoint points[3];
    FXDCWindow dc(this, ev);

    // Button with nice gradient effect and rounded corners
    // Toolbar style
    if (options & MENUBUTTON_TOOLBAR)
    {
        // Enabled and cursor inside, and not popped up
        if (isEnabled() && underCursor() && !state)
        {
            DRAW_MODERN_BUTTON_DOWN
        }
        // Enabled and popped up
        else if (isEnabled() && state)
        {
            DRAW_MODERN_BUTTON_UP
        }
        // Disabled or unchecked or not under cursor
        else
        {
            dc.setForeground(backColor);
            dc.fillRectangle(0, 0, width, height);
        }
    }
    // Normal style
    else
    {
        // Draw in up state if disabled or up
        if (!isEnabled() || !state)
        {
            DRAW_MODERN_BUTTON_UP
        }
        // If enabled and either checked or pressed
        else
        {
            DRAW_MODERN_BUTTON_DOWN
        }
    }

    // Position text & icon
    if (!label.empty())
    {
        tw = labelWidth(label);
        th = labelHeight(label);
    }

    // Icon?
    if (icon)
    {
        iw = icon->getWidth();
        ih = icon->getHeight();
    }
    // Arrows?
    else if (!(options & MENUBUTTON_NOARROWS))
    {
        if (options & MENUBUTTON_LEFT)
        {
            ih = MENUBUTTONARROW_WIDTH;
            iw = MENUBUTTONARROW_HEIGHT;
        }
        else
        {
            iw = MENUBUTTONARROW_WIDTH;
            ih = MENUBUTTONARROW_HEIGHT;
        }
    }

    // Keep some room for the arrow!
    just_x(tx, ix, tw, iw);
    just_y(ty, iy, th, ih);

    // Move a bit when pressed
    if (state)
    {
        ++tx;
        ++ty;
        ++ix;
        ++iy;
    }

    // Draw icon
    if (icon)
    {
        if (isEnabled())
        {
            dc.drawIcon(icon, ix, iy);
        }
        else
        {
            dc.drawIconSunken(icon, ix, iy);
        }
    }
    // Draw arrows
    else if (!(options & MENUBUTTON_NOARROWS))
    {
        // Right arrow
        if ((options & MENUBUTTON_RIGHT) == MENUBUTTON_RIGHT)
        {
            if (isEnabled())
            {
                dc.setForeground(textColor);
            }
            else
            {
                dc.setForeground(shadowColor);
            }
            points[0].x = ix;
            points[0].y = iy;
            points[1].x = ix;
            points[1].y = iy + MENUBUTTONARROW_WIDTH - 1;
            points[2].x = ix + MENUBUTTONARROW_HEIGHT;
            points[2].y = (short)(iy + (MENUBUTTONARROW_WIDTH >> 1));
            dc.fillPolygon(points, 3);
        }
        // Left arrow
        else if (options & MENUBUTTON_LEFT)
        {
            if (isEnabled())
            {
                dc.setForeground(textColor);
            }
            else
            {
                dc.setForeground(shadowColor);
            }
            points[0].x = ix + MENUBUTTONARROW_HEIGHT;
            points[0].y = iy;
            points[1].x = ix + MENUBUTTONARROW_HEIGHT;
            points[1].y = iy + MENUBUTTONARROW_WIDTH - 1;
            points[2].x = ix;
            points[2].y = (short)(iy + (MENUBUTTONARROW_WIDTH >> 1));
            dc.fillPolygon(points, 3);
        }
        // Up arrow
        else if (options & MENUBUTTON_UP)
        {
            if (isEnabled())
            {
                dc.setForeground(textColor);
            }
            else
            {
                dc.setForeground(shadowColor);
            }
            points[0].x = (short)(ix + (MENUBUTTONARROW_WIDTH >> 1));
            points[0].y = iy - 1;
            points[1].x = ix;
            points[1].y = iy + MENUBUTTONARROW_HEIGHT;
            points[2].x = ix + MENUBUTTONARROW_WIDTH;
            points[2].y = iy + MENUBUTTONARROW_HEIGHT;
            dc.fillPolygon(points, 3);
        }
        // Down arrow
        else
        {
            if (isEnabled())
            {
                dc.setForeground(textColor);
            }
            else
            {
                dc.setForeground(shadowColor);
            }
            points[0].x = ix + 1;
            points[0].y = iy;
            points[2].x = ix + MENUBUTTONARROW_WIDTH - 1;
            points[2].y = iy;
            points[1].x = (short)(ix + (MENUBUTTONARROW_WIDTH >> 1));
            points[1].y = iy + MENUBUTTONARROW_HEIGHT;
            dc.fillPolygon(points, 3);
        }
    }

    // Draw text
    if (!label.empty())
    {
        dc.setFont(font);
        if (isEnabled())
        {
            dc.setForeground(textColor);
            drawLabel(dc, label, hotoff, tx, ty, tw, th);
        }
        else
        {
            dc.setForeground(hiliteColor);
            drawLabel(dc, label, hotoff, tx + 1, ty + 1, tw, th);
            dc.setForeground(shadowColor);
            drawLabel(dc, label, hotoff, tx, ty, tw, th);
        }
    }

    // Draw focus
    if (hasFocus())
    {
        if (isEnabled())
        {
            dc.drawFocusRectangle(border + 1, border + 1, width - 2 * border - 2, height - 2 * border - 2);
        }
    }
    return 1;
}


//
// Hack of FXArrowButton
//


// Handle repaint
long FXArrowButton::onPaint(FXObject*, FXSelector, void* ptr)
{
    // Initialize modern controls
    INIT_MODERN

    FXEvent* ev = (FXEvent*)ptr;
    FXDCWindow dc(this, ev);
    FXPoint points[3];
    int xx, yy, ww, hh, q;

    // Button with nice gradient effect and rounded corners
    // Toolbar style
    if (options & ARROW_TOOLBAR)
    {
        // Enabled and cursor inside, and up
        if (isEnabled() && underCursor() && !state)
        {
            DRAW_MODERN_BUTTON_UP
        }
        // Enabled and cursor inside and down
        else if (isEnabled() && state)
        {
            DRAW_MODERN_BUTTON_DOWN
        }
        // Disabled or unchecked or not under cursor
        else
        {
            dc.setForeground(backColor);
            dc.fillRectangle(0, 0, width, height);
        }
    }
    // Normal style
    else
    {
        // Draw sunken if enabled and pressed
        if (isEnabled() && state)
        {
            DRAW_MODERN_BUTTON_DOWN
        }
        // Draw in up state if disabled or up
        else
        {
            DRAW_MODERN_BUTTON_UP
        }
    }

    // Compute size of the arrows
    ww = width - padleft - padright - (border << 1);
    hh = height - padtop - padbottom - (border << 1);
    if (options & (ARROW_UP | ARROW_DOWN))
    {
        q = ww | 1;
        if (q > (hh << 1))
        {
            q = (hh << 1) - 1;
        }
        ww = q;
        hh = q >> 1;
    }
    else
    {
        q = hh | 1;
        if (q > (ww << 1))
        {
            q = (ww << 1) - 1;
        }
        ww = q >> 1;
        hh = q;
    }

    if (options & JUSTIFY_LEFT)
    {
        xx = padleft + border;
    }
    else if (options & JUSTIFY_RIGHT)
    {
        xx = width - ww - padright - border;
    }
    else
    {
        xx = (width - ww) / 2;
    }

    if (options & JUSTIFY_TOP)
    {
        yy = padtop + border;
    }
    else if (options & JUSTIFY_BOTTOM)
    {
        yy = height - hh - padbottom - border;
    }
    else
    {
        yy = (height - hh) / 2;
    }

    if (state)
    {
        ++xx;
        ++yy;
    }

    if (isEnabled())
    {
        dc.setForeground(arrowColor);
    }
    else
    {
        dc.setForeground(shadowColor);
    }

    // NB Size of arrow should stretch
    if (options & ARROW_UP)
    {
        points[0].x = xx + (ww >> 1);
        points[0].y = yy - 1;
        points[1].x = xx;
        points[1].y = yy + hh;
        points[2].x = xx + ww;
        points[2].y = yy + hh;
        dc.fillPolygon(points, 3);
    }
    else if (options & ARROW_DOWN)
    {
        points[0].x = xx + 1;
        points[0].y = yy;
        points[1].x = xx + ww - 1;
        points[1].y = yy;
        points[2].x = xx + (ww >> 1);
        points[2].y = yy + hh;
        dc.fillPolygon(points, 3);
    }
    else if (options & ARROW_LEFT)
    {
        points[0].x = xx + ww;
        points[0].y = yy;
        points[1].x = xx + ww;
        points[1].y = yy + hh - 1;
        points[2].x = xx;
        points[2].y = yy + (hh >> 1);
        dc.fillPolygon(points, 3);
    }
    else if (options & ARROW_RIGHT)
    {
        points[0].x = xx;
        points[0].y = yy;
        points[1].x = xx;
        points[1].y = yy + hh - 1;
        points[2].x = xx + ww;
        points[2].y = yy + (hh >> 1);
        dc.fillPolygon(points, 3);
    }
    return 1;
}


//
// Hack of FXProgressBar
//

// Note : Not implemented for the dial and vertical progress bar!
//        This hacks assumes that border = 2


// Draw only the interior, i.e. the part that changes
void FXProgressBar::drawInterior(FXDCWindow& dc)
{
    static FXbool init = true;
    static FXColor topcolor, bottomcolor, bordercolor;

    FXPoint bordercorners[4] = { FXPoint(1, 1), FXPoint(1, height - 2), FXPoint(width - 2, 1),
                                 FXPoint(width - 2, height - 2) };

    // Init modern controls (here we don't use the macro)
    if (init)
    {
        FXuint r = FXREDVAL(barColor);
        FXuint g = FXGREENVAL(barColor);
        FXuint b = FXBLUEVAL(barColor);

        topcolor = FXRGB(FXMIN(1.2 * r, 255), FXMIN(1.2 * g, 255), FXMIN(1.2 * b, 255));
        bottomcolor = FXRGB(0.9 * r, 0.9 * g, 0.9 * b);

        r = FXREDVAL(baseColor);
        g = FXGREENVAL(baseColor);
        b = FXBLUEVAL(baseColor);

        bordercolor = FXRGB(0.5 * r, 0.5 * g, 0.5 * b);

        init = false;
    }

    int percent, barlength, barfilled, tx, ty, tw, th, n, d;
    char numtext[6];

    if (options & PROGRESSBAR_DIAL)
    {
        // If total is 0, it's 100%
        barfilled = 23040;
        percent = 100;
        if (total != 0)
        {
            barfilled = (FXuint)(((double)progress * (double)23040) / (double)total);
            percent = (FXuint)(((double)progress * 100.0) / (double)total);
        }

        tw = width - (border << 1) - padleft - padright;
        th = height - (border << 1) - padtop - padbottom;
        d = FXMIN(tw, th) - 1;

        tx = border + padleft + ((tw - d) / 2);
        ty = border + padtop + ((th - d) / 2);

        if (barfilled != 23040)
        {
            dc.setForeground(barBGColor);
            dc.fillArc(tx, ty, d, d, 5760, 23040 - barfilled);
        }
        if (barfilled != 0)
        {
            dc.setForeground(barColor);
            dc.fillArc(tx, ty, d, d, 5760, -barfilled);
        }

        // Draw outside circle
        dc.setForeground(borderColor);
        dc.drawArc(tx + 1, ty, d, d, 90 * 64, 45 * 64);
        dc.drawArc(tx, ty + 1, d, d, 135 * 64, 45 * 64);
        dc.setForeground(baseColor);
        dc.drawArc(tx - 1, ty, d, d, 270 * 64, 45 * 64);
        dc.drawArc(tx, ty - 1, d, d, 315 * 64, 45 * 64);

        dc.setForeground(shadowColor);
        dc.drawArc(tx, ty, d, d, 45 * 64, 180 * 64);
        dc.setForeground(hiliteColor);
        dc.drawArc(tx, ty, d, d, 225 * 64, 180 * 64);

        // Draw text
        if (options & PROGRESSBAR_PERCENTAGE)
        {
            dc.setFont(font);
            tw = font->getTextWidth("100%", 4);
            if (tw > (10 * d) / 16)
            {
                return;
            }
            th = font->getFontHeight();
            if (th > d / 2)
            {
                return;
            }
            snprintf(numtext, sizeof(numtext), "%d%%", percent);
            n = strlen(numtext);
            tw = font->getTextWidth(numtext, n);
            //th = font->getFontHeight();
            tx = tx + d / 2 - tw / 2;
            ty = ty + d / 2 + font->getFontAscent() + 5;
            //dc.setForeground(textNumColor);
#ifdef HAVE_XFT_H
            dc.setForeground(barBGColor); // Code for XFT until XFT can use BLT_SRC_XOR_DST
            dc.drawText(tx - 1, ty, numtext, n);
            dc.drawText(tx + 1, ty, numtext, n);
            dc.drawText(tx, ty - 1, numtext, n);
            dc.drawText(tx, ty + 1, numtext, n);
            dc.setForeground(textNumColor);
            dc.drawText(tx, ty, numtext, n);
#else
            dc.setForeground(FXRGB(255, 255, 255)); // Original code
            dc.setFunction(BLT_SRC_XOR_DST);
            dc.drawText(tx, ty, numtext, n);
#endif
        }
    }
    // Vertical bar
    else if (options & PROGRESSBAR_VERTICAL)
    {
        // If total is 0, it's 100%
        barlength = height - border - border;
        barfilled = barlength;
        percent = 100;

        if (total != 0)
        {
            barfilled = (FXuint)(((double)progress * (double)barlength) / (double)total);
            percent = (FXuint)(((double)progress * 100.0) / (double)total);
        }

        // Draw completed bar
        if (0 < barfilled)
        {
            dc.setForeground(barColor);
            dc.fillRectangle(border, height - border - barfilled, width - (border << 1), barfilled);
        }

        // Draw uncompleted bar
        if (barfilled < barlength)
        {
            dc.setForeground(barBGColor);
            dc.fillRectangle(border, border, width - (border << 1), barlength - barfilled);
        }

        // Draw text
        if (options & PROGRESSBAR_PERCENTAGE)
        {
            dc.setFont(font);
            snprintf(numtext, sizeof(numtext), "%d%%", percent);
            n = strlen(numtext);
            tw = font->getTextWidth(numtext, n);
            th = font->getFontHeight();
            ty = (height - th) / 2 + font->getFontAscent();
            tx = (width - tw) / 2;
            if (height - border - barfilled > ty) // In upper side
            {
                dc.setForeground(textNumColor);
                dc.setClipRectangle(border, border, width - (border << 1), height - (border << 1));
                dc.drawText(tx, ty, numtext, n);
            }
            else if (ty - th > height - border - barfilled) // In lower side
            {
                dc.setForeground(textAltColor);
                dc.setClipRectangle(border, border, width - (border << 1), height - (border << 1));
                dc.drawText(tx, ty, numtext, n);
            }
            else                          // In between!
            {
                dc.setForeground(textAltColor);
                dc.setClipRectangle(border, height - border - barfilled, width - (border << 1), barfilled);
                dc.drawText(tx, ty, numtext, n);
                dc.setForeground(textNumColor);
                dc.setClipRectangle(border, border, width - (border << 1), barlength - barfilled);
                dc.drawText(tx, ty, numtext, n);
                dc.clearClipRectangle();
            }
        }
    }
    // Horizontal bar
    else
    {
        // If total is 0, it's 100%
        barlength = width - border - border;
        barfilled = barlength;
        percent = 100;
        if (total != 0)
        {
            barfilled = (FXuint)(((double)progress * (double)barlength) / (double)total);
            percent = (FXuint)(((double)progress * 100.0) / (double)total);
        }

        // Draw uncompleted bar
        if (barfilled < barlength)
        {
            // Ggradient with rounded corners
            dc.setForeground(barBGColor);
            dc.fillRectangle(border + barfilled + (border >> 1), border >> 1, barlength - barfilled, height - border);
        }

        // Draw completed bar
        if (0 < barfilled)
        {
            // Gradient with rounded corners
            drawGradientRectangle(dc, topcolor, bottomcolor, border - 1, border - 1, barfilled + 2, height - border,
                                  true);
            dc.setForeground(bordercolor);
            dc.fillRectangle(barfilled + 3, 2, 1, height - (border << 1));
            dc.drawPoints(bordercorners, 4);

            FXPoint barcorners[2] = { FXPoint(barfilled + 2, 1), FXPoint(barfilled + 2, height - border) };
            dc.drawPoints(barcorners, 2);
        }
        // Draw text
        if (options & PROGRESSBAR_PERCENTAGE)
        {
            dc.setFont(font);
            snprintf(numtext, sizeof(numtext), "%d%%", percent);
            n = strlen(numtext);
            tw = font->getTextWidth(numtext, n);
            th = font->getFontHeight();
            ty = (height - th) / 2 + font->getFontAscent();
            tx = (width - tw) / 2;
            if (border + barfilled <= tx) // In right side
            {
                dc.setForeground(textNumColor);
                dc.setClipRectangle(border, border, width - (border << 1), height - (border << 1));
                dc.drawText(tx, ty, numtext, n);
            }
            else if (tx + tw <= border + barfilled) // In left side
            {
                dc.setForeground(textAltColor);
                dc.setClipRectangle(border, border, width - (border << 1), height - (border << 1));
                dc.drawText(tx, ty, numtext, n);
            }
            else                    // In between!
            {
                dc.setForeground(textAltColor);
                dc.setClipRectangle(border, border, barfilled, height);
                dc.drawText(tx, ty, numtext, n);
                dc.setForeground(textNumColor);
                dc.setClipRectangle(border + barfilled, border, barlength - barfilled, height);
                dc.drawText(tx, ty, numtext, n);
                dc.clearClipRectangle();
            }
        }
    }
}


// Draw the progress bar
long FXProgressBar::onPaint(FXObject*, FXSelector, void* ptr)
{
    // Initialize modern controls
    INIT_MODERN

    FXEvent* event = (FXEvent*)ptr;
    FXDCWindow dc(this, event);

    // Draw borders if any
    drawFrame(dc, 0, 0, width, height);

    // Background
    dc.setForeground(getBaseColor());
    dc.fillRectangle(border, border, width - (border << 1), height - (border << 1));

    // !!! Hack to get a rounded rectangle shape only if _TEXTFIELD_NOFRAME is not specified !!!
    if (!(options & _TEXTFIELD_NOFRAME))
    {
        // Outside Background
        dc.setForeground(baseColor);
        dc.fillRectangle(0, 0, width, height);
        dc.drawPoints(basebackground, 4);

        // Border
        dc.setForeground(bordercolor);
        dc.drawRectangle(2, 0, width - 5, 0);
        dc.drawRectangle(2, height - 1, width - 5, height - 1);
        dc.drawRectangle(0, 2, 0, height - 5);
        dc.drawRectangle(width - 1, 2, 0, height - 5);
        dc.drawPoints(bordercorners, 4);
        dc.setForeground(shadecolor);
        dc.drawPoints(bordershade, 16);
        dc.setForeground(backColor);
        dc.fillRectangle(2, 1, width - 4, height - 2);
    }
    // !!! End of hack

    // Interior
    drawInterior(dc);
    return 1;
}


//
// Hack of FXPacker
//


// This hack draws a rectangle with rounded corners and supports dark theme
void FXPacker::drawGrooveRectangle(FXDCWindow& dc, FXint x, FXint y, FXint w, FXint h)
{
    static FXbool init = true;
    static FXColor bordercolor, shadecolor, shadowcolor;

    FXPoint bordershade[16] =
    {
        FXPoint(x, y + 1), FXPoint(x + 1, y), FXPoint(x + 1, y + 2), FXPoint(x + 2, y + 1),
        FXPoint(x + w - 2, y), FXPoint(x + w - 1, y + 1), FXPoint(x + w - 3, y + 1),
        FXPoint(x + w - 2, y + 2), FXPoint(x, y + h - 2), FXPoint(x + 1, y + h - 1),
        FXPoint(x + 1, y + h - 3), FXPoint(x + 2, y + h - 2),
        FXPoint(x + w - 1, y + h - 2), FXPoint(x + w - 2, y + h - 1),
        FXPoint(x + w - 2, y + h - 3), FXPoint(x + w - 3, y + h - 2)
    };
    FXPoint bordercorners[4] =
    {
        FXPoint(x + 1, y + 1), FXPoint(x + 1, y + h - 2), FXPoint(x + w - 2, y + 1),
        FXPoint(x + w - 2, y + h - 2)
    };

    if (init)
    {
        FXuint rk = FXREDVAL(backColor);
        FXuint gk = FXGREENVAL(backColor);
        FXuint bk = FXBLUEVAL(backColor);

        bordercolor = getApp()->getBorderColor();
        FXuint rb = FXREDVAL(bordercolor);
        FXuint gb = FXGREENVAL(bordercolor);
        FXuint bb = FXBLUEVAL(bordercolor);

        // Dark theme
        if ((rk + gk + bk) / 3 < DARK_COLOR_THRESHOLD)
        {
            shadecolor = FXRGB(FXMIN(1.1 * rk, 255), FXMIN(1.1 * gk, 255), FXMIN(1.1 * bk, 255));
            shadowcolor = FXRGB(FXMIN(1.1 * rb, 255), FXMIN(1.1 * gb, 255), FXMIN(1.1 * bb, 255));
        }
        else // Light theme
        {
            shadecolor = FXRGB(0.9 * rk, 0.9 * gk, 0.9 * bk);
            shadowcolor = FXRGB(0.9 * rb, 0.9 * gb, 0.9 * bb);
        }

        init = false;
    }

    if ((0 < w) && (0 < h))
    {
        // Rectangle with rounded corners
        // Draw the 4 edges
        dc.setForeground(bordercolor);
        dc.drawRectangle(x + w - 1, y + 2, 0, h - 5);   // right
        dc.drawRectangle(x, y + 2, 0, h - 5);           // left
        dc.drawRectangle(x + 2, y, w - 5, 0);           // up
        dc.drawRectangle(x + 2, y + h - 1, w - 5, 0);   // down

        // Draw the 4 rounded corners (with shade)
        dc.setForeground(shadowcolor);
        dc.drawPoints(bordercorners, 4);
        dc.setForeground(shadecolor);
        dc.drawPoints(bordershade, 16);
    }
}


//
// Hack of FXFrame
//

// Identical to FXPacker::drawGrooveRectangle()

// This hack draws a rectangle with rounded corners and supports dark theme
void FXFrame::drawGrooveRectangle(FXDCWindow& dc, FXint x, FXint y, FXint w, FXint h)
{
    static FXbool init = true;
    static FXColor bordercolor, shadecolor, shadowcolor;

    FXPoint bordershade[16] =
    {
        FXPoint(x, y + 1), FXPoint(x + 1, y), FXPoint(x + 1, y + 2), FXPoint(x + 2, y + 1),
        FXPoint(x + w - 2, y), FXPoint(x + w - 1, y + 1), FXPoint(x + w - 3, y + 1),
        FXPoint(x + w - 2, y + 2), FXPoint(x, y + h - 2), FXPoint(x + 1, y + h - 1),
        FXPoint(x + 1, y + h - 3), FXPoint(x + 2, y + h - 2),
        FXPoint(x + w - 1, y + h - 2), FXPoint(x + w - 2, y + h - 1),
        FXPoint(x + w - 2, y + h - 3), FXPoint(x + w - 3, y + h - 2)
    };
    FXPoint bordercorners[4] =
    {
        FXPoint(x + 1, y + 1), FXPoint(x + 1, y + h - 2), FXPoint(x + w - 2, y + 1),
        FXPoint(x + w - 2, y + h - 2)
    };

    if (init)
    {
        FXuint rk = FXREDVAL(backColor);
        FXuint gk = FXGREENVAL(backColor);
        FXuint bk = FXBLUEVAL(backColor);

        bordercolor = getApp()->getBorderColor();
        FXuint rb = FXREDVAL(bordercolor);
        FXuint gb = FXGREENVAL(bordercolor);
        FXuint bb = FXBLUEVAL(bordercolor);

        // Dark theme
        if ((rk + gk + bk) / 3 < DARK_COLOR_THRESHOLD)
        {
            shadecolor = FXRGB(FXMIN(1.1 * rk, 255), FXMIN(1.1 * gk, 255), FXMIN(1.1 * bk, 255));
            shadowcolor = FXRGB(FXMIN(1.1 * rb, 255), FXMIN(1.1 * gb, 255), FXMIN(1.1 * bb, 255));
        }
        else // Light theme
        {
            shadecolor = FXRGB(0.9 * rk, 0.9 * gk, 0.9 * bk);
            shadowcolor = FXRGB(0.9 * rb, 0.9 * gb, 0.9 * bb);
        }

        init = false;
    }

    if ((0 < w) && (0 < h))
    {
        // Rectangle with rounded corners
        // Draw the 4 edges
        dc.setForeground(bordercolor);
        dc.drawRectangle(x + w - 1, y + 2, 0, h - 5);   // right
        dc.drawRectangle(x, y + 2, 0, h - 5);           // left
        dc.drawRectangle(x + 2, y, w - 5, 0);           // up
        dc.drawRectangle(x + 2, y + h - 1, w - 5, 0);   // down

        // Draw the 4 rounded corners (with shade)
        dc.setForeground(shadowcolor);
        dc.drawPoints(bordercorners, 4);
        dc.setForeground(shadecolor);
        dc.drawPoints(bordershade, 16);
    }
}


// Handle dark theme
void FXFrame::drawDoubleRaisedRectangle(FXDCWindow& dc, FXint x, FXint y, FXint w, FXint h)
{
    // !!! Hack here !!!
    FXColor backcolor = getApp()->getBackColor();
    FXuint rk = FXREDVAL(backcolor);
    FXuint gk = FXREDVAL(backcolor);
    FXuint bk = FXREDVAL(backcolor);

    // Dark theme
    if ((rk + gk + bk) / 3 < DARK_COLOR_THRESHOLD)
    {
        if (0 < w && 0 < h)
        {
            dc.setForeground(hiliteColor);
            dc.fillRectangle(x, y + h - 1, w, 1);
            dc.fillRectangle(x + w - 1, y, 1, h);
            dc.setForeground(borderColor);
            dc.fillRectangle(x, y, w - 1, 1);
            dc.fillRectangle(x, y, 1, h - 1);
            if (1 < w && 1 < h)
            {
                dc.setForeground(baseColor);
                dc.fillRectangle(x + 1, y + 1, w - 2, 1);
                dc.fillRectangle(x + 1, y + 1, 1, h - 2);
                dc.setForeground(shadowColor);
                dc.fillRectangle(x + 1, y + h - 2, w - 2, 1);
                dc.fillRectangle(x + w - 2, y + 1, 1, h - 2);
            }
        }
    }
    else // Light theme
    {
        if (0 < w && 0 < h)
        {
            dc.setForeground(borderColor);
            dc.fillRectangle(x, y + h - 1, w, 1);
            dc.fillRectangle(x + w - 1, y, 1, h);
            dc.setForeground(hiliteColor);
            dc.fillRectangle(x, y, w - 1, 1);
            dc.fillRectangle(x, y, 1, h - 1);
            if (1 < w && 1 < h)
            {
                dc.setForeground(baseColor);
                dc.fillRectangle(x + 1, y + 1, w - 2, 1);
                dc.fillRectangle(x + 1, y + 1, 1, h - 2);
                dc.setForeground(shadowColor);
                dc.fillRectangle(x + 1, y + h - 2, w - 2, 1);
                dc.fillRectangle(x + w - 2, y + 1, 1, h - 2);
            }
        }
    }
}


//
// Hack of FXMenuRadio
//

// For HiDPI scaling, remove highlight part of grayed text and support dark theme

#define LEADSPACE   22
#define TRAILSPACE  16


// Handle repaint
long FXMenuRadio::onPaint(FXObject*, FXSelector, void* ptr)
{
    FXEvent* ev = (FXEvent*)ptr;
    FXDCWindow dc(this, ev);
    FXint xx, yy;

    xx = LEADSPACE;

    // Grayed out
    if (!isEnabled())
    {
        dc.setForeground(backColor);
        dc.fillRectangle(0, 0, width, height);
        if (!label.empty())
        {
            yy = font->getFontAscent() + (height - font->getFontHeight()) / 2;
            dc.setFont(font);

            // !!! Hack here !!!
            FXColor backcolor = getApp()->getBackColor();
            FXuint rk = FXREDVAL(backcolor);
            FXuint gk = FXREDVAL(backcolor);
            FXuint bk = FXREDVAL(backcolor);

            FXColor bordercolor = getApp()->getBorderColor();
            FXuint r = FXREDVAL(bordercolor);
            FXuint g = FXGREENVAL(bordercolor);
            FXuint b = FXBLUEVAL(bordercolor);

            FXColor color;
            if ((rk + gk + bk) / 3 < DARK_COLOR_THRESHOLD)
            {
                color = FXRGB(FXMIN(1.1 * r, 255), FXMIN(1.1 * g, 255), FXMIN(1.1 * b, 255));
            }
            else
            {
                color = bordercolor;
            }

            dc.setForeground(color);
            dc.drawText(xx, yy, label);
            if (!accel.empty())
            {
                dc.drawText(width - TRAILSPACE - font->getTextWidth(accel), yy, accel);
            }
            if (0 <= hotoff)
            {
                dc.fillRectangle(xx + font->getTextWidth(&label[0], hotoff), yy + 1,
                                 font->getTextWidth(&label[hotoff], wclen(&label[hotoff])), 1);
            }
        }
    }
    // Active
    else if (isActive())
    {
        dc.setForeground(selbackColor);
        dc.fillRectangle(0, 0, width, height);
        if (!label.empty())
        {
            yy = font->getFontAscent() + (height - font->getFontHeight()) / 2;
            dc.setFont(font);
            dc.setForeground(isEnabled() ? seltextColor : shadowColor);
            dc.drawText(xx, yy, label);
            if (!accel.empty())
            {
                dc.drawText(width - TRAILSPACE - font->getTextWidth(accel), yy, accel);
            }
            if (0 <= hotoff)
            {
                dc.fillRectangle(xx + font->getTextWidth(&label[0], hotoff), yy + 1,
                                 font->getTextWidth(&label[hotoff], wclen(&label[hotoff])), 1);
            }
        }
    }
    // Normal
    else
    {
        dc.setForeground(backColor);
        dc.fillRectangle(0, 0, width, height);
        if (!label.empty())
        {
            yy = font->getFontAscent() + (height - font->getFontHeight()) / 2;
            dc.setFont(font);
            dc.setForeground(textColor);
            dc.drawText(xx, yy, label);
            if (!accel.empty())
            {
                dc.drawText(width - TRAILSPACE - font->getTextWidth(accel), yy, accel);
            }
            if (0 <= hotoff)
            {
                dc.fillRectangle(xx + font->getTextWidth(&label[0], hotoff), yy + 1,
                                 font->getTextWidth(&label[hotoff], wclen(&label[hotoff])), 1);
            }
        }
    }

    // Draw the radio
    xx = 5 / scaleint;
    yy = (height - 9 * scaleint) / 2;

    if (!isEnabled())
    {
        dc.setForeground(backColor);
    }
    else
    {
        dc.setForeground(radioColor);
    }
    dc.fillArc(xx, yy, 9 * scaleint, 9 * scaleint, 0, 360 * 64);
    dc.setForeground(shadowColor);
    dc.drawArc(xx, yy, 9 * scaleint, 9 * scaleint, 0, 360 * 64);

    // Draw the bullit
    if (check != FALSE)
    {
        FXRectangle recs[3];
        recs[0].x = xx + 4 * scaleint;
        recs[0].y = yy + 3 * scaleint;
        recs[0].w = 2 * scaleint;
        recs[0].h = 1 * scaleint;
        recs[1].x = xx + 3 * scaleint;
        recs[1].y = yy + 4 * scaleint;
        recs[1].w = 4 * scaleint;
        recs[1].h = 2 * scaleint;
        recs[2].x = xx + 4 * scaleint;
        recs[2].y = yy + 6 * scaleint;
        recs[2].w = 2 * scaleint;
        recs[2].h = 1 * scaleint;
        if (isEnabled())
        {
            if (check == MAYBE)
            {
                dc.setForeground(shadowColor);
            }
            else
            {
                dc.setForeground(textColor);
            }
        }
        else
        {
            dc.setForeground(shadowColor);
        }
        dc.fillRectangles(recs, 3);
    }

    return 1;
}


//
// Hack of FXMenuCheck
//

// For HiDPI scaling, remove the highlight part of the disabled text and support dark theme

// Handle repaint
long FXMenuCheck::onPaint(FXObject*, FXSelector, void* ptr)
{
    FXEvent* ev = (FXEvent*)ptr;
    FXDCWindow dc(this, ev);
    FXint xx, yy;

    xx = LEADSPACE;

    // Grayed out
    if (!isEnabled())
    {
        dc.setForeground(backColor);
        dc.fillRectangle(0, 0, width, height);
        if (!label.empty())
        {
            yy = font->getFontAscent() + (height - font->getFontHeight()) / 2;
            dc.setFont(font);

            // !!! Hack here !!!
            FXColor backcolor = getApp()->getBackColor();
            FXuint rk = FXREDVAL(backcolor);
            FXuint gk = FXREDVAL(backcolor);
            FXuint bk = FXREDVAL(backcolor);

            FXColor bordercolor = getApp()->getBorderColor();
            FXuint r = FXREDVAL(bordercolor);
            FXuint g = FXGREENVAL(bordercolor);
            FXuint b = FXBLUEVAL(bordercolor);

            FXColor color;
            if ((rk + gk + bk) / 3 < DARK_COLOR_THRESHOLD)
            {
                color = FXRGB(FXMIN(1.1 * r, 255), FXMIN(1.1 * g, 255), FXMIN(1.1 * b, 255));
            }
            else
            {
                color = bordercolor;
            }

            dc.setForeground(color);
            dc.drawText(xx, yy, label);
            if (!accel.empty())
            {
                dc.drawText(width - TRAILSPACE - font->getTextWidth(accel), yy, accel);
            }
            if (0 <= hotoff)
            {
                dc.fillRectangle(xx + font->getTextWidth(&label[0], hotoff), yy + 1,
                                 font->getTextWidth(&label[hotoff], wclen(&label[hotoff])), 1);
            }
        }
    }
    // Active
    else if (isActive())
    {
        dc.setForeground(selbackColor);
        dc.fillRectangle(0, 0, width, height);
        if (!label.empty())
        {
            yy = font->getFontAscent() + (height - font->getFontHeight()) / 2;
            dc.setFont(font);
            dc.setForeground(isEnabled() ? seltextColor : shadowColor);
            dc.drawText(xx, yy, label);
            if (!accel.empty())
            {
                dc.drawText(width - TRAILSPACE - font->getTextWidth(accel), yy, accel);
            }
            if (0 <= hotoff)
            {
                dc.fillRectangle(xx + font->getTextWidth(&label[0], hotoff), yy + 1,
                                 font->getTextWidth(&label[hotoff], wclen(&label[hotoff])), 1);
            }
        }
    }
    // Normal
    else
    {
        dc.setForeground(backColor);
        dc.fillRectangle(0, 0, width, height);
        if (!label.empty())
        {
            yy = font->getFontAscent() + (height - font->getFontHeight()) / 2;
            dc.setFont(font);
            dc.setForeground(textColor);
            dc.drawText(xx, yy, label);
            if (!accel.empty())
            {
                dc.drawText(width - TRAILSPACE - font->getTextWidth(accel), yy, accel);
            }
            if (0 <= hotoff)
            {
                dc.fillRectangle(xx + font->getTextWidth(&label[0], hotoff), yy + 1,
                                 font->getTextWidth(&label[hotoff], wclen(&label[hotoff])), 1);
            }
        }
    }

    // Draw the box
    xx = 5 / scaleint;
    yy = (height - 9 * scaleint) / 2;
    if (!isEnabled())
    {
        dc.setForeground(backColor);
    }
    else
    {
        dc.setForeground(boxColor);
    }
    dc.fillRectangle(xx + 1, yy + 1, 8 * scaleint, 8 * scaleint);
    dc.setForeground(shadowColor);
    dc.drawRectangle(xx, yy, 9 * scaleint, 9 * scaleint);

    // Draw the check
    if (check != FALSE)
    {
        FXRectangle recs[7];
        recs[0].x = xx + 2 * scaleint;
        recs[0].y = yy + 4 * scaleint;
        recs[0].w = 1 * scaleint;
        recs[0].h = 3 * scaleint;
        recs[1].x = xx + 3 * scaleint;
        recs[1].y = yy + 5 * scaleint;
        recs[1].w = 1 * scaleint;
        recs[1].h = 3 * scaleint;
        recs[2].x = xx + 4 * scaleint;
        recs[2].y = yy + 6 * scaleint;
        recs[2].w = 1 * scaleint;
        recs[2].h = 3 * scaleint;
        recs[3].x = xx + 5 * scaleint;
        recs[3].y = yy + 5 * scaleint;
        recs[3].w = 1 * scaleint;
        recs[3].h = 3 * scaleint;
        recs[4].x = xx + 6 * scaleint;
        recs[4].y = yy + 4 * scaleint;
        recs[4].w = 1 * scaleint;
        recs[4].h = 3 * scaleint;
        recs[5].x = xx + 7 * scaleint;
        recs[5].y = yy + 3 * scaleint;
        recs[5].w = 1 * scaleint;
        recs[5].h = 3 * scaleint;
        recs[6].x = xx + 8 * scaleint;
        recs[6].y = yy + 2 * scaleint;
        recs[6].w = 1 * scaleint;
        recs[6].h = 3 * scaleint;

        if (isEnabled())
        {
            if (check == MAYBE)
            {
                dc.setForeground(shadowColor);
            }
            else
            {
                dc.setForeground(textColor);
            }
        }
        else
        {
            dc.setForeground(shadowColor);
        }
        dc.fillRectangles(recs, 7);
    }

    return 1;
}


//
// Hack of FXTreeList
//

// For HiDPI scaling


#define HALFBOX_SIZE        4   // Half box size

// Draw item list
long FXTreeList::onPaint(FXObject*, FXSelector, void* ptr)
{
    FXEvent* event = (FXEvent*)ptr;
    FXTreeItem* item = firstitem;
    FXTreeItem* p;
    FXint yh, xh, x, y, w, h, xp, hh;
    FXDCWindow dc(this, event);

    dc.setFont(font);
    x = pos_x;
    y = pos_y;
    if (options & TREELIST_ROOT_BOXES)
    {
        x += (4 + indent);
    }
    while (item && y < event->rect.y + event->rect.h)
    {
        w = item->getWidth(this);
        h = item->getHeight(this);
        if (event->rect.y <= y + h)
        {
            // Draw item
            dc.setForeground(backColor);
            dc.fillRectangle(0, y, width, h);
            item->draw(this, dc, x, y, w, h);

            // Show other paraphernalia such as dotted lines and expand-boxes
            if ((options & (TREELIST_SHOWS_LINES | TREELIST_SHOWS_BOXES)) &&
                (item->parent || (options & TREELIST_ROOT_BOXES)))
            {
                hh = h / 2;
                yh = y + hh;
                xh = x - indent + (SIDE_SPACING / 2) - (scaleint - 1) * SIDE_SPACING;
                dc.setForeground(lineColor);
                dc.setBackground(backColor);
                dc.setStipple(STIPPLE_GRAY, pos_x & 1, pos_y & 1);
                if (options & TREELIST_SHOWS_LINES)   // Connect items with lines
                {
                    p = item->parent;
                    xp = xh;
                    dc.setFillStyle(FILL_OPAQUESTIPPLED);
                    while (p)
                    {
                        xp -= (indent + p->getHeight(this) / 2);
                        if (p->next)
                        {
                            dc.fillRectangle(xp, y, 1, h);
                        }
                        p = p->parent;
                    }
                    if ((options & TREELIST_SHOWS_BOXES) && (item->hasItems() || item->getFirst()))
                    {
                        if (item->prev || item->parent)
                        {
                            dc.fillRectangle(xh, y, 1, yh - y - scaleint * HALFBOX_SIZE);
                        }
                        if (item->next)
                        {
                            dc.fillRectangle(xh, yh + scaleint * HALFBOX_SIZE, 1,
                                             (y + h - yh - scaleint * HALFBOX_SIZE));
                        }
                    }
                    else
                    {
                        if (item->prev || item->parent)
                        {
                            dc.fillRectangle(xh, y, 1, hh);
                        }
                        if (item->next)
                        {
                            dc.fillRectangle(xh, yh, 1, h);
                        }
                        dc.fillRectangle(xh, yh, x + (SIDE_SPACING / 2) - scaleint * 2 - xh, 1);
                    }
                    dc.setFillStyle(FILL_SOLID);
                }

                // Boxes before items for expand/collapse of item
                if ((options & TREELIST_SHOWS_BOXES) && (item->hasItems() || item->getFirst()))
                {
                    dc.setFillStyle(FILL_OPAQUESTIPPLED);
                    dc.fillRectangle((xh + scaleint * 4), yh, scaleint * ((SIDE_SPACING / 2) - 2), scaleint * 1);
                    dc.setFillStyle(FILL_SOLID);
                    dc.drawRectangle(xh - scaleint * HALFBOX_SIZE, yh - scaleint * HALFBOX_SIZE,
                                     scaleint * (HALFBOX_SIZE + HALFBOX_SIZE),
                                     scaleint * (HALFBOX_SIZE + HALFBOX_SIZE));
                    dc.setForeground(textColor);
                    dc.fillRectangle(xh - scaleint * (HALFBOX_SIZE - 2), yh,
                                     scaleint * (HALFBOX_SIZE + HALFBOX_SIZE - 3), scaleint * 1);
                    if (!(options & TREELIST_AUTOSELECT) && !item->isExpanded())
                    {
                        dc.fillRectangle(xh, yh - scaleint * (HALFBOX_SIZE - 2), scaleint * 1,
                                         scaleint * (HALFBOX_SIZE + HALFBOX_SIZE - 3));
                    }
                }
            }
        }

        y += h;

        // Move on to the next item
        if (item->first && ((options & TREELIST_AUTOSELECT) || item->isExpanded()))
        {
            x += (indent + h / 2);
            item = item->first;
            continue;
        }
        while (!item->next && item->parent)
        {
            item = item->parent;
            x -= (indent + item->getHeight(this) / 2);
        }
        item = item->next;
    }
    if (y < event->rect.y + event->rect.h)
    {
        dc.setForeground(backColor);
        dc.fillRectangle(event->rect.x, y, event->rect.w, event->rect.y + event->rect.h - y);
    }
    return 1;
}


//
// Hack of FXHeader
//


#define ICON_SPACING  4


// !!! Draw filled arrows with custom arrow color, supporting dark theme !!!
// Draw item
void FXHeaderItem::draw(const FXHeader* header, FXDC& dc, FXint x, FXint y, FXint w, FXint h)
{
    FXint tx, ty, tw, th, ix, iy, iw, ih, s, ml, mr, mt, mb, beg, end, t, xx, yy, bb, aa, ax, ay, ah;
    FXFont* font = header->getFont();

    // Get border width and padding
    bb = header->getBorderWidth();
    ml = header->getPadLeft() + bb;
    mr = header->getPadRight() + bb;
    mt = header->getPadTop() + bb;
    mb = header->getPadBottom() + bb;

    // Shrink by margins
    x += ml;
    w -= ml + mr;
    y += mt;
    h -= mt + mb;

    // Initial clip rectangle
    dc.setClipRectangle(x, y, w, h);

    // Text width and height
    tw = th = iw = ih = beg = s = 0;
    do
    {
        end = beg;
        while (end < label.length() && label[end] != '\n')
        {
            end++;
        }
        if ((t = font->getTextWidth(&label[beg], end - beg)) > tw)
        {
            tw = t;
        }
        th += font->getFontHeight();
        beg = end + 1;
    }
    while (end < label.length());

    // Icon size
    if (icon)
    {
        iw = icon->getWidth();
        ih = icon->getHeight();
    }

    // Icon-text spacing
    if (iw && tw)
    {
        s = ICON_SPACING;
    }

    // Arrow color
    FXuint rk = FXREDVAL(header->getBackColor());
    FXuint gk = FXGREENVAL(header->getBackColor());
    FXuint bk = FXBLUEVAL(header->getBackColor());

    FXuint r = FXREDVAL(header->getBorderColor());
    FXuint g = FXGREENVAL(header->getBorderColor());
    FXuint b = FXBLUEVAL(header->getBorderColor());

    FXColor arrowcolor;

    // Dark theme
    if ((rk + gk + bk) / 3 < DARK_COLOR_THRESHOLD)
    {
        arrowcolor = FXRGB(FXMIN(1.5 * r, 255), FXMIN(1.5 * g, 255), FXMIN(1.5 * b, 255));
    }
    else // Light theme
    {
        arrowcolor = FXRGB(0.7 * r, 0.7 * g, 0.7 * b);
    }

    // Draw arrows
    // !!! Hack to draw filled arrows with text color !!!
    if (state & (ARROW_UP | ARROW_DOWN))
    {
        aa = (font->getFontHeight() - 5) | 1;
        ay = y + (h - aa) / 2;
        ax = x + w - aa - 2;
        if (state & ARROW_UP)
        {
            FXPoint points[3];
            ah = 0.9 * (aa + 1);
            points[0].x = ax + ah / 2;
            points[0].y = ay;
            points[1].x = ax + ah;
            points[1].y = ay + ah;
            points[2].x = ax;
            points[2].y = ay + ah;
            dc.setForeground(arrowcolor);
            dc.fillPolygon(points, 3);
        }
        else
        {
            FXPoint points[3];
            ah = 0.9 * aa;
            points[0].x = ax;
            points[0].y = ay + 1;
            points[1].x = ax + ah;
            points[1].y = ay + 1;
            points[2].x = ax + ah / 2;
            points[2].y = ay + ah + 1;
            dc.setForeground(arrowcolor);
            dc.fillPolygon(points, 3);
        }
        w -= aa + 4;
        dc.setClipRectangle(x, y, w, h);
    }

    // Fix x coordinate
    if (state & LEFT)
    {
        if (state & BEFORE)
        {
            ix = x; tx = ix + iw + s;
        }
        else if (state & AFTER)
        {
            tx = x; ix = tx + tw + s;
        }
        else
        {
            ix = x; tx = x;
        }
    }
    else if (state & RIGHT)
    {
        if (state & BEFORE)
        {
            tx = x + w - tw;
            ix = tx - iw - s;
        }
        else if (state & AFTER)
        {
            ix = x + w - iw;
            tx = ix - tw - s;
        }
        else
        {
            ix = x + w - iw;
            tx = x + w - tw;
        }
    }
    else
    {
        if (state & BEFORE)
        {
            ix = x + (w - tw - iw - s) / 2;
            tx = ix + iw + s;
        }
        else if (state & AFTER)
        {
            tx = x + (w - tw - iw - s) / 2;
            ix = tx + tw + s;
        }
        else
        {
            ix = x + (w - iw) / 2;
            tx = x + (w - tw) / 2;
        }
    }

    // Fix y coordinate
    if (state & TOP)
    {
        if (state & ABOVE)
        {
            iy = y;
            ty = iy + ih;
        }
        else if (state & BELOW)
        {
            ty = y;
            iy = ty + th;
        }
        else
        {
            iy = y;
            ty = y;
        }
    }
    else if (state & BOTTOM)
    {
        if (state & ABOVE)
        {
            ty = y + h - th;
            iy = ty - ih;
        }
        else if (state & BELOW)
        {
            iy = y + h - ih;
            ty = iy - th;
        }
        else
        {
            iy = y + h - ih;
            ty = y + h - th;
        }
    }
    else
    {
        if (state & ABOVE)
        {
            iy = y + (h - th - ih) / 2;
            ty = iy + ih;
        }
        else if (state & BELOW)
        {
            ty = y + (h - th - ih) / 2;
            iy = ty + th;
        }
        else
        {
            iy = y + (h - ih) / 2;
            ty = y + (h - th) / 2;
        }
    }

    // Offset a bit when pressed
    if (state & PRESSED)
    {
        tx++;
        ty++;
        ix++;
        iy++;
    }

    // Paint icon
    if (icon)
    {
        dc.drawIcon(icon, ix, iy);
    }

    // Text color
    dc.setForeground(header->getTextColor());

    // Draw text
    yy = ty + font->getFontAscent();
    beg = 0;
    do
    {
        end = beg;
        while (end < label.length() && label[end] != '\n')
        {
            end++;
        }
        if (state & LEFT)
        {
            xx = tx;
        }
        else if (state & RIGHT)
        {
            xx = tx + tw - font->getTextWidth(&label[beg], end - beg);
        }
        else
        {
            xx = tx + (tw - font->getTextWidth(&label[beg], end - beg)) / 2;
        }
        dc.drawText(xx, yy, &label[beg], end - beg);
        yy += font->getFontHeight();
        beg = end + 1;
    }
    while (end < label.length());

    // Restore original clip path
    dc.clearClipRectangle();
}


// !!! Display a simple groove rectangle around header !!!
// Handle repaint
long FXHeader::onPaint(FXObject*, FXSelector, void* ptr)
{
    FXEvent* ev = (FXEvent*)ptr;
    FXDCWindow dc(this, ev);
    FXint x, y, w, h, i, ilo, ihi;

    // Set font
    dc.setFont(font);

    // Paint background
    dc.setForeground(backColor);
    dc.fillRectangle(ev->rect.x, ev->rect.y, ev->rect.w, ev->rect.h);

    // Vertical
    if (options & HEADER_VERTICAL)
    {
        // Determine affected items
        ilo = getItemAt(ev->rect.y);
        ihi = getItemAt(ev->rect.y + ev->rect.h);

        // Fragment below first item
        if (ilo < 0)
        {
            y = pos;
            if (0 < items.no())
            {
                y = pos + items[0]->getPos();
            }
            if (0 < y)
            {
                drawGrooveRectangle(dc, 0, 0, width, y);
            }
            ilo = 0;
        }

        // Fragment above last item
        if (ihi >= items.no())
        {
            y = pos;
            if (0 < items.no())
            {
                y = pos + items[items.no() - 1]->getPos() + items[items.no() - 1]->getSize();
            }
            if (y < height)
            {
                drawGrooveRectangle(dc, 0, y, width, height - y);
            }
            ihi = items.no() - 1;
        }

        // Draw only affected items
        for (i = ilo; i <= ihi; i++)
        {
            y = pos + items[i]->getPos();
            h = items[i]->getSize();
            drawGrooveRectangle(dc, 0, y, width, h - 2);
            items[i]->draw(this, dc, 0, y, width, h);
        }
    }
    // Horizontal
    // !!! Hack to display a simple border rectangle around header !!!
    else
    {
        // Determine affected items
        ilo = getItemAt(ev->rect.x);
        ihi = getItemAt(ev->rect.x + ev->rect.w);

        // Fragment below first item
        if (ilo < 0)
        {
            x = pos;
            if (0 < items.no())
            {
                x = pos + items[0]->getPos();
            }
            if (0 < x)
            {
                drawGrooveRectangle(dc, 0, 0, x, height);
            }
            ilo = 0;
        }

        // Fragment above last item
        if (ihi >= items.no())
        {
            x = pos;
            if (0 < items.no())
            {
                x = pos + items[items.no() - 1]->getPos() + items[items.no() - 1]->getSize();
            }
            if (x < width)
            {
                drawGrooveRectangle(dc, x, 0, width - x, height);
            }
            ihi = items.no() - 1;
        }

        // Draw only the affected items
        for (i = ilo; i <= ihi; i++)
        {
            x = pos + items[i]->getPos();
            w = items[i]->getSize();
            drawGrooveRectangle(dc, x, 0, w - 2, height);
            items[i]->draw(this, dc, x, 0, w, height);
        }
    }

    return 1;
}


//
// Hack of FXMenuSeparator
//


// Handle repaint
long FXMenuSeparator::onPaint(FXObject*, FXSelector, void* ptr)
{
    FXEvent* ev = (FXEvent*)ptr;
    FXDCWindow dc(this, ev);

    dc.setForeground(backColor);
    dc.fillRectangle(ev->rect.x, ev->rect.y, ev->rect.w, ev->rect.h);

    // !!! Change color !!!
    dc.setForeground(getApp()->getBorderColor());
    dc.fillRectangle(1, 0, width, 1);

    return 1;
}


//
// Hack of FXTabItem
//

// Only top tab has been modified


#define TAB_ORIENT_MASK    (TAB_TOP | TAB_LEFT | TAB_RIGHT | TAB_BOTTOM)
#define TABBOOK_MASK       (TABBOOK_SIDEWAYS | TABBOOK_BOTTOMTABS)

// Handle repaint
long FXTabItem::onPaint(FXObject*, FXSelector, void* ptr)
{
    FXEvent* ev = (FXEvent*)ptr;
    FXDCWindow dc(this, ev);
    FXint tw = 0, th = 0, iw = 0, ih = 0, tx, ty, ix, iy;

    dc.setForeground(backColor);
    dc.fillRectangle(ev->rect.x, ev->rect.y, ev->rect.w, ev->rect.h);

    switch (options & TAB_ORIENT_MASK)
    {
    case TAB_LEFT:
        dc.setForeground(hiliteColor);
        dc.drawLine(2, 0, width - 1, 0);
        dc.drawLine(0, 2, 1, 1);
        dc.drawLine(0, height - 4, 0, 2);
        dc.setForeground(shadowColor);
        dc.fillRectangle(1, height - 3, 1, 1);
        dc.fillRectangle(2, height - 2, width - 3, 1);
        dc.setForeground(borderColor);
        dc.drawLine(3, height - 1, width - 1, height - 1);
        break;

    case TAB_RIGHT:
        dc.setForeground(hiliteColor);
        dc.drawLine(0, 0, width - 3, 0);
        dc.drawLine(width - 3, 0, width - 1, 3);
        dc.setForeground(shadowColor);
        dc.drawLine(width - 2, 2, width - 2, height - 2);
        dc.drawLine(0, height - 2, width - 2, height - 2);
        dc.setForeground(borderColor);
        dc.drawLine(0, height - 1, width - 3, height - 1);
        dc.drawLine(width - 1, 3, width - 1, height - 4);
        dc.drawLine(width - 3, height - 1, width - 1, height - 4);
        break;

    case TAB_BOTTOM:
        dc.setForeground(hiliteColor);
        dc.drawLine(0, 0, 0, height - 4);
        dc.drawLine(0, height - 4, 1, height - 2);
        dc.setForeground(shadowColor);
        dc.fillRectangle(2, height - 2, width - 4, 1);
        dc.drawLine(width - 2, 0, width - 2, height - 3);
        dc.fillRectangle(width - 2, 0, 2, 1);
        dc.setForeground(borderColor);
        dc.drawLine(3, height - 1, width - 4, height - 1);
        dc.drawLine(width - 4, height - 1, width - 1, height - 4);
        dc.fillRectangle(width - 1, 1, 1, height - 4);
        break;

    // !!! Change color and remove shadow !!!
    case TAB_TOP:

        // Don't display anything if label is empty
        if (!label.empty())
        {
            dc.setForeground(borderColor);
            dc.fillRectangle(0, 2, 1, height - 2);
            dc.drawLine(0, 2, 2, 0);
            dc.fillRectangle(2, 0, width - 5, 1);
            dc.fillRectangle(width - 2, 2, 1, height - 1);
            dc.fillRectangle(width - 3, 1, 1, 1);
        }

        break;
    }
    if (!label.empty())
    {
        tw = labelWidth(label);
        th = labelHeight(label);
    }
    if (icon)
    {
        iw = icon->getWidth();
        ih = icon->getHeight();
    }
    just_x(tx, ix, tw, iw);
    just_y(ty, iy, th, ih);

    if (icon)
    {
        if (isEnabled())
        {
            dc.drawIcon(icon, ix, iy);
        }
        else
        {
            dc.drawIconSunken(icon, ix, iy);
        }
    }
    if (!label.empty())
    {
        dc.setFont(font);
        if (isEnabled())
        {
            dc.setForeground(textColor);
            drawLabel(dc, label, hotoff, tx, ty, tw, th);

            if (hasFocus())
            {
                dc.drawFocusRectangle(border + 1, border + 1, width - 2 * border - 2, height - 2 * border - 2);
            }
        }
        else
        {
            dc.setForeground(hiliteColor);
            drawLabel(dc, label, hotoff, tx + 1, ty + 1, tw, th);
            dc.setForeground(shadowColor);
            drawLabel(dc, label, hotoff, tx, ty, tw, th);
        }
    }
    return 1;
}


//
// Hack of FXToolBarGrip
//

// Only double vertical toolbar grip has been modified


// Size
#define GRIP_SINGLE  3          // Single grip for arrangable toolbars
#define GRIP_DOUBLE  7          // Double grip for dockable toolbars


// Handle repaint
long FXToolBarGrip::onPaint(FXObject*, FXSelector, void* ptr)
{
    FXEvent* event = static_cast<FXEvent*>(ptr);
    FXDCWindow dc(this, event);
    FXint xx, yy, ww, hh;

    dc.setForeground(backColor);
    dc.fillRectangle(border, border, width - (border << 1), height - (border << 1));
    ww = width - padleft - padright - (border << 1);
    hh = height - padtop - padbottom - (border << 1);
    if (width > height)
    {
        xx = border + padleft;
        if (options & TOOLBARGRIP_DOUBLE)
        { // =
            yy = border + padtop + (hh - GRIP_DOUBLE) / 2;
            dc.setForeground(hiliteColor);
            dc.fillRectangle(xx, yy, 1, 2);
            dc.fillRectangle(xx, yy + 4, 1, 2);
            dc.fillRectangle(xx, yy, ww - 1, 1);
            dc.fillRectangle(xx, yy + 4, ww - 1, 1);
            dc.setForeground(shadowColor);
            dc.fillRectangle(xx + ww - 1, yy, 1, 3);
            dc.fillRectangle(xx + ww - 1, yy + 4, 1, 3);
            dc.fillRectangle(xx, yy + 2, ww - 1, 1);
            dc.fillRectangle(xx, yy + 6, ww - 1, 1);
            if (flags & (FLAG_ACTIVE | FLAG_TRYDRAG | FLAG_DODRAG))
            {
                dc.setForeground(activeColor);
                dc.fillRectangle(xx + 1, yy + 1, ww - 2, 1);
                dc.fillRectangle(xx + 1, yy + 5, ww - 2, 1);
            }
        }
        else
        { // -
            yy = border + padtop + (hh - GRIP_SINGLE) / 2;
            dc.setForeground(hiliteColor);
            dc.fillRectangle(xx, yy, 1, 2);
            dc.fillRectangle(xx, yy, ww - 1, 1);
            dc.setForeground(shadowColor);
            dc.fillRectangle(xx + ww - 1, yy, 1, 3);
            dc.fillRectangle(xx, yy + 2, ww - 1, 1);
            if (flags & (FLAG_ACTIVE | FLAG_TRYDRAG | FLAG_DODRAG))
            {
                dc.setForeground(activeColor);
                dc.fillRectangle(xx + 1, yy + 1, ww - 2, 1);
            }
        }
    }
    else
    {
        yy = border + padtop;
        if (options & TOOLBARGRIP_DOUBLE)
        { // ||
            xx = border + padleft + (ww - GRIP_DOUBLE) / 2;

            // !!! Change color and remove shadow !!!
            dc.setForeground(borderColor);
            dc.fillRectangle(xx, yy, 1, 1);
            dc.fillRectangle(xx + 4, yy, 1, 1);
            dc.fillRectangle(xx, yy, 1, hh - 1);
            dc.fillRectangle(xx + 4, yy, 1, hh - 1);

            if (flags & (FLAG_ACTIVE | FLAG_TRYDRAG | FLAG_DODRAG))
            {
                dc.setForeground(activeColor);
                dc.fillRectangle(xx + 1, yy + 1, 1, hh - 2);
                dc.fillRectangle(xx + 5, yy + 1, 1, hh - 2);
            }
        }
        else
        { // |
            xx = border + padleft + (ww - GRIP_SINGLE) / 2;
            dc.setForeground(hiliteColor);
            dc.fillRectangle(xx, yy, 2, 1);
            dc.fillRectangle(xx, yy, 1, hh - 1);
            dc.setForeground(shadowColor);
            dc.fillRectangle(xx, yy + hh - 1, 3, 1);
            dc.fillRectangle(xx + 2, yy, 1, hh - 1);
            if (flags & (FLAG_ACTIVE | FLAG_TRYDRAG | FLAG_DODRAG))
            {
                dc.setForeground(activeColor);
                dc.fillRectangle(xx + 1, yy + 1, 1, hh - 2);
            }
        }
    }
    drawFrame(dc, 0, 0, width, height);
    return 1;
}


//
// Hack of FXDCWindow
//

// Paint icon as gray instead of sunken and support dark theme


// This draws a sunken icon
void FXDCWindow::drawIconSunken(const FXIcon* icon, FXint dx, FXint dy)
{
    if (!surface)
    {
        fxerror("FXDCWindow::drawIconSunken: DC not connected to drawable.\n");
    }
    if (!icon || !icon->id() || !icon->etch)
    {
        fxerror("FXDCWindow::drawIconSunken: illegal icon specified.\n");
    }
    XGCValues gcv;

    FXColor basecolor = getApp()->getBaseColor();

    // !!! Hack here !!!
    FXColor backcolor = getApp()->getBackColor();
    FXuint rk = FXREDVAL(backcolor);
    FXuint gk = FXREDVAL(backcolor);
    FXuint bk = FXREDVAL(backcolor);

    FXuint r = FXREDVAL(basecolor);
    FXuint g = FXGREENVAL(basecolor);
    FXuint b = FXBLUEVAL(basecolor);

    FXColor clr;
    if ((rk + gk + bk) / 3 < DARK_COLOR_THRESHOLD)
    {
        clr = FXRGB(FXMIN(1.5 * r, 255), FXMIN(1.5 * g, 255), FXMIN(1.5 * b, 255));
    }
    else
    {
        clr = FXRGB(0.8 * r, 0.8 * g, 0.8 * b);
    }

    // Erase to black
    gcv.background = 0;
    gcv.foreground = 0xffffffff;
    gcv.function = BLT_NOT_SRC_AND_DST;
    XChangeGC(DISPLAY(getApp()), (GC)ctx, GCForeground | GCBackground | GCFunction, &gcv);
    XCopyPlane(DISPLAY(getApp()), icon->etch, surface->id(), (GC)ctx, 0, 0, icon->width,
               icon->height, dx + 1, dy + 1, 1);

    // Paint gray part
    gcv.function = BLT_SRC_OR_DST;
    gcv.foreground = visual->getPixel(clr);
    XChangeGC(DISPLAY(getApp()), (GC)ctx, GCForeground | GCFunction, &gcv);
    XCopyPlane(DISPLAY(getApp()), icon->etch, surface->id(), (GC)ctx, 0, 0, icon->width,
               icon->height, dx + 1, dy + 1, 1);

    // Restore stuff
    gcv.foreground = devfg;
    gcv.background = devbg;
    gcv.function = rop;
    XChangeGC(DISPLAY(getApp()), (GC)ctx, GCForeground | GCBackground | GCFunction, &gcv);
}


//
// Hack of FXSeparator
//

// Draw a simple line for groove separator and support dark theme

// Handle repaint
long FXSeparator::onPaint(FXObject*, FXSelector, void* ptr)
{
    FXEvent* ev = (FXEvent*)ptr;
    FXDCWindow dc(this, ev);
    FXint kk, ll;

    // Draw background
    dc.setForeground(backColor);
    dc.fillRectangle(ev->rect.x, ev->rect.y, ev->rect.w, ev->rect.h);

    // Draw frame
    drawFrame(dc, 0, 0, width, height);

    // !!! Hack here !!!
    FXColor backcolor = getApp()->getBackColor();
    FXuint rk = FXREDVAL(backcolor);
    FXuint gk = FXREDVAL(backcolor);
    FXuint bk = FXREDVAL(backcolor);

    FXuint r = FXREDVAL(borderColor);
    FXuint g = FXGREENVAL(borderColor);
    FXuint b = FXBLUEVAL(borderColor);

    FXColor color;
    if ((rk + gk + bk) / 3 < DARK_COLOR_THRESHOLD)
    {
        color = FXRGB(FXMIN(1.1 * r, 255), FXMIN(1.1 * g, 255), FXMIN(1.1 * b, 255));
    }
    else
    {
        color = borderColor;
    }

    // Horizonal orientation
    if ((height - padbottom - padtop) < (width - padleft - padright))
    {
        kk = (options & (SEPARATOR_GROOVE | SEPARATOR_RIDGE)) ? 2 : 1;
        ll = border + padtop + (height - padbottom - padtop - (border << 1) - kk) / 2;
        if (options & SEPARATOR_GROOVE)
        {
            // !!! Hack here !!!
            dc.setForeground(color);
            dc.fillRectangle(border + padleft, ll, width - padright - padleft - (border << 1), 1);
        }
        else if (options & SEPARATOR_RIDGE)
        {
            dc.setForeground(hiliteColor);
            dc.fillRectangle(border + padleft, ll, width - padright - padleft - (border << 1), 1);
            dc.setForeground(shadowColor);
            dc.fillRectangle(border + padleft, ll + 1, width - padright - padleft - (border << 1), 1);
        }
        else if (options & SEPARATOR_LINE)
        {
            dc.setForeground(borderColor);
            dc.fillRectangle(border + padleft, ll, width - padright - padleft - (border << 1), 1);
        }
    }
    // Vertical orientation
    else
    {
        kk = (options & (SEPARATOR_GROOVE | SEPARATOR_RIDGE)) ? 2 : 1;
        ll = border + padleft + (width - padleft - padright - (border << 1) - kk) / 2;
        if (options & SEPARATOR_GROOVE)
        {
            // !!! Hack here !!!
            dc.setForeground(color);
            dc.fillRectangle(ll, padtop + border, 1, height - padtop - padbottom - (border << 1));
        }
        else if (options & SEPARATOR_RIDGE)
        {
            dc.setForeground(hiliteColor);
            dc.fillRectangle(ll, padtop + border, 1, height - padtop - padbottom - (border << 1));
            dc.setForeground(shadowColor);
            dc.fillRectangle(ll + 1, padtop + border, 1, height - padtop - padbottom - (border << 1));
        }
        else if (options & SEPARATOR_LINE)
        {
            dc.setForeground(borderColor);
            dc.fillRectangle(ll, padtop + border, 1, height - padtop - padbottom - (border << 1));
        }
    }
    return 1;
}


//
// Hack of FXMenuCommand
//

// Remove the highlight part of the disabled text and support dark theme
// Blend icon to background or selection background

// Handle repaint
long FXMenuCommand::onPaint(FXObject*, FXSelector, void* ptr)
{
    FXEvent* ev = (FXEvent*)ptr;
    FXDCWindow dc(this, ev);
    FXint xx, yy;

    xx = LEADSPACE;

    // Grayed out
    if (!isEnabled())
    {
        dc.setForeground(backColor);
        dc.fillRectangle(0, 0, width, height);
        if (icon)
        {
            dc.drawIconSunken(icon, 3, (height - icon->getHeight()) / 2);
            if (icon->getWidth() + 5 > xx)
            {
                xx = icon->getWidth() + 5;
            }
        }
        if (!label.empty())
        {
            yy = font->getFontAscent() + (height - font->getFontHeight()) / 2;
            dc.setFont(font);

            // !!! Hack here !!!
            FXColor backcolor = getApp()->getBackColor();
            FXuint rk = FXREDVAL(backcolor);
            FXuint gk = FXREDVAL(backcolor);
            FXuint bk = FXREDVAL(backcolor);

            FXColor bordercolor = getApp()->getBorderColor();
            FXuint r = FXREDVAL(bordercolor);
            FXuint g = FXGREENVAL(bordercolor);
            FXuint b = FXBLUEVAL(bordercolor);

            FXColor color;
            if ((rk + gk + bk) / 3 < DARK_COLOR_THRESHOLD)
            {
                color = FXRGB(FXMIN(1.1 * r, 255), FXMIN(1.1 * g, 255), FXMIN(1.1 * b, 255));
            }
            else
            {
                color = bordercolor;
            }

            dc.setForeground(color);
            dc.drawText(xx + 1, yy + 1, label);
            if (!accel.empty())
            {
                dc.drawText(width - TRAILSPACE - font->getTextWidth(accel) + 1, yy + 1, accel);
            }
            if (0 <= hotoff)
            {
                dc.fillRectangle(xx + font->getTextWidth(&label[0], hotoff) + 1, yy + 2,
                                 font->getTextWidth(&label[hotoff], wclen(&label[hotoff])), 1);
            }
        }
    }
    // Active
    else if (isActive())
    {
        // !!! Hack here !!!
        if (icon)
        {
            FXColor* tmpdata;
            FXuint w = icon->getWidth();
            FXuint h = icon->getHeight();

            // Save original pixels
            if (!FXMEMDUP(&tmpdata, icon->getData(), FXColor, w * h))
            {
                throw FXMemoryException(_("Unable to load image"));
            }

            // Blend to selection color
            icon->blend(selbackColor);
            icon->render();

            // Restore original pixels
            icon->setData(tmpdata, IMAGE_KEEP | IMAGE_OWNED, w, h);
        }

        dc.setForeground(selbackColor);
        dc.fillRectangle(0, 0, width, height);
        if (icon)
        {
            dc.drawIcon(icon, 3, (height - icon->getHeight()) / 2);
            if (icon->getWidth() + 5 > xx)
            {
                xx = icon->getWidth() + 5;
            }
        }
        if (!label.empty())
        {
            yy = font->getFontAscent() + (height - font->getFontHeight()) / 2;
            dc.setFont(font);
            dc.setForeground(isEnabled() ? seltextColor : shadowColor);
            dc.drawText(xx, yy, label);
            if (!accel.empty())
            {
                dc.drawText(width - TRAILSPACE - font->getTextWidth(accel), yy, accel);
            }
            if (0 <= hotoff)
            {
                dc.fillRectangle(xx + font->getTextWidth(&label[0], hotoff), yy + 1,
                                 font->getTextWidth(&label[hotoff], wclen(&label[hotoff])), 1);
            }
        }
    }
    // Normal
    else
    {
        // !!! Hack here !!!
        if (icon)
        {
            FXColor* tmpdata;
            FXuint w = icon->getWidth();
            FXuint h = icon->getHeight();

            // Save original pixels
            if (!FXMEMDUP(&tmpdata, icon->getData(), FXColor, w * h))
            {
                throw FXMemoryException(_("Unable to load image"));
            }

            // Blend to background color
            icon->blend(backColor);
            icon->render();

            // Restore original pixels
            icon->setData(tmpdata, IMAGE_KEEP | IMAGE_OWNED, w, h);
        }

        dc.setForeground(backColor);
        dc.fillRectangle(0, 0, width, height);
        if (icon)
        {
            dc.drawIcon(icon, 3, (height - icon->getHeight()) / 2);
            if (icon->getWidth() + 5 > xx)
            {
                xx = icon->getWidth() + 5;
            }
        }
        if (!label.empty())
        {
            yy = font->getFontAscent() + (height - font->getFontHeight()) / 2;
            dc.setFont(font);
            dc.setForeground(textColor);
            dc.drawText(xx, yy, label);
            if (!accel.empty())
            {
                dc.drawText(width - TRAILSPACE - font->getTextWidth(accel), yy, accel);
            }
            if (0 <= hotoff)
            {
                dc.fillRectangle(xx + font->getTextWidth(&label[0], hotoff), yy + 1,
                                 font->getTextWidth(&label[hotoff], wclen(&label[hotoff])), 1);
            }
        }
    }
    return 1;
}


//
// Hack of FXLabel
//

// Remove the highlight part of the disabled text and support dark theme

// Handle repaint
long FXLabel::onPaint(FXObject*, FXSelector, void* ptr)
{
    FXEvent* ev = (FXEvent*)ptr;
    FXDCWindow dc(this, ev);
    FXint tw = 0, th = 0, iw = 0, ih = 0, tx, ty, ix, iy;

    dc.setForeground(backColor);
    dc.fillRectangle(0, 0, width, height);
    if (!label.empty())
    {
        tw = labelWidth(label);
        th = labelHeight(label);
    }
    if (icon)
    {
        iw = icon->getWidth();
        ih = icon->getHeight();
    }
    just_x(tx, ix, tw, iw);
    just_y(ty, iy, th, ih);
    if (icon)
    {
        if (isEnabled())
        {
            dc.drawIcon(icon, ix, iy);
        }
        else
        {
            dc.drawIconSunken(icon, ix, iy);
        }
    }
    if (!label.empty())
    {
        dc.setFont(font);
        if (isEnabled())
        {
            dc.setForeground(textColor);
            drawLabel(dc, label, hotoff, tx, ty, tw, th);
        }
        else
        {
            // !!! Hack here !!!
            FXColor backcolor = getApp()->getBackColor();
            FXuint rk = FXREDVAL(backcolor);
            FXuint gk = FXREDVAL(backcolor);
            FXuint bk = FXREDVAL(backcolor);

            FXuint r = FXREDVAL(borderColor);
            FXuint g = FXGREENVAL(borderColor);
            FXuint b = FXBLUEVAL(borderColor);

            FXColor color;
            if ((rk + gk + bk) / 3 < DARK_COLOR_THRESHOLD)
            {
                color = FXRGB(FXMIN(1.1 * r, 255), FXMIN(1.1 * g, 255), FXMIN(1.1 * b, 255));
            }
            else
            {
                color = borderColor;
            }

            dc.setForeground(color);
            drawLabel(dc, label, hotoff, tx + 1, ty + 1, tw, th);
        }
    }
    drawFrame(dc, 0, 0, width, height);
    return 1;
}


//
// Hack of FXGroupBox
//

// Remove the highlight part of the disabled text and support dark theme

#define FRAME_MASK      (FRAME_SUNKEN | FRAME_RAISED | FRAME_THICK)

// Handle repaint
long FXGroupBox::onPaint(FXObject*, FXSelector, void* ptr)
{
    FXEvent* event = (FXEvent*)ptr;
    FXDCWindow dc(this, event);
    FXint tw, th, yy, xx;

    xx = 0;
    yy = 0;

    // Paint background
    dc.setForeground(backColor);
    dc.fillRectangle(event->rect.x, event->rect.y, event->rect.w, event->rect.h);

    // Draw label if there is one
    if (!label.empty())
    {
        yy = 2 + font->getFontAscent() / 2;
    }

    // We should really just draw what's exposed!
    switch (options & FRAME_MASK)
    {
    case FRAME_LINE: drawBorderRectangle(dc, 0, yy, width, height - yy); break;
    case FRAME_SUNKEN: drawSunkenRectangle(dc, 0, yy, width, height - yy); break;
    case FRAME_RAISED: drawRaisedRectangle(dc, 0, yy, width, height - yy); break;
    case FRAME_GROOVE: drawGrooveRectangle(dc, 0, yy, width, height - yy); break;
    case FRAME_RIDGE: drawRidgeRectangle(dc, 0, yy, width, height - yy); break;
    case FRAME_SUNKEN | FRAME_THICK: drawDoubleSunkenRectangle(dc, 0, yy, width, height - yy); break;
    case FRAME_RAISED | FRAME_THICK: drawDoubleRaisedRectangle(dc, 0, yy, width, height - yy); break;
    }

    // Draw label
    if (!label.empty())
    {
        tw = font->getTextWidth(label);
        th = font->getFontHeight() + 4;
        if (options & GROUPBOX_TITLE_RIGHT)
        {
            xx = width - tw - 12;
        }
        else if (options & GROUPBOX_TITLE_CENTER)
        {
            xx = (width - tw) / 2 - 4;
        }
        else
        {
            xx = 4;
        }
        if (xx < 4)
        {
            xx = 4;
        }
        if (tw + 16 > width)
        {
            tw = width - 16;
        }
        if (0 < tw)
        {
            dc.setForeground(backColor);
            dc.setFont(font);
            dc.fillRectangle(xx, yy, tw + 8, 2);
            dc.setClipRectangle(xx + 4, 0, tw, th);
            if (isEnabled())
            {
                dc.setForeground(textColor);
                dc.drawText(xx + 4, 2 + font->getFontAscent(), label);
            }
            else
            {
                // !!! Hack here !!!
                FXColor backcolor = getApp()->getBackColor();
                FXuint rk = FXREDVAL(backcolor);
                FXuint gk = FXREDVAL(backcolor);
                FXuint bk = FXREDVAL(backcolor);

                FXuint r = FXREDVAL(borderColor);
                FXuint g = FXGREENVAL(borderColor);
                FXuint b = FXBLUEVAL(borderColor);

                FXColor color;
                if ((rk + gk + bk) / 3 < DARK_COLOR_THRESHOLD)
                {
                    color = FXRGB(FXMIN(1.1 * r, 255), FXMIN(1.1 * g, 255), FXMIN(1.1 * b, 255));
                }
                else
                {
                    color = borderColor;
                }

                dc.setForeground(color);
                dc.drawText(xx + 4, 2 + font->getFontAscent(), label);
            }
        }
    }
    return 1;
}


//
// Hack of FXDragCorner
//

// Use simple lines instead of sunken lines


// Slightly different from Frame border
long FXDragCorner::onPaint(FXObject*, FXSelector, void* ptr)
{
    FXEvent* ev = (FXEvent*)ptr;
    FXDCWindow dc(this, ev);

    dc.setForeground(backColor);
    dc.fillRectangle(ev->rect.x, ev->rect.y, ev->rect.w, ev->rect.h);

    // !!! Hack here !!!
    FXColor backcolor = getApp()->getBackColor();
    FXuint rk = FXREDVAL(backcolor);
    FXuint gk = FXREDVAL(backcolor);
    FXuint bk = FXREDVAL(backcolor);

    FXColor bordercolor = getApp()->borderColor;
    FXuint r = FXREDVAL(bordercolor);
    FXuint g = FXGREENVAL(bordercolor);
    FXuint b = FXBLUEVAL(bordercolor);

    FXColor color;
    if ((rk + gk + bk) / 3 < DARK_COLOR_THRESHOLD)
    {
        color = FXRGB(FXMIN(1.5 * r, 255), FXMIN(1.5 * g, 255), FXMIN(1.5 * b, 255));
    }
    else
    {
        color = FXRGB(0.5 * r, 0.5 * g, 0.5 * b);
    }

    dc.setForeground(color);
    dc.drawLine(width - 2, height - 1, width, height - 3);
    dc.drawLine(width - 8, height - 1, width, height - 9);
    dc.drawLine(width - 14, height - 1, width, height - 15);

    return 1;
}



//
// Hack of FXSlider
//

// Flatter slider


// Draw slider head
void FXSlider::drawSliderHead(FXDCWindow& dc, FXint x, FXint y, FXint w, FXint h)
{
    FXint m;

    dc.setForeground(baseColor);
    dc.fillRectangle(x, y, w, h);
    if (options & SLIDER_VERTICAL)
    {
        m = (h >> 1);
        if (options & SLIDER_ARROW_LEFT)
        {
            dc.setForeground(hiliteColor);
            dc.drawLine(x + m, y, x + w - 1, y);
            dc.drawLine(x, y + m, x + m, y);
            dc.setForeground(shadowColor);
            dc.drawLine(x + 1, y + h - m - 1, x + m + 1, y + h - 1);
            dc.drawLine(x + m, y + h - 2, x + w - 1, y + h - 2);
            dc.drawLine(x + w - 2, y + 1, x + w - 2, y + h - 1);
            dc.setForeground(borderColor);
            dc.drawLine(x, y + h - m - 1, x + m, y + h - 1);
            dc.drawLine(x + w - 1, y + h - 1, x + w - 1, y);
            dc.fillRectangle(x + m, y + h - 1, w - m, 1);
        }
        else if (options & SLIDER_ARROW_RIGHT)
        {
            dc.setForeground(hiliteColor);
            dc.drawLine(x, y, x + w - m - 1, y);
            dc.drawLine(x, y + 1, x, y + h - 1);
            dc.drawLine(x + w - 1, y + m, x + w - m - 1, y);
            dc.setForeground(shadowColor);
            dc.drawLine(x + w - 2, y + h - m - 1, x + w - m - 2, y + h - 1);
            dc.drawLine(x + 1, y + h - 2, x + w - m - 1, y + h - 2);
            dc.setForeground(borderColor);
            dc.drawLine(x + w - 1, y + h - m - 1, x + w - m - 1, y + h - 1);
            dc.drawLine(x, y + h - 1, x + w - m - 1, y + h - 1);
        }
        else if (options & SLIDER_INSIDE_BAR)
        {
            // !!! Hack here !!!
            drawGrooveRectangle(dc, x, y, w, h);
        }
        else
        {
            // !!! Hack here !!!
            drawGrooveRectangle(dc, x, y, w, h);
        }
    }
    else
    {
        m = (w >> 1);
        if (options & SLIDER_ARROW_UP)
        {
            dc.setForeground(hiliteColor);
            dc.drawLine(x, y + m, x + m, y);
            dc.drawLine(x, y + m, x, y + h - 1);
            dc.setForeground(shadowColor);
            dc.drawLine(x + w - 1, y + m + 1, x + w - m - 1, y + 1);
            dc.drawLine(x + w - 2, y + m + 1, x + w - 2, y + h - 1);
            dc.drawLine(x + 1, y + h - 2, x + w - 2, y + h - 2);
            dc.setForeground(borderColor);
            dc.drawLine(x + w - 1, y + m, x + w - m - 1, y);
            dc.drawLine(x + w - 1, y + m, x + w - 1, y + h - 1);
            dc.fillRectangle(x, y + h - 1, w, 1);
        }
        else if (options & SLIDER_ARROW_DOWN)
        {
            dc.setForeground(hiliteColor);
            dc.drawLine(x, y, x + w - 1, y);
            dc.drawLine(x, y + 1, x, y + h - m - 1);
            dc.drawLine(x, y + h - m - 1, x + m, y + h - 1);
            dc.setForeground(shadowColor);
            dc.drawLine(x + w - 2, y + 1, x + w - 2, y + h - m - 1);
            dc.drawLine(x + w - 1, y + h - m - 2, x + w - m - 1, y + h - 2);
            dc.setForeground(borderColor);
            dc.drawLine(x + w - 1, y + h - m - 1, x + w - m - 1, y + h - 1);
            dc.fillRectangle(x + w - 1, y, 1, h - m);
        }
        else if (options & SLIDER_INSIDE_BAR)
        {
            // !!! Hack here !!!
            drawGrooveRectangle(dc, x, y, w, h);
        }
        else
        {
            // !!! Hack here !!!
            drawGrooveRectangle(dc, x, y, w, h);
        }
    }
}


#define TICKSIZE   4   // Length of ticks

// Handle repaint
long FXSlider::onPaint(FXObject*, FXSelector, void* ptr)
{
    FXEvent* event = (FXEvent*)ptr;
    FXint tx, ty, hhs = headsize / 2;
    FXint xx, yy, ww, hh;
    FXDCWindow dc(this, event);

    // Repaint background
    dc.setForeground(backColor);
    dc.fillRectangle(0, 0, width, height);

    // Repaint border
    drawFrame(dc, 0, 0, width, height);

    // Slot placement
    xx = border + padleft;
    yy = border + padtop;
    ww = width - (border << 1) - padleft - padright;
    hh = height - (border << 1) - padtop - padbottom;
    FXASSERT(range[0] <= pos && pos <= range[1]);

    // Draw the slot
    if (options & SLIDER_VERTICAL)
    {
        // Adjust slot placement for tickmarks
        if (options & SLIDER_TICKS_LEFT)
        {
            xx += TICKSIZE; ww -= TICKSIZE;
        }
        if (options & SLIDER_TICKS_RIGHT)
        {
            ww -= TICKSIZE;
        }

        // Draw slider
        if (options & SLIDER_INSIDE_BAR)
        {
            // !!! Hack here !!!
            drawBorderRectangle(dc, xx, yy, ww, hh);

            dc.setStipple(STIPPLE_GRAY);
            dc.setForeground(slotColor);
            dc.setBackground(baseColor);
            dc.setFillStyle(FILL_OPAQUESTIPPLED);
            dc.fillRectangle(xx + 2, yy + 2, ww - 4, hh - 4);
            dc.setFillStyle(FILL_SOLID);
            if (options & SLIDER_TICKS_LEFT)
            {
                drawVertTicks(dc, border + padleft, yy, ww, hh);
            }
            if (options & SLIDER_TICKS_RIGHT)
            {
                drawVertTicks(dc, width - padright - border - TICKSIZE, yy, ww, hh);
            }
            if (isEnabled())
            {
                drawSliderHead(dc, xx + 2, headpos, ww - 4, headsize);
            }
        }
        else
        {
            if (options & SLIDER_ARROW_LEFT)
            {
                tx = xx + hhs + (ww - slotsize - hhs) / 2;
            }
            else if (options & SLIDER_ARROW_RIGHT)
            {
                tx = xx + (ww - slotsize - hhs) / 2;
            }
            else
            {
                tx = xx + (ww - slotsize) / 2;
            }

            // !!! Hack here !!!
            drawBorderRectangle(dc, tx, yy, slotsize, hh);

            dc.setForeground(slotColor);
            dc.fillRectangle(tx + 2, yy + 2, slotsize - 4, hh - 4);
            if (options & SLIDER_TICKS_LEFT)
            {
                drawVertTicks(dc, border + padleft, yy, ww, hh);
            }
            if (options & SLIDER_TICKS_RIGHT)
            {
                drawVertTicks(dc, width - padright - border - TICKSIZE, yy, ww, hh);
            }
            if (isEnabled())
            {
                drawSliderHead(dc, xx, headpos, ww, headsize);
            }
        }
    }
    else
    {
        // Adjust slot placement for tickmarks
        if (options & SLIDER_TICKS_TOP)
        {
            yy += TICKSIZE; hh -= TICKSIZE;
        }
        if (options & SLIDER_TICKS_BOTTOM)
        {
            hh -= TICKSIZE;
        }

        // Draw slider
        if (options & SLIDER_INSIDE_BAR)
        {
            // !!! Hack here !!!
            drawBorderRectangle(dc, xx, yy, ww, hh);

            dc.setForeground(slotColor);
            dc.setStipple(STIPPLE_GRAY);
            dc.setForeground(slotColor);
            dc.setBackground(baseColor);
            dc.setFillStyle(FILL_OPAQUESTIPPLED);
            dc.fillRectangle(xx + 2, yy + 2, ww - 4, hh - 4);
            dc.setFillStyle(FILL_SOLID);
            if (options & SLIDER_TICKS_TOP)
            {
                drawHorzTicks(dc, xx, border + padtop, ww, hh);
            }
            if (options & SLIDER_TICKS_BOTTOM)
            {
                drawHorzTicks(dc, xx, height - border - padbottom - TICKSIZE, ww, hh);
            }
            if (isEnabled())
            {
                drawSliderHead(dc, headpos, yy + 2, headsize, hh - 4);
            }
        }
        else
        {
            if (options & SLIDER_ARROW_UP)
            {
                ty = yy + hhs + (hh - slotsize - hhs) / 2;
            }
            else if (options & SLIDER_ARROW_DOWN)
            {
                ty = yy + (hh - slotsize - hhs) / 2;
            }
            else
            {
                ty = yy + (hh - slotsize) / 2;
            }

            // !!! Hack here !!!
            drawBorderRectangle(dc, xx, ty, ww, slotsize);

            dc.setForeground(slotColor);
            dc.fillRectangle(xx + 2, ty + 2, ww - 4, slotsize - 4);
            if (options & SLIDER_TICKS_TOP)
            {
                drawHorzTicks(dc, xx, border + padtop, ww, hh);
            }
            if (options & SLIDER_TICKS_BOTTOM)
            {
                drawHorzTicks(dc, xx, height - border - padbottom - TICKSIZE, ww, hh);
            }
            if (isEnabled())
            {
                drawSliderHead(dc, headpos, yy, headsize, hh);
            }
        }
    }
    return 1;
}


//
// Hack of FXOptionMenu
//

// Button with gradient effect and dark theme support


#define MENUGLYPH_WIDTH  10
#define MENUGLYPH_HEIGHT 5


// Handle repaint
long FXOptionMenu::onPaint(FXObject*, FXSelector, void* ptr)
{
    FXint tw = 0, th = 0, iw = MENUGLYPH_WIDTH, ih = MENUGLYPH_HEIGHT, tx, ty, ix, iy;
    FXEvent* ev = (FXEvent*)ptr;
    FXDCWindow dc(this, ev);

    // Gradient colors
    FXuint r = FXREDVAL(baseColor);
    FXuint g = FXGREENVAL(baseColor);
    FXuint b = FXBLUEVAL(baseColor);

    // !!! Hack here !!!
    FXColor topcolor;
    FXColor bottomcolor;

    // Dark theme
    if ((r + g + b) / 3 < DARK_COLOR_THRESHOLD)
    {
        topcolor = FXRGB(FXMIN(1.1 * r, 255), FXMIN(1.1 * g, 255), FXMIN(1.1 * b, 255));
        bottomcolor = FXRGB(0.7 * r, 0.7 * g, 0.7 * b);
    }
    else // Light theme
    {
        topcolor = FXRGB(FXMIN(1.1 * r, 255), FXMIN(1.1 * g, 255), FXMIN(1.1 * b, 255));
        bottomcolor = FXRGB(0.9 * r, 0.9 * g, 0.9 * b);
    }

    // Draw background and frame
    drawGradientRectangle(dc, topcolor, bottomcolor, 0, 0, width, height);
    drawFrame(dc, 0, 0, width, height);

    // Position text & icon
    if (!label.empty())
    {
        tw = labelWidth(label);
        th = labelHeight(label);
    }
    if (icon)
    {
        iw = icon->getWidth();
        ih = icon->getHeight();
    }

    just_x(tx, ix, tw, iw);
    just_y(ty, iy, th, ih);

    // Draw enabled state
    if (isEnabled())
    {
        if (icon)
        {
            dc.drawIcon(icon, ix, iy);
        }
        else
        {
            drawDoubleRaisedRectangle(dc, ix, iy, MENUGLYPH_WIDTH, MENUGLYPH_HEIGHT);
        }
        if (!label.empty())
        {
            dc.setFont(font);
            dc.setForeground(textColor);
            drawLabel(dc, label, hotoff, tx, ty, tw, th);
        }
        if (hasFocus())
        {
            dc.drawFocusRectangle(border + 1, border + 1, width - 2 * border - 2, height - 2 * border - 2);
        }
    }
    // Draw grayed-out state
    else
    {
        if (icon)
        {
            dc.drawIconSunken(icon, ix, iy);
        }
        else
        {
            drawDoubleRaisedRectangle(dc, ix, iy, MENUGLYPH_WIDTH, MENUGLYPH_HEIGHT);
        }
        if (!label.empty())
        {
            dc.setFont(font);
            dc.setForeground(hiliteColor);
            drawLabel(dc, label, hotoff, tx + 1, ty + 1, tw, th);
            dc.setForeground(shadowColor);
            drawLabel(dc, label, hotoff, tx, ty, tw, th);
        }
    }
    return 1;
}
