/*
    Note.cpp - This file is part of Element
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


void
Note::setEventId (const int id) const
{
    ValueTree temp = node();
    temp.setProperty ("eventId", id, nullptr);
}

bool Note::isValid() const { return node().isValid(); }
const int Note::eventId() const { return node().getProperty ("eventId", 0); }
const int Note::keyId() const { return (int) node().getProperty (Slugs::id, 0); }
const int Note::channel() const { return (int) node().getProperty ("channel", 1); }
const float Note::velocity() const { return (float) node().getProperty("velocity", 0.8f); }
const double Note::tickStart() const { return (double) node().getProperty ("start"); }
const double Note::beatLength() const { return (double) node().getProperty ("length"); }
const double Note::tickEnd() const { return tickStart() + beatLength(); }

void
Note::getBeats (Range<double>& beats) const
{
    beats.setStart (tickStart());
    beats.setLength (beatLength());
}

void
Note::resize (EditDeltas& changes, double length)
{
    if (length < 0.015625f) {
        // 1/64th note
        length = 0.015625f;
    }

    changes.length = length - beatLength();
}

void  Note::changeLength (EditDeltas& d, double len) { resize (d, len); }

void
Note::changeChannel (EditDeltas& changes, int c)
{
    c = clampNoMoreThan (c, 1, 16);
    changes.channel = c - channel();
}

void
Note::changeKeyId (EditDeltas& changes, int key)
{
    jassert (isPositiveAndBelow (key, 128));
    changes.note = key - keyId();
}

void
Note::changeVelocity (EditDeltas& changes, float vel)
{
    changes.velocity = clampNoMoreThan (vel, 0.0f, 1.0f) - velocity();
}


void
Note::move (EditDeltas& changes, const double beat)
{
    changes.start = beat - tickStart();
}

void
Note::applyEdits (EditDeltas& changes, bool reset)
{
    if (changes.note != 0)
        node().setProperty (Slugs::id, keyId() + changes.note, nullptr);

    if (changes.channel != 0)
        node().setProperty ("channel", channel() + changes.channel, nullptr);

    if (changes.start != 0.0f)
        node().setProperty ("start", tickStart() + changes.start, nullptr);

    if (changes.length != 0.0f)
        node().setProperty ("length", beatLength() + changes.length, nullptr);

    if (reset)
        changes.reset();
}

MidiMessage
Note::noteOn() const
{
    MidiMessage mm = MidiMessage::noteOn (channel(), keyId(), velocity());
    mm.setTimeStamp (tickStart());
    return mm;
}

MidiMessage
Note::noteOff() const
{
    MidiMessage mm = MidiMessage::noteOff (channel(), keyId());
    mm.setTimeStamp (tickEnd());
    return mm;
}

void
Note::getMidi (MidiMessage& on, MidiMessage& off) const
{
    on  = noteOn();
    off = noteOff();
}

ValueTree
Note::sequenceNode() const
{
    return node().getParent();
}


Note::Note (int note, double start, double length, int channel, float velocity)
    : ObjectModel (Slugs::note)
{
    node().setProperty (Slugs::id, note, nullptr);
    node().setProperty (Slugs::channel, channel, nullptr);
    node().setProperty (Slugs::start, start, nullptr);
    node().setProperty (Slugs::length, length, nullptr);
    node().setProperty (Slugs::velocity, velocity, nullptr);
    node().setProperty ("eventId", 0, nullptr);
}
