#ifndef FONTDIALOG_H
#define FONTDIALOG_H

#include "ComboBox.h"
#include "DialogBox.h"

class FontSelector;

// Font selection widget
class FXAPI FontSelector : public FXPacker
{
    FXDECLARE(FontSelector)
protected:
    FXTextField* family = NULL;
    FXList* familylist = NULL;
    FXTextField* weight = NULL;
    FXList* weightlist = NULL;
    FXTextField* style = NULL;
    FXList* stylelist = NULL;
    FXTextField* size = NULL;
    FXList* sizelist = NULL;
    ComboBox* charset = NULL;
    ComboBox* setwidth = NULL;
    ComboBox* pitch = NULL;
    FXCheckButton* scalable = NULL;
    FXCheckButton* allfonts = NULL;
    FXButton* accept = NULL;
    FXButton* cancel = NULL;
    FXLabel* preview = NULL;
    FXFont* previewfont = NULL;
    FXFontDesc selected;
protected:
    FontSelector()
    {
    }
    void listFontFaces();
    void listWeights();
    void listSlants();
    void listFontSizes();
    void previewFont();

private:
    FontSelector(const FontSelector&);
    FontSelector& operator=(const FontSelector&);

public:
    long onCmdFamily(FXObject*, FXSelector, void*);
    long onCmdWeight(FXObject*, FXSelector, void*);
    long onCmdStyle(FXObject*, FXSelector, void*);
    long onCmdStyleText(FXObject*, FXSelector, void*);
    long onCmdSize(FXObject*, FXSelector, void*);
    long onCmdSizeText(FXObject*, FXSelector, void*);
    long onCmdCharset(FXObject*, FXSelector, void*);
    long onUpdCharset(FXObject*, FXSelector, void*);
    long onCmdSetWidth(FXObject*, FXSelector, void*);
    long onUpdSetWidth(FXObject*, FXSelector, void*);
    long onCmdPitch(FXObject*, FXSelector, void*);
    long onUpdPitch(FXObject*, FXSelector, void*);
    long onCmdScalable(FXObject*, FXSelector, void*);
    long onUpdScalable(FXObject*, FXSelector, void*);
    long onCmdAllFonts(FXObject*, FXSelector, void*);
    long onUpdAllFonts(FXObject*, FXSelector, void*);
public:
    enum
    {
        ID_FAMILY=FXPacker::ID_LAST,
        ID_WEIGHT,
        ID_STYLE,
        ID_STYLE_TEXT,
        ID_SIZE,
        ID_SIZE_TEXT,
        ID_CHARSET,
        ID_SETWIDTH,
        ID_PITCH,
        ID_SCALABLE,
        ID_ALLFONTS,
        ID_LAST
    };
public:

    // Constructor
    FontSelector(FXComposite* p, FXObject* tgt = NULL, FXSelector sel = 0, FXuint opts = 0,
                 int x = 0, int y = 0, int w = 0, int h = 0);

    // Create server-side resources
    virtual void create();

    // Return a pointer to the "Accept" button
    FXButton* acceptButton() const
    {
        return accept;
    }

    // Return a pointer to the "Cancel" button
    FXButton* cancelButton() const
    {
        return cancel;
    }

    // Set font selection
    void setFontSelection(const FXFontDesc& fontdesc);

    // Get font selection
    void getFontSelection(FXFontDesc& fontdesc) const;

    // Save to a stream
    virtual void save(FXStream& store) const;

    // Load from a stream
    virtual void load(FXStream& store);

    // Destructor
    virtual ~FontSelector();
};



// Font selection dialog
class FXAPI FontDialog : public DialogBox
{
    FXDECLARE(FontDialog)
protected:
    FontSelector* fontbox = NULL;
protected:
    FontDialog()
    {
    }
private:
    FontDialog(const FontDialog&);
    FontDialog& operator=(const FontDialog&);

public:
    // Constructor
    FontDialog(FXWindow* owner, const FXString& name, FXuint opts = 0, int x = 0, int y = 0, int w = 750, int h = 480);

    // Save dialog to a stream
    virtual void save(FXStream& store) const;

    // Load dialog from a stream
    virtual void load(FXStream& store);

    // Set the current font selection
    void setFontSelection(const FXFontDesc& fontdesc);

    // Get the current font selection
    void getFontSelection(FXFontDesc& fontdesc) const;

    // Destructor
    virtual ~FontDialog();
};


#endif
