/*
    DockArea.cpp - This file is part of Element
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

namespace kv {

DockArea::DockArea()
    : layout (*this)
{ 
    layout.setIsVertical (false);
}

DockArea::DockArea (const bool vertical)
    : layout (*this, vertical)
{
}

DockArea::DockArea (Dock::Placement placement)
    : layout(*this)
{
    switch (placement)
    {
        case Dock::TopPlacement:
        case Dock::BottomPlacement:
            layout.setIsVertical (true);
        case Dock::LeftPlacement:
        case Dock::RightPlacement:
        default:
            layout.setIsVertical (false);
    }
}

DockArea::~DockArea ()
{
}

void DockArea::append (DockItem* const item)
{
    layout.append (item);
    addAndMakeVisible (item);
    resized();
}

void DockArea::detachItem (DockItem* item)
{
    removeChildComponent (item);
    layout.remove (item);
}

void DockArea::resized()
{
    layout.layoutItems (0, 0, getWidth(), getHeight());
}

}
