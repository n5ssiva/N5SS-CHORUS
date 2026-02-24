#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
N5ssChorusProcessor::N5ssChorusProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
    initPresets();
}

N5ssChorusProcessor::~N5ssChorusProcessor() {}

//==============================================================================
void N5ssChorusProcessor::initPresets()
{
    // Preset values: rA, rB, dA, dB, delA, delB, fbAB, fbBA, lpf
    // Keys are bitmasks of active mode buttons (1-4)
    
    // Single button presets
    presets[1]  = { 0.28f, 0.31f, 1.8f, 1.5f, 20.0f, 21.5f, 0.08f, 0.06f, 0.70f };
    presets[2]  = { 0.38f, 0.42f, 3.2f, 2.8f, 22.0f, 24.0f, 0.15f, 0.12f, 0.65f };
    presets[4]  = { 0.52f, 0.58f, 4.5f, 4.0f, 24.0f, 27.0f, 0.20f, 0.17f, 0.58f };
    presets[8]  = { 0.65f, 0.72f, 6.0f, 5.5f, 26.0f, 30.0f, 0.25f, 0.22f, 0.50f };
    
    // Two button combinations
    presets[3]  = { 0.30f, 0.38f, 2.2f, 2.0f, 20.5f, 23.0f, 0.10f, 0.09f, 0.68f };
    presets[5]  = { 0.35f, 0.48f, 3.0f, 2.5f, 21.0f, 25.0f, 0.14f, 0.11f, 0.63f };
    presets[9]  = { 0.40f, 0.55f, 3.5f, 3.8f, 22.0f, 27.0f, 0.16f, 0.14f, 0.58f };
    presets[6]  = { 0.45f, 0.50f, 3.8f, 3.5f, 23.0f, 25.5f, 0.18f, 0.15f, 0.60f };
    presets[10] = { 0.50f, 0.62f, 4.8f, 4.5f, 24.0f, 28.5f, 0.22f, 0.19f, 0.53f };
    presets[12] = { 0.58f, 0.68f, 5.5f, 5.2f, 25.0f, 29.0f, 0.24f, 0.21f, 0.48f };
}

//==============================================================================
int N5ssChorusProcessor::getKey()
{
    int k = 0;
    if (modeActive[0]) k |= 1;
    if (modeActive[1]) k |= 2;
    if (modeActive[2]) k |= 4;
    if (modeActive[3]) k |= 8;
    return k;
}

//==============================================================================
void N5ssChorusProcessor::applyPreset()
{
    int k = getKey();
    
    // Bypass if no mode buttons active OR if wet level is Bypass
    if (k == 0 || wetLevel == WetLevel::Bypass)
    {
        bypassed = true;
        return;
    }
    
    bypassed = false;
    
    auto it = presets.find(k);
    if (it != presets.end())
    {
        auto& p = it->second;
        rA = p.rA; rB = p.rB;
        dA = p.dA; dB = p.dB;
        delA = p.delA; delB = p.delB;
        fbAB = p.fbAB; fbBA = p.fbBA;
        lpf = p.lpf;
    }
}

//==============================================================================
float N5ssChorusProcessor::getWetMixValue() const
{
    switch (wetLevel)
    {
        case WetLevel::Bypass: return 0.0f;
        case WetLevel::Mix25:  return 0.25f;
        case WetLevel::Mix50:  return 0.50f;
        case WetLevel::Mix75:  return 0.75f;
        case WetLevel::Mix100: return 1.0f;
        default:               return 0.0f;
    }
}

//==============================================================================
void N5ssChorusProcessor::prepareToPlay(double sampleRate, int)
{
    sr = sampleRate;
    wPos = 0;
    phA = 0.0f;
    phB = 0.5f;
    
    // Clear delay buffers and filter states
    for (int c = 0; c < 2; c++)
    {
        for (int i = 0; i < maxDelay; i++)
        {
            dlA[c][i] = 0;
            dlB[c][i] = 0;
        }
        lpA[c] = 0;
        lpB[c] = 0;
        comp[c] = 0;
    }
}

void N5ssChorusProcessor::releaseResources() {}

//==============================================================================
void N5ssChorusProcessor::processBlock(juce::AudioBuffer<float>& buf, juce::MidiBuffer&)
{
    int ns = buf.getNumSamples();
    int nc = juce::jmin(buf.getNumChannels(), 2);

    // Skip processing if bypassed
    if (bypassed)
        return;

    // Get current wet mix value from enum
    float mix = getWetMixValue();
    
    // Convert delay times from ms to samples
    float bA = delA * 0.001f * (float)sr;
    float bB = delB * 0.001f * (float)sr;
    float dpA = dA * 0.001f * (float)sr;
    float dpB = dB * 0.001f * (float)sr;
    
    // LFO phase increments
    float piA = rA / (float)sr;
    float piB = rB / (float)sr;
    float pi2 = 2.0f * juce::MathConstants<float>::pi;

    for (int i = 0; i < ns; i++)
    {
        // Calculate LFO values
        float lA = std::sin(pi2 * phA);
        float lB = std::sin(pi2 * phB);

        // Calculate modulated delay times
        float cA = juce::jlimit(1.0f, (float)(maxDelay - 1), bA + lA * dpA);
        float cB = juce::jlimit(1.0f, (float)(maxDelay - 1), bB + lB * dpB);

        // Calculate read positions with wraparound
        float rpA = (float)wPos - cA;
        if (rpA < 0) rpA += maxDelay;
        float rpB = (float)wPos - cB;
        if (rpB < 0) rpB += maxDelay;

        // Linear interpolation indices
        int a0 = (int)rpA, a1 = (a0 + 1) % maxDelay;
        float fA = rpA - (float)a0;
        int b0 = (int)rpB, b1 = (b0 + 1) % maxDelay;
        float fB = rpB - (float)b0;

        // Process each channel
        for (int c = 0; c < nc; c++)
        {
            float* d = buf.getWritePointer(c);
            float dry = d[i];

            // Soft compression on input
            float cx = dry;
            float lv = std::fabs(dry);
            comp[c] = comp[c] * 0.995f + lv * 0.005f;
            if (comp[c] > 0.01f)
                cx = dry * (1.0f / (1.0f + comp[c] * 0.5f));

            // Read from delay lines with interpolation
            float wA = dlA[c][a0] * (1.0f - fA) + dlA[c][a1] * fA;
            float wB = dlB[c][b0] * (1.0f - fB) + dlB[c][b1] * fB;

            // Apply lowpass filtering
            lpA[c] += lpf * (wA - lpA[c]); wA = lpA[c];
            lpB[c] += lpf * (wB - lpB[c]); wB = lpB[c];

            // Write to delay lines with cross-feedback
            dlA[c][wPos] = cx + wB * fbBA;
            dlB[c][wPos] = cx + wA * fbAB;

            // Stereo width mixing (slightly different L/R balance)
            float sw = (c == 0) ? wA * 0.6f + wB * 0.4f : wA * 0.4f + wB * 0.6f;
            
            // Makeup gain for compression
            if (comp[c] > 0.01f)
                sw *= (1.0f + comp[c] * 0.5f);

            // Mix dry/wet
            d[i] = dry * (1.0f - mix) + sw * mix;
        }

        // Advance LFO phases
        phA += piA; if (phA >= 1.0f) phA -= 1.0f;
        phB += piB; if (phB >= 1.0f) phB -= 1.0f;
        
        // Advance write position
        wPos = (wPos + 1) % maxDelay;
    }
}

//==============================================================================
juce::AudioProcessorEditor* N5ssChorusProcessor::createEditor()
{
    return new N5ssChorusEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new N5ssChorusProcessor();
}
