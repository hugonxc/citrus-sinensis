/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
CitrussinensisAudioProcessorEditor::CitrussinensisAudioProcessorEditor (CitrussinensisAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
    forwardFFT(11),
    window (1 << 11, juce::dsp::WindowingFunction<float>::hann)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);
    startTimerHz (30);
    
    gainDial.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    gainDial.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, 50);
    gainDial.setRange(-48.0, 10.0);
    gainDial.setValue(-1.0);
    gainDial.addListener(this);
    addAndMakeVisible(gainDial);
}

CitrussinensisAudioProcessorEditor::~CitrussinensisAudioProcessorEditor()
{
}

//==============================================================================
void CitrussinensisAudioProcessorEditor::paint (juce::Graphics& g)
{
    for (int i = 1; i < audioProcessor.scopeSize; ++i)
    {
        auto width  = getLocalBounds().getWidth();
        auto height = getLocalBounds().getHeight();
        
        g.drawText("spectrum", 0, 0, width, height, juce::Justification::centred);

        g.drawLine ({ (float) juce::jmap (i - 1, 0, audioProcessor.scopeSize - 1, 0, width),
                              juce::jmap (audioProcessor.scopeData[i - 1], 0.0f, 1.0f, (float) height, 0.0f),
                      (float) juce::jmap (i,     0, audioProcessor.scopeSize - 1, 0, width),
                              juce::jmap (audioProcessor.scopeData[i],     0.0f, 1.0f, (float) height, 0.0f) });
    }
}

void CitrussinensisAudioProcessorEditor::resized()
{
    juce::Rectangle<int> area = getBounds();
    
    gainDial.setBounds(area.removeFromTop(getHeight() / 2));
    
}

//==============================================================================
void CitrussinensisAudioProcessorEditor::timerCallback()
{
    drawNextFrameOfSpectrum();
    audioProcessor.nextFFTBlockReady = false;
    repaint();
}

void CitrussinensisAudioProcessorEditor::drawNextFrameOfSpectrum ()
{
    window.multiplyWithWindowingTable(audioProcessor.fftData, audioProcessor.fftSize);
    
    forwardFFT.performFrequencyOnlyForwardTransform(audioProcessor.fftData);
    
    auto mindB = -100.0f;
    auto maxdB = 0.0f;
    
    for (int i = 0; i < audioProcessor.scopeSize; ++i)
    {
        auto skewedProportionX = 1.0f - std::exp (std::log (1.0f - (float) i / (float) audioProcessor.scopeSize) * 0.2f);
        auto fftDataIndex = juce::jlimit (0, audioProcessor.fftSize / 2, (int) (skewedProportionX * (float) audioProcessor.fftSize * 0.5f));
        auto level = juce::jmap (juce::jlimit (mindB, maxdB, juce::Decibels::gainToDecibels (audioProcessor.fftData[fftDataIndex]) - juce::Decibels::gainToDecibels ((float) audioProcessor.fftSize)),mindB, maxdB, 0.0f, 1.0f);
     
        audioProcessor.scopeData[i] = level;
    }
}

//==============================================================================
void CitrussinensisAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
        if (slider == &gainDial)
        {
            audioProcessor.rawVolume = pow(10, gainDial.getValue() / 20);
        }
}
