/*
BEGIN_JUCE_MODULE_DECLARATION
  ID:               fxme_juce_tools
  vendor:           odoare
  version:          0.0.1
  name:             FX-Mechanics JUCE tools
  description:      Various personnal tools for JUCE development
  website:          http://www.github.com/odoare/FxmeJuceTools
  license:          MIT
  dependencies:     juce_audio_basics, juce_audio_devices, juce_audio_formats, juce_audio_utils, juce_core, juce_data_structures, juce_events, juce_graphics, juce_gui_basics   
 END_JUCE_MODULE_DECLARATION
 */

#pragma once
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_dsp/juce_dsp.h>

namespace fxme
{
  #include "Components/FxmeMeters.h"
  #include "Components/ScopeComponent.h"
  #include "Components/SpectrogramComponent.h"
  #include "Components/SpectrumComponent.h"
  #include "Components/ScrollingScopeComponent.h"
  #include "Components/FxmeKnob.h"
  #include "Components/FxmeButton.h"
  #include "LookAndFeels/FxmeLookAndFeel.h"
  #include "Dsp/CircularFifo.h"
}
