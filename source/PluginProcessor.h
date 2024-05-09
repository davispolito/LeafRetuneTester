#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#if (MSVC)
#include "ipps.h"
#endif

#include "LEAF/leaf.h"
#include <chowdsp_plugin_base/chowdsp_plugin_base.h>
#include <chowdsp_plugin_utils/chowdsp_plugin_utils.h>
#include <chowdsp_plugin_state/chowdsp_plugin_state.h>

struct LevelParams : chowdsp::ParamHolder
{
    LevelParams() : chowdsp::ParamHolder ("Level")
    {
        add (percent, gain);
    }

    static constexpr auto percentID = chowdsp::StringLiteral { "percent" };
    chowdsp::PercentParameter::Ptr percent { juce::ParameterID { percentID, 100 }, "Percent" };
    chowdsp::GainDBParameter::Ptr gain { juce::ParameterID { chowdsp::StringLiteral { "gain" }, 100 }, "Gain", juce::NormalisableRange { -30.0f, 0.0f }, 0.0f };
};

struct PluginParameterState : chowdsp::ParamHolder
{
    PluginParameterState()
    {
        add (freqParam, shiftParam);
    }

    chowdsp::FloatParameter::Ptr freqParam {
        juce::ParameterID { "Frequency", 100 },
        "Frequency",
        chowdsp::ParamUtils::createNormalisableRange (100.0f, 1000.0f, 300.0f),
        300.0f,
        &chowdsp::ParamUtils::floatValToString,
        &chowdsp::ParamUtils::stringToFloatVal
    };

    chowdsp::FloatParameter::Ptr shiftParam {
        juce::ParameterID { "Shift", 100 },
        "Shift",
        chowdsp::ParamUtils::createNormalisableRange (0.25f, 4.0f, 1.0f),
        1.0f,
        &chowdsp::ParamUtils::floatValToString,
        &chowdsp::ParamUtils::stringToFloatVal
    };
};

struct PluginNonParameterState : chowdsp::NonParamState
{
    PluginNonParameterState()
    {
        addStateValues ({ &editorBounds });
    }

    chowdsp::StateValue<juce::Point<int>> editorBounds { "editor_bounds", { 600, 1000 } };
};

using State = chowdsp::PluginStateImpl<PluginParameterState, PluginNonParameterState>;

class StatefulPlugin : public chowdsp::PluginBase<State>
{
public:
    StatefulPlugin();

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processAudioBlock (juce::AudioBuffer<float>& buffer) override;

    juce::AudioProcessorEditor* createEditor() override;

    juce::UndoManager undoManager { 3000 };

    chowdsp::SharedPluginSettings pluginSettings;
    //std::optional<chowdsp::presets::frontend::SettingsInterface> presetsSettings;
    LEAF leaf;
    tCycle myOsc;
    tSimpleRetune shifter;
    tAttackDetection ad;
    chowdsp::AudioFileSaveLoadHelper saveLoadHelper;


    juce::AudioBuffer<float> _buffer;
    juce::AudioBuffer<float> _ringBuffer;
    int ringRead;
    int ringWrite;
    int _num_samples;
    int index;
    char dummy_memory[32];
    int prevAttack;
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StatefulPlugin)
};
