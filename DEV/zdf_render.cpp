/*
================================================================================
FILE: zdf_render.cpp
================================================================================
ACADEMIC PROJECT: Zero Delay Feedback Moog Ladder Filter Implementation
RESEARCH CONTEXT: Virtual Analog Modeling and Real-time Audio Processing
AUTHOR: [Timothy Paul Read]
DATE: [2025/5/25]
INSTITUTION: [Goldsmiths, University of London]

ABSTRACT:
This implementation demonstrates a Zero Delay Feedback (ZDF) approach to digital
Moog ladder filter modeling. The ZDF technique, developed by Vadim Zavalishin,
solves the fundamental problem of delay-free feedback loops in digital 
implementations of analog circuits. This particular implementation showcases
real-time parameter modulation using Low Frequency Oscillation (LFO) control
of the filter cutoff frequency.

THEORETICAL BACKGROUND:
The Moog ladder filter, originally designed by Robert Moog in the 1960s, is
characterized by its four-pole lowpass response with resonant feedback. The
analog circuit's feedback path creates mathematical challenges when digitized,
as discrete-time systems inherently introduce delay. The ZDF approach maintains
the instantaneous feedback characteristics of the analog original.

RESEARCH CONTRIBUTIONS:
1. Demonstrates ZDF implementation in embedded real-time context
2. Shows dynamic parameter modulation techniques
3. Illustrates proper memory management for real-time audio objects
4. Provides baseline for comparison with other digital modeling approaches

IMPLEMENTATION NOTES:
- Uses pointer-based object management for runtime flexibility
- Implements sinusoidal LFO with proper phase accumulation
- Maintains real-time constraints through efficient coefficient updates
- Demonstrates professional audio development practices
================================================================================
*/

#include <Bela.h>              // Bela real-time audio framework
#include <cmath>               // Mathematical functions for signal processing
#include "ZDFMoogLadderFilter.h"  // Zero Delay Feedback Moog filter implementation

/*
================================================================================
GLOBAL STATE VARIABLES
================================================================================
ACADEMIC RATIONALE:
In real-time audio systems, global variables are often necessary to maintain
state between audio callback invocations. While generally discouraged in 
general programming, the real-time audio domain requires this approach due to:
1. Callback-based architecture constraints
2. Performance requirements (avoiding allocation in audio thread)
3. State persistence across buffer boundaries
================================================================================
*/

// FILTER OBJECT MANAGEMENT
// Declared as pointer to enable runtime initialization with sample rate
// This follows the RAII (Resource Acquisition Is Initialization) principle
// adapted for real-time audio constraints
ZDFMoogLadderFilter* zdfMLFilter = nullptr;

// OSCILLATOR STATE VARIABLES
// Primary oscillator for filter input signal generation
float phase = 0.0f;                    // Current oscillator phase [0, 2π]
float frequency = 220.0f;              // Oscillator frequency in Hz (A3 note)
float phaseIncrement = 0.0f;           // Per-sample phase increment

// LOW FREQUENCY OSCILLATOR (LFO) STATE
// Controls dynamic modulation of filter cutoff frequency
float lfoPhase = 0.0f;                 // LFO phase accumulator [0, 2π]
float lfoFreq = 0.1f;                  // LFO frequency in Hz (very slow modulation)

/*
================================================================================
FUNCTION: setup()
================================================================================
ACADEMIC PURPOSE:
Initialization function called once before audio processing begins. This function
establishes the audio processing environment and initializes all filter and
oscillator parameters. Critical for ensuring deterministic behavior in real-time
audio systems.

PARAMETERS:
@param context: BelaContext structure containing audio system configuration
@param userData: Generic pointer for user-defined data (unused in this implementation)

RETURNS:
@return bool: true if initialization successful, false otherwise

COMPUTATIONAL COMPLEXITY: O(1)
REAL-TIME SAFETY: Safe (no dynamic allocation during audio processing)
================================================================================
*/
bool setup(BelaContext *context, void *userData)
{
    /*
    PHASE INCREMENT CALCULATION
    Mathematical derivation:
    - Oscillator frequency: f Hz
    - Sample rate: fs Hz  
    - Phase increment per sample: Δφ = 2πf/fs radians
    
    This ensures proper frequency generation regardless of sample rate,
    maintaining pitch accuracy across different audio configurations.
    */
    phaseIncrement = 2.0f * M_PI * frequency / context->audioSampleRate;
    
    /*
    FILTER OBJECT INITIALIZATION
    Dynamic allocation is performed here (setup phase) rather than in the
    audio callback to maintain real-time constraints. The filter requires
    sample rate information for proper coefficient calculation.
    */
    zdfMLFilter = new ZDFMoogLadderFilter(context->audioSampleRate);
    
    /*
    FILTER STATE INITIALIZATION
    Reset ensures clean initial conditions, preventing artifacts from
    uninitialized memory or previous processing sessions.
    */
    zdfMLFilter->reset();
    
    /*
    FILTER PARAMETER CONFIGURATION
    Initial parameter values chosen based on musical and technical considerations:
    - Resonance: 0.28 provides subtle resonance without self-oscillation
    - Cutoff: 400 Hz places filter in mid-frequency range for clear audible effects
    
    Academic note: These values can be adjusted for different research objectives
    Higher resonance values (0.7-0.8) demonstrate more pronounced resonant behavior
    */
	zdfMLFilter->setResonance(0.28f);  // Moderate resonance for musical character
	zdfMLFilter->setCutoff(400.0f);    // Initial cutoff frequency in Hz
    
    return true;  // Successful initialization
}

/*
================================================================================
FUNCTION: render()
================================================================================
ACADEMIC PURPOSE:
Core audio processing function called repeatedly at audio rate. Implements the
complete signal processing chain: oscillator generation, LFO modulation, filter
processing, and audio output. This function must maintain strict real-time
constraints and deterministic execution time.

PARAMETERS:
@param context: BelaContext containing current audio buffer and system state
@param userData: Generic user data pointer (unused)

COMPUTATIONAL COMPLEXITY: O(n) where n = context->audioFrames
REAL-TIME CONSTRAINTS: Must complete within buffer duration (typically 1-10ms)
ACADEMIC SIGNIFICANCE: Demonstrates complete real-time audio signal processing chain
================================================================================
*/
void render(BelaContext *context, void *userData)
{
    /*
    MAIN AUDIO PROCESSING LOOP
    Processes audio sample-by-sample to maintain phase coherence and enable
    per-sample parameter updates. This approach allows for smooth modulation
    and prevents aliasing artifacts that could occur with block-based processing.
    */
    for(unsigned int n = 0; n < context->audioFrames; ++n)
    {
        /*
        PRIMARY OSCILLATOR GENERATION
        Generates sinusoidal waveform using phase accumulation method.
        This approach provides:
        1. High frequency accuracy
        2. Smooth phase continuity across buffer boundaries
        3. Low computational overhead compared to table lookup methods
        */
        float in = sinf(phase);           // Generate sinusoidal sample
        
        /*
        PHASE ACCUMULATION AND WRAPAROUND
        Maintains phase accumulator within [0, 2π] range to prevent
        floating-point precision degradation over long periods.
        Critical for maintaining long-term frequency stability.
        */
        phase += phaseIncrement;
        if(phase > 2.0f * M_PI)
            phase -= 2.0f * M_PI;

        /*
        LOW FREQUENCY OSCILLATOR (LFO) GENERATION
        Generates slow modulation signal for dynamic filter control.
        LFO frequency is typically much lower than audio frequencies,
        creating perceptible but non-rhythmic modulation effects.
        */
        float lfo = sinf(lfoPhase);       // Generate LFO sample [-1, +1]
        
        /*
        LFO PHASE ACCUMULATION
        Similar to primary oscillator but with much lower frequency.
        Maintains same precision considerations for long-term stability.
        */
        lfoPhase += 2.0f * M_PI * lfoFreq / context->audioSampleRate;
        if(lfoPhase > 2.0f * M_PI)
            lfoPhase -= 2.0f * M_PI;

        /*
        DYNAMIC CUTOFF FREQUENCY CALCULATION
        Maps LFO output [-1, +1] to desired cutoff frequency range [300Hz, 3000Hz].
        
        Mathematical transformation:
        1. (lfo + 1.0f) * 0.5f maps [-1,+1] to [0,1]
        2. Multiply by frequency range (3000 - 300 = 2700 Hz)
        3. Add minimum frequency (300 Hz)
        
        Result: Smooth sweep from 300Hz to 3000Hz and back
        This range spans approximately 3.3 octaves, providing dramatic audible changes
        */
        float thisCutoff = 300.0f + ((lfo + 1.0f) * 0.5f * (3000.0f - 300.0f));

        /*
        REAL-TIME FILTER PARAMETER UPDATE
        Updates filter cutoff frequency for current sample.
        ZDF filters can handle per-sample parameter changes without artifacts,
        unlike some other digital filter topologies.
        */
        zdfMLFilter->setCutoff(thisCutoff);

        /*
        FILTER PROCESSING
        Applies Moog ladder filter to input signal.
        The ZDF implementation maintains the characteristic Moog sound while
        avoiding the delay issues inherent in naive digital implementations.
        */
        float out = zdfMLFilter->process(in);

        /*
        AUDIO OUTPUT
        Writes processed signal to both left and right channels.
        Demonstrates stereo audio handling in Bela framework.
        */
        audioWrite(context, n, 0, out);   // Left channel output
        audioWrite(context, n, 1, out);   // Right channel output
    }
}

/*
================================================================================
FUNCTION: cleanup()
================================================================================
ACADEMIC PURPOSE:
Resource deallocation function called when audio processing terminates.
Essential for preventing memory leaks in embedded systems where resources
are limited. Follows RAII principles for proper resource management.

PARAMETERS:
@param context: BelaContext (unused but required by framework)
@param userData: User data pointer (unused)

ACADEMIC SIGNIFICANCE:
Demonstrates proper resource management in real-time audio systems.
Critical for embedded systems where memory leaks can cause system instability.
================================================================================
*/
void cleanup(BelaContext *context, void *userData)
{
    /*
    FILTER OBJECT DEALLOCATION
    Releases dynamically allocated filter object.
    Uses delete operator to properly call destructor and free memory.
    Essential for preventing memory leaks in embedded systems.
    */
    delete zdfMLFilter;
}

/*
================================================================================
RESEARCH APPLICATIONS AND EXTENSIONS
================================================================================

This implementation serves as a foundation for various research directions:

1. PSYCHOACOUSTIC STUDIES:
   - Compare ZDF implementation against analog hardware
   - Measure perceptual differences in filter character
   - Evaluate modulation smoothness and musical expressiveness

2. COMPUTATIONAL EFFICIENCY ANALYSIS:
   - Benchmark ZDF against other digital filter implementations
   - Analyze real-time performance on different hardware platforms
   - Study memory usage patterns and cache efficiency

3. PARAMETER MODULATION RESEARCH:
   - Investigate alternative modulation sources (envelopes, complex LFOs)
   - Study parameter smoothing techniques for artifact reduction
   - Explore multi-dimensional parameter control strategies

4. VIRTUAL ANALOG MODELING:
   - Compare ZDF accuracy against circuit simulation software
   - Investigate nonlinear modeling additions
   - Study temperature and component tolerance modeling

ACADEMIC CITATIONS:
- Zavalishin, V. (2018). "The Art of VA Filter Design"
- Stilson, T. & Smith, J. (1996). "Analyzing the Moog VCF with Considerations for Digital Implementation"
- Huovilainen, A. (2004). "Non-Linear Digital Implementation of the Moog Ladder Filter"

EXPERIMENTAL METHODOLOGY:
For research using this implementation, consider:
- Systematic parameter sweep studies
- Comparative listening tests with trained participants  
- Objective measurement using standard audio analysis tools
- Documentation of real-time performance characteristics

================================================================================
*/

