/*
    element_engines.cpp - This file is part of Element
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

#if ELEMENT_USE_LIBJUCE
 #include <juce/modules/config.h>
 #include "element/modules/config.h"
#else
 #include "AppConfig.h"
#endif

#include "./element_engines.h"

// This is included here to avoid problems with circular dependencies
#include "../element_lv2/element_lv2.h"

namespace Element {

#include "common/MidiSequencePlayer.cpp"
#include "common/Processor.cpp"
#include "common/Shuttle.cpp"

#if ELEMENT_USE_JACK
 #include "jack/JackClient.cpp"
 #include "jack/JackDevice.cpp"
#endif
}
