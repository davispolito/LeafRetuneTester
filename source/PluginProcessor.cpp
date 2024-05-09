#include "PluginProcessor.h"
#include "PluginEditor.h"

StatefulPlugin::StatefulPlugin() : chowdsp::PluginBase<State> (&undoManager)
{
    pluginSettings->initialise ("ChowdhuryDSP/Stateful Plugin/.plugin_settings.json");

    ///////THIS IS THE FILE YOU READ IN ////
    const auto testFile = juce::File("~/Downloads/test.wav");
    auto [buffer, sampleRate] = saveLoadHelper.loadFile (testFile);

    _buffer.makeCopyOf(buffer);
    _num_samples = _buffer.getNumSamples();
    index = 0;
    LEAF_init(&leaf, 44100.0f, dummy_memory, 32, [](){return (float)rand()/RAND_MAX;});
    leaf.clearOnAllocation = 1;
    tCycle_init(&myOsc, &leaf);
    tSimpleRetune_init(&shifter, 1, 100.0f, 1000.0f, 512, &leaf);
}

void StatefulPlugin::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    LEAF_setSampleRate(&leaf, sampleRate);
}

void StatefulPlugin::processAudioBlock (juce::AudioBuffer<float>& buffer)
{
    const auto numSamples = buffer.getNumSamples();

    tCycle_setFreq(&myOsc, state.params.freqParam->getCurrentValue());
    tSimpleRetune_tuneVoice(&shifter, 0, state.params.shiftParam->getCurrentValue());


    for (int i = 0; i< numSamples; i++)
    {
        float outputSamp = tCycle_tick(&myOsc) * 0.25f;
        outputSamp = tSimpleRetune_tick(&shifter, _buffer.getSample(0, index++ % _num_samples));
        buffer.setSample(0, i, outputSamp);
        buffer.setSample(1, i, outputSamp);
    }
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
