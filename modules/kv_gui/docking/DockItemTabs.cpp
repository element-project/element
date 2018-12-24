/*
    This file is part of the Kushview Modules for JUCE
    Copyright (C) 2014-2018  Kushview, LLC.  All rights reserved.

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

namespace kv {

class DockTabBarButton : public TabBarButton
{
public:
    DockTabBarButton (const String& tabName, TabbedButtonBar& bar)
        : TabBarButton (tabName, bar)
    {
        setTriggeredOnMouseDown (true);
    }
    
    ~DockTabBarButton() { }
    
    Dock* getDock() { return findParentComponentOfClass<Dock>(); }
    
    DockPanel* getDockPanel()
    {
        if (auto* const tabs = findParentComponentOfClass<DockItemTabs>())
            return dynamic_cast<DockPanel*> (tabs->getTabContentComponent (getIndex()));
        return nullptr;
    }
    
    void mouseDown (const MouseEvent& event) override
    {
        TabBarButton::mouseDown (event);
    }
    
    void mouseDrag (const MouseEvent& event) override
    {
        if (! dragging && (event.y < 0 || event.y > getHeight()))
        {
            dragging = true;
            auto* dock = getDock();
            auto* panel = getDockPanel();
            if (dock && panel)
                dock->startDragging (panel);
        }
    }
    
    void mouseExit (const MouseEvent& event) override
    {
        TabBarButton::mouseExit (event);
    }
    
    void mouseUp (const MouseEvent& event) override
    {
        dragging = moving = false;
    }
    
private:
    bool dragging = false;
    bool moving = false;
};

static void initDockTabs (DockItemTabs& tabs, TabbedButtonBar& bar)
{
    tabs.setTabBarDepth (20);
    tabs.setOutline (0);
    tabs.setIndent (0);
    
    bar.setMinimumTabScaleFactor (0.7);
}

DockItemTabs::DockItemTabs()
    : TabbedComponent (TabbedButtonBar::TabsAtTop)
{
    initDockTabs (*this, *tabs);
}

DockItemTabs::DockItemTabs (TabbedButtonBar::Orientation orientation)
    : TabbedComponent (orientation)
{
    initDockTabs (*this, *tabs);
}

DockItemTabs::~DockItemTabs() { }

TabBarButton* DockItemTabs::createTabButton (const String &tabName, int tabIndex)
{
    ignoreUnused (tabIndex);
    return new DockTabBarButton (tabName, *tabs);
}

}
