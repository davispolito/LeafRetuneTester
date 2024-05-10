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
    _ringBuffer.setSize(1, 4096, true, false, false);
    index = 0;
    ringRead = 0;
    ringWrite = 0;
    LEAF_init(&leaf, 44100.0f, dummy_memory, 32, [](){return (float)rand()/RAND_MAX;});
    leaf.clearOnAllocation = 1;
    tCycle_init(&myOsc, &leaf);
    tSimpleRetune_init(&shifter, 1, 100.0f, 1000.0f, 512, &leaf);
    tAttackDetection_init(&ad, 1, 10, 1000, &leaf);
    prevAttack = 0.0f;
    attackCount = 0;
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
        float outputSamp = _buffer.getSample(0, index++ % _num_samples);

        int attack = tAttackDetection_detect(&ad, &outputSamp);

        if ((attack > 0) && (prevAttack == 0))
        {
            DBG(attackCount++);
        }
        prevAttack = attack;



        outputSamp = tSimpleRetune_tick(&shifter, outputSamp);

        _ringBuffer.setSample(0, ringWrite++ % 4096, outputSamp);
        buffer.setSample(0, i, outputSamp);


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
