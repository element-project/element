/*
    element_base.h - This file is part of Element
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

#ifndef ELEMENT_BASE_H_INCLUDED
#define ELEMENT_BASE_H_INCLUDED

#include "modules/juce_cryptography/juce_cryptography.h"

#if _MSC_VER
 #pragma warning( disable : 4305 )
 #pragma warning( disable : 4512 )
 #pragma warning( disable : 4996 )
 #if _MSC_VER >= 1800
  #include <atomic>
 #endif
#else
 #include <atomic>
#endif

#include <set>
#include <boost/bind.hpp>
#include <boost/intrusive/list.hpp>
#include <boost/intrusive/slist_hook.hpp>
#include <boost/signals2/signal.hpp>

#if _MSC_VER
 #ifdef min
  #undef min
 #endif
 #ifdef max
  #undef max
 #endif

 #if _MSC_VER < 1800
  #define llrint(x) juce::roundDoubleToInt(x)
 #endif
#endif

namespace Element {
using namespace juce;
#include "core/Arc.h"
#include "core/Atomic.h"
#include "core/Intrusive.h"
#include "core/LinkedList.h"
#include "core/Monitor.h"
#include "core/Parameter.h"
#include "core/Pointer.h"
#include "core/PortType.h"
#include "core/RingBuffer.h"
#include "core/Semaphore.h"
#include "core/Signals.h"
#include "core/Slugs.h"
#include "core/Types.h"
#include "core/WorkThread.h"

#include "time/DelayLockedLoop.h"
#include "time/Tempo.h"
#include "time/TimeScale.h"
#include "time/TimeStamp.h"

#include "util/Utils.h"
#include "util/Convert.h"
#include "util/FileHelpers.h"
#include "util/Fraction.h"
#include "util/Midi.h"
#include "util/RangeTypes.h"
#include "util/RelativePath.h"
#include "util/UUID.h"
}

#endif // ELEMENT_BASE_H_INCLUDED