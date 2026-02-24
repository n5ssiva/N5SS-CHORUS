#pragma once

#include <JuceHeader.h>

//==============================================================================
// Dry/Wet level presets for the mix buttons
//==============================================================================
enum class WetLevel
{
    Bypass = 0,   // 0% wet (fully dry / bypass)
    Mix25,        // 25% wet
    Mix50,        // 50% wet
    Mix75,        // 75% wet
    Mix100        // 100% wet
};

//==============================================================================
/**
 * N5ssChorusProcessor
 *
 * Vintage dimension-style chorus effect with 4 mode buttons and 5 wet level presets.
 * Based on classic BBD chorus algorithms with dual delay lines and cross-feedback.
 */
//==============================================================================
class N5ssChorusProcessor : public juce::AudioProcessor
{
public:
    N5ssChorusProcessor();
    ~N5ssChorusProcessor() override;

    //==========================================================================
    // AudioProcessor overrides
    //==========================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock&) override {}
    void setStateInformation(const void*, int) override {}

    //==========================================================================
    // Public parameters (accessed by Editor)
    //==========================================================================
    bool modeActive[4] = { false, false, false, false };  // Mode buttons 1-4
    WetLevel wetLevel = WetLevel::Bypass;                  // Current wet level
    bool bypassed = true;                                  // Bypass state
    
    void applyPreset();
    
    /** Convert WetLevel enum to actual mix value (0.0 - 1.0) */
    float getWetMixValue() const;

private:
    //==========================================================================
    // Delay line buffers
    //==========================================================================
    static const int maxDelay = 48000;
    float dlA[2][48000] = {};  // Delay line A (stereo)
    float dlB[2][48000] = {};  // Delay line B (stereo)
    int wPos = 0;              // Write position
    double sr = 44100.0;       // Sample rate
    float phA = 0.0f;          // LFO phase A
    float phB = 0.5f;          // LFO phase B (90° offset)

    //==========================================================================
    // Current preset parameters
    //==========================================================================
    float rA = 0, rB = 0;      // LFO rates (Hz)
    float dA = 0, dB = 0;      // Modulation depths (ms)
    float delA = 0, delB = 0;  // Base delay times (ms)
    float fbAB = 0, fbBA = 0;  // Cross-feedback amounts
    float lpf = 0.7f;          // Lowpass filter coefficient

    //==========================================================================
    // Processing state
    //==========================================================================
    float comp[2] = {};        // Compressor envelope followers
    float lpA[2] = {};         // Lowpass filter states A
    float lpB[2] = {};         // Lowpass filter states B

    //==========================================================================
    // Preset data
    //==========================================================================
    struct Preset
    {
        float rA, rB, dA, dB, delA, delB, fbAB, fbBA, lpf;
    };
    std::map<int, Preset> presets;
    
    void initPresets();
    int getKey();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(N5ssChorusProcessor)
};
