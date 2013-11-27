/*
    element_base.h - This file is part of Element
    Copyright (C) 2013  Michael Fisher <mfisher31@gmail.com>

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

#ifndef ELEMENT_BASE_H_INCLUDED
#define ELEMENT_BASE_H_INCLUDED

#include <atomic>
#include <set>

#include "modules/juce_core/juce_core.h"
#include "modules/juce_cryptography/juce_cryptography.h"

#include <boost/bind.hpp>
#include <boost/intrusive/list.hpp>
#include <boost/intrusive/slist_hook.hpp>
#include <boost/signals2/signal.hpp>

#include <lv2/lv2plug.in/ns/lv2core/lv2.h>
#include <lv2/lv2plug.in/ns/ext/atom/atom.h>
#include <lv2/lv2plug.in/ns/ext/midi/midi.h>
#include <lv2/lv2plug.in/ns/ext/urid/urid.h>

//namespace juce {
namespace Element {
using namespace juce;
    
#include "core/Arc.h"
#include "core/Atomic.h"
#include "core/Controller.h"
#include "core/Intrusive.h"
#include "core/LinkedList.h"
#include "core/Monitor.h"
#include "core/Pointer.h"
#include "core/Signals.h"
#include "core/Slugs.h"
#include "core/Types.h"
#include "core/URIs.h"
#include "core/Module.h"
#include "core/WorldBase.h"
    
#include "time/DelayLockedLoop.h"
#include "time/Tempo.h"
#include "time/TimeScale.h"
    
#include "util/Utils.h"
#include "util/Convert.h"
#include "util/FileHelpers.h"
#include "util/Fraction.h"
#include "util/Midi.h"
#include "util/RangeTypes.h"
#include "util/RelativePath.h"
#include "util/UUID.h"
    
}//}

#endif   // ELEMENT_BASE_H_INCLUDED