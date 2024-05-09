#include "PluginProcessor.h"
#include "PluginEditor.h"

StatefulPlugin::StatefulPlugin() : chowdsp::PluginBase<State> (&undoManager)
{
    pluginSettings->initialise ("ChowdhuryDSP/Stateful Plugin/.plugin_settings.json");
//    presetManager = std::make_unique<chowdsp::presets::PresetManager> (state, this, ".chowpreset");
//    presetManager->getPresetTree().treeInserter = &chowdsp::presets::PresetTreeInserters::vendorInserter;
//    presetsSettings.emplace (*presetManager,
//        *pluginSettings,
//        juce::File::getSpecialLocation (juce::File::userDocumentsDirectory).getChildFile ("Chowdhury DSP/Presets/Stateful Plugin"));
    LEAF_init(&leaf, 44100.0f, dummy_memory, 32, [](){return (float)rand()/RAND_MAX;});
    leaf.clearOnAllocation = 1;
    tCycle_init(&myOsc, &leaf);
}

void StatefulPlugin::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    LEAF_setSampleRate(&leaf, sampleRate);
}

void StatefulPlugin::processAudioBlock (juce::AudioBuffer<float>& buffer)
{
    if (! state.params.onOff->get())
        return;

    const auto numSamples = buffer.getNumSamples();

    tCycle_setFreq(&myOsc, state.params.freqParam->getCurrentValue());
    for (int i = 0; i< numSamples; i++)
    {
        float outputSamp = tCycle_tick(&myOsc) * 0.25f;
        buffer.setSample(0, i, outputSamp);
    }
    //buffer.applyGain (0, 0, numSamples, leftGain);
    //buffer.applyGain (1, 0, numSamples, rightGain);
}

juce::AudioProcessorEditor* StatefulPlugin::createEditor()
{
    return new PluginEditor { *this };
}

// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new StatefulPlugin();
}
