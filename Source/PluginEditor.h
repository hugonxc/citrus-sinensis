/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class CitrussinensisAudioProcessorEditor  : public juce::AudioProcessorEditor, juce::Timer, juce::Slider::Listener
{
public:
    CitrussinensisAudioProcessorEditor (CitrussinensisAudioProcessor&);
    ~CitrussinensisAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    //==============================================================================
    void timerCallback() override;
    void sliderValueChanged(juce::Slider *slider) override;
    
    //==============================================================================
    void drawNextFrameOfSpectrum();
    void drawFrame (juce::Graphics& g);
    
    //==============================================================================
    juce::Rectangle<int> titleLeftRect, titleRightRect, titleLogoRect;
    
    juce::Rectangle<int> spectrumRect;
    
    juce::Rectangle<int> gainRect;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    CitrussinensisAudioProcessor& audioProcessor;
    
    juce::dsp::FFT forwardFFT;
    juce::dsp::WindowingFunction<float> window;
    
    juce::Slider gainDial;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CitrussinensisAudioProcessorEditor)
};
