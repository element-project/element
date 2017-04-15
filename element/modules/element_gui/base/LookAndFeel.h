/*
    This file is part of the element modules for the JUCE Library
    Copyright (C) 2014  Kushview, LLC.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef EL_LOOK_AND_FEEL_E1_H
#define EL_LOOK_AND_FEEL_E1_H

enum ColourIds
{
    mainBackgroundColourId          = 0x2340000,
    treeviewHighlightColourId       = 0x2340002
};

class JUCE_API LookAndFeel_E1 : public LookAndFeel_V3
{
public:
    enum DefaultColorCodes
    {
        defaultBackgroundColor           = 0xff16191A,
        defaultTextColor                 = 0xffcccccc,
        defaultTextActiveColor           = 0xffe5e5e5,
        defaultTextBoldColor             = 0xffe4e4e4,
        defaultTextEntryBackgroundColor  = 0xff000000,
        defaultTextEntryForegroundColor  = 0xffe5e5e5,
        defaultTabColor                  = 0xff1a1a1a,
        defaultTabOnColor                = 0xff23252d
    };

    static const Colour elementBlue;
    static const Colour backgroundColor;
    static const Colour textColor;
    static const Colour textActiveColor;
    static const Colour textBoldColor;
    static const Colour highlightBackgroundColor;


    LookAndFeel_E1();
    virtual ~LookAndFeel_E1();

    virtual void drawButtonBackground (Graphics&, Button&, const Colour& backgroundColour,
                                       bool isMouseOverButton, bool isButtonDown) override;

    virtual void drawTableHeaderBackground (Graphics&, TableHeaderComponent&) override;

    virtual bool areLinesDrawnForTreeView (TreeView&) override;
    virtual void drawTreeviewPlusMinusBox (Graphics&, const Rectangle<float>& area, Colour backgroundColour, bool isOpen, bool isMouseOver) override;
    virtual int getTreeViewIndentSize (TreeView&) override;

    virtual void drawComboBox (Graphics& g, int width, int height, bool isButtonDown,
                               int buttonX, int buttonY, int buttonW, int buttonH, ComboBox& box) override;

    virtual void drawKeymapChangeButton (Graphics& g, int width, int height, Button& button, const String& keyDescription) override;

    // Menus
    virtual void drawPopupMenuBackground (Graphics& g, int width, int height) override;
    virtual void drawMenuBarBackground (Graphics&, int width, int height, bool isMouseOverBar, MenuBarComponent&) override;
    virtual void drawMenuBarItem (Graphics&, int width, int height, int itemIndex, const String& itemText,
                                  bool isMouseOverItem, bool isMenuOpen, bool isMouseOverBar, MenuBarComponent&) override;
    virtual void getIdealPopupMenuItemSize (const String &text, bool isSeparator, int standardMenuItemHeight,
                                            int& idealWidth, int& idealHeight) override;


    virtual int getTabButtonOverlap (int tabDepth) override;
    virtual int getTabButtonSpaceAroundImage() override;
    virtual void drawTabButton (TabBarButton&, Graphics&, bool isMouseOver, bool isMouseDown) override;


    virtual void drawStretchableLayoutResizerBar (Graphics&, int w, int h, bool isVerticalBar, bool isMouseOver, bool isMouseDragging) override;


    bool areScrollbarButtonsVisible() override;
    virtual void drawScrollbar (Graphics&, ScrollBar&, int x, int y, int width, int height, bool isScrollbarVertical,
                                int thumbStartPosition, int thumbSize, bool isMouseOver, bool isMouseDown) override;

    virtual void drawConcertinaPanelHeader (Graphics&, const Rectangle<int>& area, bool isMouseOver, bool isMouseDown,
                                            ConcertinaPanel&, Component&) override;

    static void createTabTextLayout (const TabBarButton& button, float length, float depth, Colour colour, TextLayout&);



private:
    Image backgroundTexture;
    Colour backgroundTextureBaseColour;
};



#endif /* EL_LOOK_AND_FEEL_E1_H */
