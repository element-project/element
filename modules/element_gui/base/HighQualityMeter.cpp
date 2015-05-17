/*
 ==============================================================================

 This file is part of the JUCETICE project - Copyright 2009 by Lucio Asnaghi.

 JUCETICE is based around the JUCE library - "Jules' Utility Class Extensions"
 Copyright 2007 by Julian Storer.

 ------------------------------------------------------------------------------

 JUCE and JUCETICE can be redistributed and/or modified under the terms of
 the GNU General Public License, as published by the Free Software Foundation;
 either version 2 of the License, or (at your option) any later version.

 JUCE and JUCETICE are distributed in the hope that they will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with JUCE and JUCETICE; if not, visit www.gnu.org/licenses or write to
 Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 Boston, MA 02111-1307 USA

 ==============================================================================

   @author  Rui Nuno Capela
   @tweaker Lucio Asnaghi

 ==============================================================================
*/

#if EL_COMPLETION
#include "JuceHeader.h"
#endif

// Meter level limits (in dB).
#define HQ_METER_MAXDB        (+4.0f)
#define HQ_METER_MINDB        (-70.0f)

// The decay rates (magic goes here :).
// - value decay rate (faster)
#define HQ_METER_DECAY_RATE1    (1.0f - 3E-2f)
// - peak decay rate (slower)
#define HQ_METER_DECAY_RATE2    (1.0f - 3E-6f)
// Number of cycles the peak stays on hold before fall-off.
#define HQ_METER_PEAK_FALLOFF    16

DigitalMeterValue::DigitalMeterValue (DigitalMeter* parent)
  : meter (parent),
    value (0.0f),
    valueHold (0),
    valueDecay (HQ_METER_DECAY_RATE1),
    peak (0),
    peakHold (0),
    peakDecay (HQ_METER_DECAY_RATE2),
    peakColor (DigitalMeter::Color6dB)
{ }

DigitalMeterValue::~DigitalMeterValue() { }

void DigitalMeterValue::setValue (const float newValue)
{
    value = newValue;
}

void DigitalMeterValue::resetPeak()
{
    peak = 0;
}

void DigitalMeterValue::refresh()
{
    if (value > 0.001f || peak > 0)
        repaint();
}

void DigitalMeterValue::paint (Graphics& g)
{
    const bool vertical = meter->isVertical();

    const int w = getWidth();
    const int h = getHeight();
    int level = 0;

    if (isEnabled())
    {
        g.setColour (meter->color (DigitalMeter::ColorBack));
        g.fillRect (0, 0, w, h);

        level = meter->getIECLevel (DigitalMeter::Color0dB);

        g.setColour (meter->color (DigitalMeter::ColorFore));
        (vertical) ? g.drawLine (0, h - level, w, h - level)
                   : g.drawLine (level, 0, level, h);
    }
    else
    {
        g.setColour (Colours::black);
        g.fillRect (0, 0, w, h);
    }

    float dB = HQ_METER_MINDB;
    if (value > 0.0f)
        dB = 20.0f * log10f (value);

    if (dB < HQ_METER_MINDB)
        dB = HQ_METER_MINDB;
    else if (dB > HQ_METER_MAXDB)
        dB = HQ_METER_MAXDB;

    level = meter->getIECScale (dB);
    if (valueHold < level)
    {
        valueHold = level;
        valueDecay = HQ_METER_DECAY_RATE1;
    }
    else
    {
        valueHold = int (float (valueHold * valueDecay));
        if (valueHold < level)
        {
            valueHold = level;
        }
        else
        {
            valueDecay *= valueDecay;
            level = valueHold;
        }
    }

    int ptOver = 0;
    int ptCurr = 0;
    int colorLevel;
    for (colorLevel = DigitalMeter::Color10dB;
         colorLevel > DigitalMeter::ColorOver && level >= ptOver;
         colorLevel--)
    {
        ptCurr = meter->getIECLevel (colorLevel);

//        g.setColour (m_pMeter->color (iLevel));

        if (vertical) {
            g.setGradientFill (ColourGradient (meter->color (colorLevel), 0, h - ptOver,
                                               meter->color (colorLevel-1), 0, h - ptCurr,
                                               false));
        } else {
            g.setGradientFill (ColourGradient (meter->color (colorLevel), ptOver, 0,
                                               meter->color (colorLevel - 1), ptCurr, 0,
                                               false));
        }

        if (level < ptCurr) {
            (vertical) ? g.fillRect (0, h - level, w, level - ptOver)
                       : g.fillRect (ptOver, 0, level - ptOver, h);
        } else {
            (vertical) ? g.fillRect (0, h - ptCurr, w, ptCurr - ptOver)
                       : g.fillRect (ptCurr, 0, ptCurr - ptOver, h);
        }

        ptOver = ptCurr;
    }

    if (level > ptOver)
    {
        g.setColour (meter->color (DigitalMeter::ColorOver));
        (level) ? g.fillRect (0, h - level, w, level - ptOver)
                : g.fillRect (level, 0, level - ptOver, h);
    }

    if (peak < level)
    {
        peak = level;
        peakHold = 0;
        peakDecay = HQ_METER_DECAY_RATE2;
        peakColor = colorLevel;
    }
    else if (++peakHold > meter->getPeakFalloff())
    {
        peak = int (float (peak * peakDecay));
        if (peak < level) {
            peak = level;
        } else {
            if (peak < meter->getIECLevel (DigitalMeter::Color10dB))
                peakColor = DigitalMeter::Color6dB;
            peakDecay *= peakDecay;
        }
    }

    g.setColour (meter->color (peakColor));
    (vertical) ? g.drawLine (0, h - peak, w, h - peak)
               : g.drawLine (peak, 0, peak, h);
}

void DigitalMeterValue::resized()
{
    peak = 0;
}

DigitalMeter::DigitalMeter (const int numPorts, bool _horizontal)
  : portCount (numPorts), // FIXME: Default port count.
    values (0),
    scale (0.0f),
    peakFalloff (HQ_METER_PEAK_FALLOFF),
    horizontal (_horizontal)
{
    getLookAndFeel().setColour (DigitalMeter::levelOverColourId, Colours::yellow.darker());
    getLookAndFeel().setColour (DigitalMeter::level0dBColourId, Colours::whitesmoke);
    getLookAndFeel().setColour (DigitalMeter::level3dBColourId, Colours::lightgreen);
    getLookAndFeel().setColour (DigitalMeter::level6dBColourId, Colours::green);
    getLookAndFeel().setColour (DigitalMeter::level10dBColourId, Colours::darkgreen.darker());
    getLookAndFeel().setColour (DigitalMeter::backgroundColourId, Colours::transparentBlack);
    getLookAndFeel().setColour (DigitalMeter::foregroundColourId, Colours::transparentWhite);

    for (int i = 0; i < LevelCount; i++)
        levels[i] = 0;

    colors[ColorOver] = findColour (levelOverColourId);
    colors[Color0dB]  = findColour (level0dBColourId);
    colors[Color3dB]  = findColour (level3dBColourId);
    colors[Color6dB]  = findColour (level6dBColourId);
    colors[Color10dB] = findColour (level10dBColourId);
    colors[ColorBack] = findColour (backgroundColourId);
    colors[ColorFore] = findColour (foregroundColourId);

    if (portCount > 0)
    {
        values = new DigitalMeterValue* [portCount];
        for (int port = 0; port < portCount; port++)
        {
            values[port] = new DigitalMeterValue (this);
            addAndMakeVisible (values[port]);
        }
    }
}


// Default destructor.
DigitalMeter::~DigitalMeter()
{
    for (int port = 0; port < portCount; port++)
        delete values[port];
    delete[] values;
}

void DigitalMeter::resized()
{
    const int length = horizontal ? getWidth() : getHeight();
    scale = 0.85f * (float) length;

    levels[Color0dB]  = getIECScale (0.0f);
    levels[Color3dB]  = getIECScale (-3.0f);
    levels[Color6dB]  = getIECScale (-6.0f);
    levels[Color10dB] = getIECScale (-10.0f);

    const int size = (horizontal ? getWidth() : getHeight()) / portCount;

    for (int port = 0; port < portCount; ++port)
    {
        if (horizontal)
            values[port]->setBounds (0, port * size, getWidth(), size);
        else
            values[port]->setBounds (port * size, 0, size, getHeight());
    }
}

void DigitalMeter::paint (Graphics&)
{
/*
    g.drawBevel (0, 0, getWidth(), getHeight(), 1,
                 Colours::black.withAlpha(0.2f),
                 Colours::white.withAlpha(0.2f));
*/
}


// Child widget accessors.
int DigitalMeter::getIECScale (const float dB) const
{
    float defaultScale = 1.0;

    if (dB < -70.0)
        defaultScale = 0.0;
    else if (dB < -60.0)
        defaultScale = (dB + 70.0) * 0.0025;
    else if (dB < -50.0)
        defaultScale = (dB + 60.0) * 0.005 + 0.025;
    else if (dB < -40.0)
        defaultScale = (dB + 50.0) * 0.0075 + 0.075;
    else if (dB < -30.0)
        defaultScale = (dB + 40.0) * 0.015 + 0.15;
    else if (dB < -20.0)
        defaultScale = (dB + 30.0) * 0.02 + 0.3;
    else // if (dB < 0.0)
        defaultScale = (dB + 20.0) * 0.025 + 0.5;

    return (int) (defaultScale * scale);
}

int DigitalMeter::getIECLevel (const int index) const
{
    return levels[index];
}

int DigitalMeter::getPortCount () const { return portCount; }
void DigitalMeter::setPeakFalloff (const int newPeakFalloff) { peakFalloff = newPeakFalloff; }
int DigitalMeter::getPeakFalloff() const { return peakFalloff; }

void DigitalMeter::resetPeaks ()
{
    for (int iPort = 0; iPort < portCount; iPort++)
        values[iPort]->resetPeak();
}

void DigitalMeter::refresh ()
{
    for (int iPort = 0; iPort < portCount; iPort++)
        values[iPort]->refresh();
}

void DigitalMeter::setValue (const int port, const float value)
{
    if (isPositiveAndBelow (port, portCount))
        values[port]->setValue (value);
}

const Colour& DigitalMeter::color (const int index) const
{
    return index < ColorCount ? colors[index] : Colours::greenyellow;
}


