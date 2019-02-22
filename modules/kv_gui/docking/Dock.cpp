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

void Dock::dumpObjects()
{
    for (auto* area : areas)
    {
        DBG("AREA: " << area->getNumChildComponents());
    }
    
    for (auto* item : items)
    {
        DBG("ITEM: " << item->getNumPanels());
    }

    for (auto* panel : panels)
    {
        DBG("PANEL: " << panel->getName() << ": " << (int) (panel->getParentComponent() == nullptr));
    }
}

Dock::Dock ()
{
    container.reset (new DockContainer (*this));
    addAndMakeVisible (container.get());
}

Dock::~Dock()
{
    container.reset (nullptr);
}

void Dock::registerPanelType (DockPanelType* newType)
{
    jassert (newType != nullptr);
    jassert (! types.contains (newType));
    types.add (newType);
    newType->getAllTypes (available);
}

DockArea* Dock::getOrCreateArea (const bool isVertical, DockArea* areaToSkip)
{
    DockArea* retArea = nullptr;
    for (auto* const area : areas)
        if (area->getNumItems() <= 0 && area->getParentComponent() == nullptr)
            { retArea = area; break; }
    if (! retArea || (areaToSkip != nullptr && retArea == areaToSkip))
        retArea = areas.add (new DockArea ());

    if (retArea)
    {
        retArea->setVertical (isVertical);
    }

    return retArea;
}

DockItem* Dock::getOrCreateItem (DockPanel* const panel)
{
    DockItem* dockItem = nullptr;
    for (auto* const item : items)
        if (item->getNumPanels() <= 0 && item->getParentComponent() == nullptr)
            { dockItem = item; break; }
    if (! dockItem)
        dockItem = items.add (new DockItem (*this));
    
    if (dockItem != nullptr)
    {
        dockItem->reset();
        if (panel != nullptr)
        {
            dockItem->panels.add (panel);
            dockItem->refreshPanelContainer();
        }
    }

    return dockItem;
}

DockPanel* Dock::getOrCreatePanel (const String& panelType)
{
    DockPanelInfo* panelInfo = nullptr;
    for (auto* const info : available)
    {
        if (info->identifier == Identifier (panelType))
            panelInfo = info;
        if (panelInfo != nullptr)
            break;
    }

    if (! panelInfo) return nullptr;

    DockPanel* panel = nullptr;
    for (auto* type : types)
    {
        panel = type->createPanel (*panelInfo);
        if (panel != nullptr) break;
    }

    if (panel != nullptr)
    {
        if (panel->getName().isEmpty())
            panel->setName (panelInfo->name);
        if (panel->getName().isEmpty())
            { jassertfalse; } // you might want to give your panel and panel type a name

        panel->identifier = panelType;

        panels.add (panel);        
        if (onPanelAdded)
            onPanelAdded (panel);
    }

    return panel;
}

void Dock::resized()
{
    container->setBounds (getLocalBounds());
}

void Dock::dragOperationStarted (const DragAndDropTarget::SourceDetails& details)
{
//    if (auto* panel = dynamic_cast<DockPanel*> (details.sourceComponent.get()))
//        if (auto* item = dynamic_cast<DockItem*> (panel->findParentComponentOfClass<DockItem>()))
//            item->setMouseCursor (MouseCursor::CopyingCursor);
}

void Dock::dragOperationEnded (const DragAndDropTarget::SourceDetails& details)
{
//    if (auto* panel = dynamic_cast<DockPanel*> (details.sourceComponent.get()))
//        if (auto* item = dynamic_cast<DockItem*> (panel->findParentComponentOfClass<DockItem>()))
//            item->setMouseCursor (MouseCursor::NormalCursor);
}

DockItem* Dock::createItem (const String& panelType, DockPlacement placement)
{
    auto* panel = getOrCreatePanel (panelType);
    
    if (! panel)
    {
        jassertfalse;
        return nullptr;
    }
    
    if (placement.isFloating())
    {
        auto* window = windows.add (new DockWindow (*this));
        auto* item = getOrCreateItem (panel);
        window->setBackgroundColour (findColour (DocumentWindow::backgroundColourId).darker());
        window->centreWithSize (window->getWidth(), window->getHeight());
        window->dockItem (item, DockPlacement::Top);
        window->setVisible (true);
        window->addToDesktop();
        window->toFront (true);
        return item;
    }
    
    if (! placement.isDirectional())
        return nullptr;
    
    auto* item = getOrCreateItem (panel);
    if (item && ! container->dockItem (item, placement))
    {
        item->reset();
        item = nullptr;
    }
    
    resized();
    return item;
}

void Dock::removeOrphanObjects()
{
    const int sizeBefore = panels.size() + items.size() + areas.size();
   #if 0
    for (int i = panels.size(); --i >= 0;)
        if (panels.getUnchecked(i)->getParentComponent() == nullptr)
            panels.remove (i);
    for (int i = items.size(); --i >= 0;)
        if (items.getUnchecked(i)->getNumPanels() <= 0 && items.getUnchecked(i)->getParentComponent() == nullptr)
            items.remove (i);
    for (int i = areas.size(); --i >= 0;)
        if (areas.getUnchecked(i)->getNumItems() <= 0 && items.getUnchecked(i)->getParentComponent() == nullptr)
            areas.remove (i);
    const int sizeAfter = panels.size() + items.size() + areas.size();
    DBG("Dock: purged " << (sizeBefore - sizeAfter) << " orphan objects.");
   #else
    DBG("Dock: not purging orphans.");
   #endif
}

void Dock::startDragging (DockPanel* const panel)
{
    jassert (panel != nullptr);
    Image image (Image::ARGB, 1, 1, true);
    DragAndDropContainer::startDragging ("DockPanel", panel, image, true);
}

ValueTree Dock::getState() const
{
    ValueTree state (Slugs::dock);
    state.setProperty (Slugs::bounds, getLocalBounds().toString(), nullptr);
    state.addChild (container->root->getState(), -1, nullptr);
    return state;
}

void Dock::loadArea (DockArea& area, const ValueTree& state)
{
    const auto sizes = state.getProperty (Slugs::sizes).toString();
    const auto barSize = (int) state.getProperty (Slugs::barSize, 4);
    area.setBounds (Dock::getBounds (state));
    area.layout.clear();
    area.layout.setBarSize (barSize);
    
    if (sizes.isNotEmpty())
        area.layout.setSizes (sizes);

    for (int i = 0; i < state.getNumChildren(); ++i)
    {
        const auto child = state.getChild (i);        
        if (child.hasType (Slugs::item))
        {
            auto* item = getOrCreateItem();
            loadItem (*item, child);
            area.append (item);
        }
        else if (child.hasType (Slugs::area))
        {
            jassert (area.isVertical() != (bool) child[Slugs::vertical]);
            auto* newArea = getOrCreateArea (! area.isVertical(), &area);
            jassert (newArea != &area);
            loadArea (*newArea, child);
            area.append (newArea);
        }
    }

    if (sizes.isNotEmpty())
        area.layout.setSizes (sizes);

    area.resized();
}

void Dock::loadItem (DockItem& item, const ValueTree& state)
{
    item.reset();
    item.setBounds (Dock::getBounds (state));
    for (int i = 0; i < state.getNumChildren(); ++i)
    {
        const auto child = state.getChild (i);
        
        if (child.hasType (Slugs::panel))
        {
            if (auto* panel = getOrCreatePanel (child["type"].toString()))
            {
                loadPanel (*panel, child);
                item.panels.addIfNotAlreadyThere (panel);
            }
            else
            {
                // couldn't create panel
                jassertfalse;
            }
        }
        else
        {
            DBG(child.toXmlString());
            jassertfalse; // not a panel?
        }
    }

    item.refreshPanelContainer();
    item.setCurrentPanelIndex ((int) state.getProperty (Slugs::panel, 0));
}

void Dock::loadPanel (DockPanel& panel, const ValueTree& state)
{
    panel.setName (state.getProperty (Slugs::name, "Panel").toString());
    panel.setBounds (Dock::getBounds (state));
}

bool Dock::applyState (const ValueTree& state)
{
    if (! state.hasType (Slugs::dock))
        return false;

    std::unique_ptr<DockContainer> newContainer;
    
    for (int i = 0; i < state.getNumChildren(); ++i)
    {
        const auto child = state.getChild (i);
        if (child.hasType (Slugs::area))
        {
            if (newContainer == nullptr)
            {
                newContainer.reset (new DockContainer (*this));
                newContainer->setBounds (Dock::getBounds (state));
                newContainer->resized();
                auto* area (newContainer->getRootArea());
                jassert(area);
                area->setVertical ((bool) child.getProperty (Slugs::vertical, true));
                area->setBounds (Dock::getBounds (child));
                loadArea (*area, child);
            }
            else
            {
                // root state should only have one area child
                jassertfalse;
            }
        }
        else if (child.hasType ("window"))
        {
            DBG("load dock window");
        }
    }

    if (newContainer)
    {
        removeChildComponent (container.get());
        container.swap (newContainer);
        addAndMakeVisible (container.get());
        resized();
        removeOrphanObjects();
        return true;
    }

    return false;
}

}
