#ifndef ICONLIST_H
#define ICONLIST_H

#include "xfeutils.h"

class IconList;
class FileList;


// Icon List options
enum
{
    /* These options are already declared in FXIconList.h
    ICONLIST_EXTENDEDSELECT    = 0,                    // Extended selection mode
    ICONLIST_SINGLESELECT      = 0x00100000,           // At most one selected item
    ICONLIST_BROWSESELECT      = 0x00200000,           // Always exactly one selected item
    ICONLIST_MULTIPLESELECT    = 0x00300000,           // Multiple selection mode
    ICONLIST_AUTOSIZE          = 0x00400000,           // Automatically size item spacing
    ICONLIST_DETAILED          = 0,                    // List mode
    ICONLIST_MINI_ICONS        = 0x00800000,           // Mini Icon mode
    ICONLIST_BIG_ICONS         = 0x01000000,           // Big Icon mode
    ICONLIST_ROWS              = 0,                    // Row-wise mode
    ICONLIST_COLUMNS           = 0x02000000,           // Column-wise mode
    ICONLIST_NORMAL            = ICONLIST_EXTENDEDSELECT
    */
    ICONLIST_SEARCH            = 0x10000000,           // Icon list is a search list (must be same value as in FileList)
    ICONLIST_STANDARD          = 0x20000000,           // Icon list is a not a file list and not a search list
    ICONLIST_FILELIST          = 0x40000000,           // Icon list is a file list
};


// Icon item
class FXAPI IconItem : public FXObject
{
    FXDECLARE(IconItem)
    friend class IconList;
protected:
    FXString label;                                     // Text of item
    FXIcon* bigIcon = NULL;                             // Big icon of item
    FXIcon* miniIcon = NULL;                            // Mini icon of item
    FXIcon* bigIconBadge = NULL;                        // Big icon badge of item
    FXIcon* miniIconBadge = NULL;                       // Mini icon badge of item
    void* data = NULL;                                  // Item user data pointer
    FXuint state = 0;                                   // Item state flags
    FXApp* app = NULL;
private:
    IconItem(const IconItem&);
    IconItem& operator=(const IconItem&);

protected:
    IconItem()
    {
    }
    virtual void draw(IconList*, FXDC&, int, int, int, int) const;
    virtual int hitItem(const IconList*, int, int, int rw = 1, int rh = 1) const;

protected:
    virtual void drawBigIcon(const IconList*, FXDC&, int, int, int, int) const;
    virtual void drawMiniIcon(const IconList*, FXDC&, int, int, int, int) const;
    FXbool isOdd(int) const;
    virtual void drawDetails(IconList*, FXDC&, int, int, int, int) const;

public:
    enum
    {
        SELECTED         = 1,            // Selected
        FOCUS            = 2,            // Focus
        DISABLED         = 4,            // Disabled
        DRAGGABLE        = 8,            // Draggable
        BIGICONOWNED     = 16,           // Big icon owned by item
        MINIICONOWNED    = 32            // Mini icon owned by item
    };
public:

    // Construct new item with given text, icons, and user-data
    IconItem(FXApp* a, const FXString& text, FXIcon* bi = NULL, FXIcon* mi = NULL, void* ptr = NULL) :
        label(text), bigIcon(bi), miniIcon(mi), data(ptr), state(0)
    {
        app = a;
    }

    // Change item's text label
    virtual void setText(const FXString& txt);

    // Return item's text label
    const FXString& getText() const
    {
        return label;
    }

    // Change item's big icon and badge, deleting the old icon if it was owned
    virtual void setBigIcon(FXIcon* icn, FXbool owned = false, FXIcon* bdg = NULL);

    // Return item's big icon
    FXIcon* getBigIcon() const
    {
        return bigIcon;
    }

    // Change item's mini icon and badge, deleting the old icon if it was owned
    virtual void setMiniIcon(FXIcon* icn, FXbool owned = false, FXIcon* bdg = NULL);

    // Return item's mini icon
    FXIcon* getMiniIcon() const
    {
        return miniIcon;
    }

    // Change item's user data
    void setData(void* ptr)
    {
        data = ptr;
    }

    // Get item's user data
    void* getData() const
    {
        return data;
    }

    // Make item draw as focused
    virtual void setFocus(FXbool focus);

    // Return true if item has focus
    FXbool hasFocus() const
    {
        return (state & FOCUS) != 0;
    }

    // Select item
    virtual void setSelected(FXbool selected);

    // Return true if this item is selected
    FXbool isSelected() const
    {
        return (state & SELECTED) != 0;
    }

    // Enable or disable item
    virtual void setEnabled(FXbool enabled);

    // Return true if this item is enabled
    FXbool isEnabled() const
    {
        return (state & DISABLED) == 0;
    }

    // Make item draggable
    virtual void setDraggable(FXbool draggable);

    // Return true if this item is draggable
    FXbool isDraggable() const
    {
        return (state & DRAGGABLE) != 0;
    }

    // Return width of item as drawn in list
    virtual int getWidth(const IconList* list) const;

    // Return height of item as drawn in list
    virtual int getHeight(const IconList* list) const;

    // Create server-side resources
    virtual void create();

    // Detach server-side resources
    virtual void detach();

    // Destroy server-side resources
    virtual void destroy();

    // Save to stream
    virtual void save(FXStream& store) const;

    // Load from stream
    virtual void load(FXStream& store);

    // Destroy item and free icons if owned
    virtual ~IconItem();
};


// Icon item collate function
typedef int (*IconListSortFunc)(const IconItem*, const IconItem*);


// List of IconItem's
typedef FXObjectListOf<IconItem>   IconItemList;


// A Icon List Widget displays a list of items, each with a text and
// optional icon.  Icon List can display its items in essentially three
// different ways; in big-icon mode, the bigger of the two icons is used
// for each item, and the text is placed underneath the icon. In mini-
// icon mode, the icons are listed in rows and columns, with the smaller
// icon preceding the text.  Finally, in detail mode the icons are listed
// in a single column, and all fields of the text are shown under a
// header control with one button for each subfield.
// When an item's selected state changes, the icon list sends
// a SEL_SELECTED or SEL_DESELECTED message.  A change of the current
// item is signified by the SEL_CHANGED message.
// The icon list sends SEL_COMMAND messages when the user clicks on an item,
// and SEL_CLICKED, SEL_DOUBLECLICKED, and SEL_TRIPLECLICKED when the user
// clicks once, twice, or thrice, respectively.
// When items are added, replaced, or removed, the icon list sends messages
// of the type SEL_INSERTED, SEL_REPLACED, or SEL_DELETED.
// In each of these cases, the index to the item, if any, is passed in the
// 3rd argument of the message.

class FXAPI IconList : public FXScrollArea
{
    FXDECLARE(IconList)
protected:
    FXHeader* header = NULL;                        // Header control
    IconItemList items;                             // Item list
    int nrows = 0;                                  // Number of rows
    int ncols = 0;                                  // Number of columns
    int anchor = 0;                                 // Anchor item
    int current = 0;                                // Current item
    int extent = 0;                                 // Extent item
    int cursor = 0;                                 // Cursor item
    int viewable = 0;                               // Visible item
    FXFont* font = NULL;                            // Font
    IconListSortFunc sortfunc = NULL;               // Item sort function
    FXColor textColor = FXRGB(0, 0, 0);             // Text color
    FXColor selbackColor = FXRGB(0, 0, 0);          // Selected back color
    FXColor seltextColor = FXRGB(0, 0, 0);          // Selected text color
    FXColor highlightColor = FXRGB(0, 0, 0);        // Highlight color
    FXColor sortColor = FXRGB(0, 0, 0);             // Sort color
    FXColor highlightSortColor = FXRGB(0, 0, 0);    // Highlight sort color
    int itemWidth = 0;                              // Item width
    int itemHeight = 0;                             // Item height
    int itemSpace = 0;                              // Space for item label
    int anchorx = 0;                                // Rectangular selection
    int anchory = 0;
    int currentx = 0;
    int currenty = 0;
    int grabx = 0;                                  // Grab point x
    int graby = 0;                                  // Grab point y
    FXString lookup;                                // Lookup string
    FXString help;                                  // Help text
    FXbool state = false;                           // State of item
    FXbool allowTooltip = false;                    // Allow tooltip in single click mode
    FXuint numsortheader = 0;                       // Index of the sorted column
    double headerpct[10];                           // Header sizes, relatively to the list width (in percent)
    int count = 0;                                  // Counter used to properly initialize the relative header sizes
    FXbool ignorecase = false;                      // Case sensitivity for file name sorting
    FXbool initheaderpct = false;                   // Indicates we have to initialize the headerpct for the deletion columns
    double scalefrac = 1.0;                         // Icon scaling factor

    FXuint idCol[NMAX_COLS + 1] = { 0 };
    FXuint nbCols = 0;

    FXuint idColTrash[NMAX_COLS + 2] = { 0 };
    FXuint nbColsTrash = 0;

    FXbool inTrash = false;

    FXuint single_click = SINGLE_CLICK_NONE;        // Single click navigation
    FXbool file_tooltips = true;                    // File tooltips
    FXbool relative_resize = true;                  // Relative resizing of the panels and columns in detailed mode

protected:
    IconList()
    {
    }
    void recompute();
    void getrowscols(int&, int&, int, int) const;
    void drawLasso(int, int, int, int);
    void lassoChanged(int, int, int, int, int, int, int, int, FXbool);
    virtual void moveContents(int, int);
    virtual IconItem* createItem(const FXString&, FXIcon*, FXIcon*, void*);
    static int compareSection(const char*, const char*, int);
    static int compareSectionCase(const char*, const char*, int);

private:
    IconList(const IconList&);
    IconList& operator=(const IconList&);

public:
    long onConfigure(FXObject*, FXSelector, void*);
    long onPaint(FXObject*, FXSelector, void*);
    long onEnter(FXObject*, FXSelector, void*);
    long onLeave(FXObject*, FXSelector, void*);
    long onUngrabbed(FXObject*, FXSelector, void*);
    long onKeyPress(FXObject*, FXSelector, void*);
    long onKeyRelease(FXObject*, FXSelector, void*);
    long onLeftBtnPress(FXObject*, FXSelector, void*);
    long onLeftBtnRelease(FXObject*, FXSelector, void*);
    long onRightBtnPress(FXObject*, FXSelector, void*);
    long onRightBtnRelease(FXObject*, FXSelector, void*);
    long onMotion(FXObject*, FXSelector, void*);
    long onQueryTip(FXObject*, FXSelector, void*);
    long onQueryHelp(FXObject*, FXSelector, void*);
    long onTipTimer(FXObject*, FXSelector, void*);
    long onCmdselectAll(FXObject*, FXSelector, void*);
    long onCmdDeselectAll(FXObject*, FXSelector, void*);
    long onCmdSelectInverse(FXObject*, FXSelector, void*);
    long onCmdArrangeByRows(FXObject*, FXSelector, void*);
    long onUpdArrangeByRows(FXObject*, FXSelector, void*);
    long onCmdArrangeByColumns(FXObject*, FXSelector, void*);
    long onUpdArrangeByColumns(FXObject*, FXSelector, void*);
    long onCmdShowDetails(FXObject*, FXSelector, void*);
    long onUpdShowDetails(FXObject*, FXSelector, void*);
    long onCmdShowBigIcons(FXObject*, FXSelector, void*);
    long onUpdShowBigIcons(FXObject*, FXSelector, void*);
    long onCmdShowMiniIcons(FXObject*, FXSelector, void*);
    long onUpdShowMiniIcons(FXObject*, FXSelector, void*);
    long onHeaderChanged(FXObject*, FXSelector, void*);
    long onHeaderResize(FXObject*, FXSelector, void*);
    long onFocusIn(FXObject*, FXSelector, void*);
    long onFocusOut(FXObject*, FXSelector, void*);
    long onClicked(FXObject*, FXSelector, void*);
    long onDoubleClicked(FXObject*, FXSelector, void*);
    long onTripleClicked(FXObject*, FXSelector, void*);
    long onCommand(FXObject*, FXSelector, void*);
    long onAutoScroll(FXObject*, FXSelector, void*);
    long onLookupTimer(FXObject*, FXSelector, void*);
    long onCmdSetValue(FXObject*, FXSelector, void*);
    long onCmdGetIntValue(FXObject*, FXSelector, void*);
    long onCmdSetIntValue(FXObject*, FXSelector, void*);
    long onCmdToggleAutosize(FXObject*, FXSelector, void*);
    long onUpdToggleAutosize(FXObject*, FXSelector, void*);
    long onCmdHeaderClicked(FXObject*, FXSelector, void*);
    long onCmdReturnKeyPressed(FXObject*, FXSelector, void*);

public:
    static int ascending(const IconItem*, const IconItem*);
    static int descending(const IconItem*, const IconItem*);
    static int ascendingCase(const IconItem*, const IconItem*);
    static int descendingCase(const IconItem*, const IconItem*);

public:
    enum
    {
        ID_SHOW_DETAILS=FXScrollArea::ID_LAST,
        ID_SHOW_MINI_ICONS,
        ID_SHOW_BIG_ICONS,
        ID_ARRANGE_BY_ROWS,
        ID_ARRANGE_BY_COLUMNS,
        ID_HEADER_CHANGE,
        ID_LOOKUPTIMER,
        ID_SELECT_ALL,
        ID_DESELECT_ALL,
        ID_SELECT_INVERSE,
        ID_AUTOSIZE,
        ID_LAST
    };
public:

    // Construct icon list with no items in it initially
    IconList(FXComposite* p, FXuint* ic, FXuint nc, FXObject* tgt = NULL, FXSelector sel = 0,
             FXuint opts = ICONLIST_NORMAL, int x = 0, int y = 0, int w = 0, int h = 0);

    // Create server-side resources
    virtual void create();

    // Detach server-side resources
    virtual void detach();

    // Recalculate layout
    virtual void recalc();

    // Perform layout
    virtual void layout();

    // Compute and return content width
    virtual int getContentWidth();

    // Return content height
    virtual int getContentHeight();

    // Icon list can receive focus
    virtual bool canFocus() const;

    // Move the focus to this window
    virtual void setFocus();

    // Remove the focus from this window
    virtual void killFocus();

    // Return viewport size
    virtual int getViewportHeight();

    // Resize this window to the specified width and height
    virtual void resize(int w, int h);

    // Move and resize this window in the parent's coordinates
    virtual void position(int x, int y, int w, int h);

    // Return ignore case flag
    FXbool getIgnoreCase() const
    {
        return ignorecase;
    }

    // Set ignore case flag
    void setIgnoreCase(const FXbool);

    // Return number of items
    int getNumItems() const
    {
        return items.no();
    }

    // Return number of rows
    int getNumRows() const
    {
        return nrows;
    }

    // Return number of columns
    int getNumCols() const
    {
        return ncols;
    }

    // Return header control
    FXHeader* getHeader() const
    {
        return header;
    }

    // Autosize name header in detailed mode
    void autosizeNameHeader(void);

    // Set headers from array of strings
    void setHeaders(const char** strings, int size = 1);

    // Set headers from newline separated strings
    void setHeaders(const FXString& strings, int size = 1);

    // Append header with given text and optional icon
    void appendHeader(const FXString& text, FXIcon* icon = NULL, int size = 1);

    // Remove header at index
    void removeHeader(int index);

    // Change text of header at index
    void setHeaderText(int index, const FXString& text);

    // Return text of header at index
    FXString getHeaderText(int index) const;

    // Change icon of header at index
    void setHeaderIcon(int index, FXIcon* icon);

    // Return icon of header at index
    FXIcon* getHeaderIcon(int index) const;

    // Change size of header at index
    void setHeaderSize(int index, int size);

    // Return width of header at index
    int getHeaderSize(int index) const;

    // Return number of headers
    int getNumHeaders() const;

    // Return the item at the given index
    IconItem* getItem(int index) const;

    // Replace the item with a [possibly subclassed] item
    int setItem(int index, IconItem* item, FXbool notify = false);

    // Replace items text, icons, and user-data pointer
    int setItem(int index, const FXString& text, FXIcon* big = NULL, FXIcon* mini = NULL, void* ptr = NULL,
                FXbool notify = false);

    // Fill list by appending items from array of strings
    int fillItems(const char** strings, FXIcon* big = NULL, FXIcon* mini = NULL, void* ptr = NULL,
                  FXbool notify = false);

    // Fill list by appending items from newline separated strings
    int fillItems(const FXString& strings, FXIcon* big = NULL, FXIcon* mini = NULL, void* ptr = NULL,
                  FXbool notify = false);

    // Insert a new [possibly subclassed] item at the give index
    int insertItem(int index, IconItem* item, FXbool notify = false);

    // Insert item at index with given text, icons, and user-data pointer
    int insertItem(int index, const FXString& text, FXIcon* big = NULL, FXIcon* mini = NULL, void* ptr = NULL,
                   FXbool notify = false);

    // Append a [possibly subclassed] item to the end of the list
    int appendItem(IconItem* item, FXbool notify = false);

    // Append new item with given text and optional icons, and user-data pointer
    int appendItem(const FXString& text, FXIcon* big = NULL, FXIcon* mini = NULL, void* ptr = NULL,
                   FXbool notify = false);

    // Prepend a [possibly subclassed] item to the end of the list
    int prependItem(IconItem* item, FXbool notify = false);

    // Prepend new item with given text and optional icons, and user-data pointer
    int prependItem(const FXString& text, FXIcon* big = NULL, FXIcon* mini = NULL, void* ptr = NULL,
                    FXbool notify = false);

    // Move item from oldindex to newindex
    int moveItem(int newindex, int oldindex, FXbool notify = false);

    // Extract item from list
    IconItem* extractItem(int index, FXbool notify = false);

    // Remove item from list
    void removeItem(int index, FXbool notify = false);

    // Remove all items from list
    void clearItems(FXbool notify = false);

    // Return item width
    int getItemWidth() const
    {
        return itemWidth;
    }

    // Return item height
    int getItemHeight() const
    {
        return itemHeight;
    }

    // Return index of item at x,y, or -1 if none
    virtual int getItemAt(int x, int y, int state = -1);

    // Search items by name, beginning from item start.  If the start
    // item is -1 the search will start at the first item in the list.
    // Flags may be SEARCH_FORWARD or SEARCH_BACKWARD to control the
    // search direction; this can be combined with SEARCH_NOWRAP or SEARCH_WRAP
    // to control whether the search wraps at the start or end of the list.
    // The option SEARCH_IGNORECASE causes a case-insensitive match.  Finally,
    // passing SEARCH_PREFIX causes searching for a prefix of the item name.
    // Return -1 if no matching item is found.
    int findItem(const FXString& text, int start = -1, FXuint flags = SEARCH_FORWARD | SEARCH_WRAP) const;


    // Search items by associated user data, beginning from item start. If the
    // start item is -1 the search will start at the first item in the list.
    // Flags may be SEARCH_FORWARD or SEARCH_BACKWARD to control the
    // search direction; this can be combined with SEARCH_NOWRAP or SEARCH_WRAP
    // to control whether the search wraps at the start or end of the list.
    int findItemByData(const void* ptr, int start = -1, FXuint flags = SEARCH_FORWARD | SEARCH_WRAP) const;

    // Scroll to make item at index visible
    virtual void makeItemVisible(int index);

    // Change item text
    void setItemText(int index, const FXString& text);

    // Return item text
    FXString getItemText(int index) const;

    // Change item big icon
    void setItemBigIcon(int index, FXIcon* icon, FXbool owned = false);

    // Return big icon of item at index
    FXIcon* getItemBigIcon(int index) const;

    // Change item mini icon
    void setItemMiniIcon(int index, FXIcon* icon, FXbool owned = false);

    // Return mini icon of item at index
    FXIcon* getItemMiniIcon(int index) const;

    // Change item user-data pointer
    void setItemData(int index, void* ptr);

    // Return item user-data pointer
    void* getItemData(int index) const;

    // Return true if item is selected
    FXbool isItemSelected(int index) const
    {
        if ((FXuint)index >= (FXuint)items.no())
        {
            fxerror("%s::isItemSelected: index out of range.\n", getClassName());
        }
        return items[index]->isSelected();
    }

    // Return true if item at index is current
    FXbool isItemCurrent(int index) const;

    // Return true if item at index is visible
    FXbool isItemVisible(int index) const;

    // Return true if item at index is enabled
    FXbool isItemEnabled(int index) const;

    // Return item hit code: 0 outside, 1 icon, 2 text
    int hitItem(int index, int x, int y, int ww = 1, int hh = 1) const;

    // Repaint item at index
    void updateItem(int index) const;

    // Enable item at index
    virtual FXbool enableItem(int index);

    // Disable item at index
    virtual FXbool disableItem(int index);

    // Select item at index
    virtual FXbool selectItem(int index, FXbool notify = false);

    // Deselect item at index
    virtual FXbool deselectItem(int index, FXbool notify = false);

    // Toggle item at index
    virtual FXbool toggleItem(int index, FXbool notify = false);

    // Select items in rectangle
    virtual FXbool selectInRectangle(int x, int y, int w, int h, FXbool notify = false);

    // Extend selection from anchor index to index
    virtual FXbool extendSelection(int index, FXbool notify = false);

    // Deselect all items
    virtual FXbool killSelection(FXbool notify = false);

    // Change current item index
    virtual void setCurrentItem(int index, FXbool notify = false);

    // Return current item index, or -1 if none
    int getCurrentItem() const
    {
        return current;
    }

    // Change anchor item index
    void setAnchorItem(int index);

    // Return anchor item index, or -1 if none
    int getAnchorItem() const
    {
        return anchor;
    }

    // Return index of item under cursor, or -1 if none
    int getCursorItem() const
    {
        return cursor;
    }

    // Sort items
    void sortItems();

    // Return sort function
    IconListSortFunc getSortFunc() const
    {
        return sortfunc;
    }

    // Change sort function
    void setSortFunc(IconListSortFunc func)
    {
        sortfunc = func;
    }

    // Set sort header
    void setSortHeader(const FXuint num)
    {
        numsortheader = num;
    }

    // Get sort header
    FXuint getSortHeader()
    {
        return numsortheader;
    }

    // Change text font
    void setFont(FXFont* fnt);

    // Return text font
    FXFont* getFont() const
    {
        return font;
    }

    // Return normal text color
    FXColor getTextColor() const
    {
        return textColor;
    }

    // Change normal text color
    void setTextColor(FXColor clr);

    // Return selected text background
    FXColor getSelBackColor() const
    {
        return selbackColor;
    }

    // Change selected text background
    void setSelBackColor(FXColor clr);

    // Return selected text color
    FXColor getSelTextColor() const
    {
        return seltextColor;
    }

    // Return highlight color
    FXColor getHighlightColor() const
    {
        return highlightColor;
    }

    // Return sort color
    FXColor getSortColor() const
    {
        return sortColor;
    }

    // Return highlight sort color
    FXColor getHighlightSortColor() const
    {
        return highlightSortColor;
    }

    // Change selected text color
    void setSelTextColor(FXColor clr);

    // Change maximum item space for each item
    void setItemSpace(int s);

    // Return maximum item space
    int getItemSpace() const
    {
        return itemSpace;
    }

    // Get the current icon list style
    FXuint getListStyle() const;

    // Set the current icon list style
    void setListStyle(FXuint style);

    // Get the current icon list type
    FXuint getListType() const;

    // Set the status line help text for this widget
    void setHelpText(const FXString& text);

    // Get the status line help text for this widget
    const FXString& getHelpText() const
    {
        return help;
    }

    // Get column id from header position
    FXString getHeaderName(FXuint);

    // Get header name from column id
    FXuint getHeaderId(int);

    // Save list to a stream
    virtual void save(FXStream& store) const;

    // Load list from a stream
    virtual void load(FXStream& store);

    // Destructor
    virtual ~IconList();
};

#endif
