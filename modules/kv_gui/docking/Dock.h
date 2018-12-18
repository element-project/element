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

#pragma once

namespace kv {

class DockArea;
class DockItem;
class DockLayout;

class Dock : public Component
{
public:
    enum Placement {
        TopArea,
        BottomArea,
        LeftArea,
        RightArea,
        Center,
        Floating
    };

    Dock();
    virtual ~Dock();

    //Component* mainComponent();
    //void setMainComponent (Component* comp);

    DockArea& getTopArea()    { return *areas [Dock::TopArea]; }
    DockArea& getBottomArea() { return *areas [Dock::BottomArea]; }
    DockArea& getLeftArea()   { return *areas [Dock::LeftArea]; }
    DockArea& getRightArea()  { return *areas [Dock::RightArea]; }

    DockItem* createItem (const String& id, const String& name, Dock::Placement placement);
    DockItem* getItem    (const String& id);

    void paint (Graphics&) override { }

    void resized() override;

private:
    void detatchAll (DockItem* item);

    DockItem* maximizedItem;
    OwnedArray<DockArea>      areas;
    OwnedArray<DockItem>      items;

    class MiddleLayout;
    ScopedPointer<MiddleLayout> middleLayout;

    StretchableLayoutManager areaLayout;
    StretchableLayoutResizerBar leftbar, rightbar;

    friend class DockItem;
};

class DockLayout
{
public:
    DockLayout (Component& holder_, bool vertical = false);

    ~DockLayout ();

    void append (DockItem* child);

    void remove (DockItem* const child);

    void layoutItems (int x, int y, int w, int h);

    void layoutItems();

    void setIsVertical (bool vertical) { isVertical = vertical; }

    bool getIsVertical() const { return isVertical; }

    DockItem* root();

private:

    void buildComponentArray();

    bool isVertical;
    Component& holder;
    Array<DockItem*> items;
    OwnedArray<StretchableLayoutResizerBar> bars;
    Array<Component*> comps;
    StretchableLayoutManager layout;
};

class DockArea : public Component
{
public:
    DockArea();
    DockArea(Dock::Placement placement);
    ~DockArea();

    void append (DockItem* const item);
    void detachItem (DockItem* item);

    void paint(Graphics&) { }
    void resized();
private:

    void disposeEmptyLayouts();

    OwnedArray<DockLayout> layouts;
};


class DockItem : public Component,
                 public DragAndDropTarget
{
public:

    DockItem (Dock& parent, const String& slug, const String& name);
    virtual ~DockItem();

    void append (const String& itemID);

    void dockTo (DockItem* target, Dock::Placement placement);

    bool isToplevel() { return nullptr == getDockArea(); }
    DockArea* getDockArea() { return dynamic_cast<DockArea*> (getParentComponent()); }

    void layoutItems();

    /* Component */
    void paint (Graphics& g);

    void setContentOwned (Component* component)
    {
        if (content != nullptr)
            removeChildComponent (content);

        content = component;
        addAndMakeVisible (content);
    }

    void resized()
    {
        if (isMaximized())
        {
            content->setBounds (getLocalBounds());
        }
        else
        {
            overlay.centreWithSize (getWidth() - 2, getHeight() - 2);

            if (content != nullptr)
                content->setBounds (36, 0, getWidth() - 36, getHeight());
        }
    }

    void mouseDown (const MouseEvent& ev);

    /* Drag and drop */

    bool isInterestedInDragSource (const SourceDetails& dragSourceDetails)
    {
        if (dragSourceDetails.description.toString() == "dock-item")
            return true;
        return false;
    }

    void itemDropped (const SourceDetails& dragSourceDetails)
    {
        Component* comp = dragSourceDetails.sourceComponent.get();
        DockItem* item = (DockItem*) comp;

        overlay.setVisible (false);

        if (item == this)
            return;

        item->dockTo (this, Dock::RightArea);

    }

    virtual void itemDragEnter (const SourceDetails&)
    {
        overlay.toFront (false);
        overlay.setVisible (true);
    }

    virtual void itemDragExit (const SourceDetails&)
    {
        overlay.setVisible (false);
    }

    bool isMaximized() const {
        return dock.maximizedItem == this;
    }

    void setMaximized (const bool shouldBeMaximized)
    {
        if (shouldBeMaximized == isMaximized())
            return;

        if (shouldBeMaximized) {
            dock.maximizedItem = this;
        } else {
            dock.maximizedItem = nullptr;
        }

        dock.resized();
    }

public:
    Dock&      dock;
    DockLayout layout;

    friend class DockArea;
    friend class Dock;

    bool dragging;

    ScopedPointer<Component> content;

    void detach()
    {
        if (DockArea* area = getDockArea())
        {
            area->detachItem (this);
            area->resized();
        }
    }

    class DragOverlay : public Component
    {
    public:
        DragOverlay() { }
        void paint (Graphics &g)
        {
            g.setOpacity (.50);
            g.fillAll (Colours::teal);

            g.setColour (Colours::black);
            g.drawRect (0, 0, getWidth(), getHeight(),2);
            g.drawRect (30, 30, getWidth() - 60, getHeight() - 60);
        }

    private:

    } overlay;

    class Grip  : public Component,
                  public DragAndDropContainer
    {
    public:
        Grip (Component& parent_) : parent(parent_)
        {
           #if JUCE_IOS
            setSize (36, 48);
           #else
            setSize (12, 16);
           #endif
        }

        void paint (Graphics& g)
        {
            g.setColour(Colours::white);

            const int pad = 1;
            const int size = 1;
            int x = 4, y = 0;

            for (int i = 0 ; i < getHeight() / (pad + size); ++i)
            {
                y = i * (pad + size);
                g.fillEllipse ((float)x, (float)y + 3.0f, (float)size, (float)size);
                g.fillEllipse ((float)x + 2.0f, (float)y + 3.0f, (float)size, (float)size);
            }
        }

        void mouseDown (const MouseEvent&)
        {
            parent.setAlpha (0.9);
            startDragging ("dock-item" ,&parent, Image(), true);
        }

        void mouseDrag (const MouseEvent&)
        {
        }

        void mouseUp (const MouseEvent&)
        {
            parent.setAlpha (1);
        }

    private:
        ComponentDragger dragger;
        Component& parent;

    } grip;
};

}
