/*
    Icons.h - This file is part of Element
    Copyright (C) 2015  Michael Fisher <mfisher31@gmail.com>

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

#ifndef ELEMENT_ICONS_H
#define ELEMENT_ICONS_H

struct Icon
{
    Icon() : path (nullptr) { }
    Icon (const Path& p, const Colour& c)  : path (&p), colour (c) {}
    Icon (const Path* p, const Colour& c)  : path (p),  colour (c) {}

    void draw (Graphics& g, const Rectangle<float>& area, bool isCrossedOut) const
    {
        if (path != nullptr)
        {
            g.setColour (colour);

            const RectanglePlacement placement (RectanglePlacement::centred | RectanglePlacement::onlyReduceInSize);
            g.fillPath (*path, placement.getTransformToFit (path->getBounds(), area));

            if (isCrossedOut)
            {
                g.setColour (Colours::red.withAlpha (0.8f));
                g.drawLine ((float) area.getX(), area.getY() + area.getHeight() * 0.2f,
                            (float) area.getRight(), area.getY() + area.getHeight() * 0.8f, 3.0f);
            }
        }
    }

    Icon withContrastingColourTo (const Colour& background) const
    {
        return Icon (path, background.contrasting (colour, 0.6f));
    }

    const Path* path;
    Colour colour;
};



class Icons : public DeletedAtShutdown
{
public:
    Icons();

    Path folder, document, imageDoc,
         config, exporter, juceLogo,
         graph, jigsaw, info, warning,
         bug, mainJuceLogo;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Icons)
};

const Icons& getIcons();


#endif   // ELEMENT_ICONS_H
