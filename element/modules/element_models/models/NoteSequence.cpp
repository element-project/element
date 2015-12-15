/*
    NoteSequence.cpp - This file is part of Element
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


/** Add a note from a value tree object */
Note
NoteSequence::addNote (const ValueTree& tree)
{
    if (tree.hasType (Slugs::note))
        return Note::make (ValueTree::invalid);
    
    if (tree.getParent().isValid())
    {
        ValueTree p = tree.getParent();
        p.removeChild (tree, nullptr);
    }
    
    Note nt = Note::make (tree);
    node().addChild (nt.node(), -1, nullptr);
    
    return nt;
}

int32
NoteSequence::ppq() const
{
    return node().getProperty (Slugs::ppq, 1920);
}
