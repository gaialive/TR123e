/*
================================================================================
FILE: render_with_NEON_MOOGFILTER.cpp
================================================================================
ACADEMIC PROJECT: SIMD-Optimized Moog Ladder Filter with Advanced Modulation
RESEARCH CONTEXT: High-Performance Real-Time Audio Processing
OPTIMIZATION FOCUS: ARM NEON SIMD Instructions for Embedded Systems

ABSTRACT:
This implementation represents an advanced study in high-performance real-time
audio processing, combining a SIMD-optimized Moog ladder filter with sophisticated
modulation systems. The implementation demonstrates the application of ARM NEON
SIMD instructions to achieve parallel processing of audio samples, while
maintaining the complex modulation routing typical of analog synthesizers.

THEORETICAL BACKGROUND:
SIMD (Single Instruction, Multiple Data) processing allows simultaneous operation
on multiple data elements, theoretically providing 4x performance improvement
for floating-point operations. In audio processing, this translates to processing
four audio samples simultaneously, crucial for maintaining real-time constraints
in embedded systems with limited computational resources.

RESEARCH CONTRIBUTIONS:
1. Demonstrates SIMD optimization in real-time audio context
2. Integrates complex modulation systems (ADSR, portamento, key follow)
3. Shows advanced MIDI processing with velocity sensitivity and timing
4. Provides performance comparison baseline for scalar implementations

PERFORMANCE IMPLICATIONS:
- Theoretical 4x speed improvement through parallel processing
- Reduced CPU load enabling additional processing or lower latency
- Cache-efficient block processing patterns
- Optimized memory access patterns for embedded systems
================================================================================
*/

// NEON Render.cpp
#include <Bela.h>                      // Bela real-time audio framework
#include <libraries/Midi/Midi.h>       // MIDI input/output handling
#include <cmath>                       // Mathematical functions
#include "ADSR.h"                      // Attack-Decay-Sustain-Release envelope
#include "KeyFollow.h"                 // Keyboard tracking for filter frequency
#include "MidiHandler.h"               // MIDI message processing and timing
#include "MoogFilterEnvelope.h"        // Filter-specific envelope generator
#include "PortamentoFilter.h"          // Portamento detection and filtering
#include "PortamentoPlayer.h"          // Smooth note transitions
#include "ResonanceRamp.h"             // Smooth resonance parameter changes
#include "VelocityParser.h"            // Velocity-sensitive note detection
#include "MoogFilter.h"                // NEON-optimized MoogFilter implementation

/*
================================================================================
OSCILLATOR STATE MANAGEMENT
================================================================================
ACADEMIC RATIONALE:
Maintains primary oscillator state for filter input generation. Uses phase
accumulation method for high accuracy and computational efficiency. The
separation of phase and frequency allows for real-time frequency modulation
without phase discontinuities.
================================================================================
*/
float oscillatorPhase = 0.0f;         // Current oscillator phase [0, 2π]
float twoPi = 2.0f * M_PI;            // Precomputed constant for efficiency

/*
================================================================================
MIDI PROCESSING SUBSYSTEM
================================================================================
ACADEMIC PURPOSE:
Implements comprehensive MIDI processing including message parsing, timing
analysis, velocity interpretation, and portamento detection. This subsystem
demonstrates advanced real-time MIDI processing techniques essential for
responsive musical interfaces.
================================================================================
*/
Midi midi;                             // Bela MIDI interface object
MidiHandler midiHandler(44100.0f, 1.0f); // 1ms processing delay for stability

/*
VELOCITY PARSING CONFIGURATION
Threshold-based velocity parsing distinguishes between note-on and note-off
events based on velocity values. Threshold of 64 provides good separation
while accommodating various MIDI controller characteristics.
*/
VelocityParser velocityParser(64);     // Velocity threshold for note detection

/*
PORTAMENTO PROCESSING SYSTEM
Detects and processes portamento (glide) between notes. Essential for
expressive performance and authentic analog synthesizer emulation.
*/
PortamentoFilter portamentoFilter;     // Portamento event detection
PortamentoPlayer portamentoPlayer(44100.0f, 100.0f); // 100ms default glide time

/*
================================================================================
MODULATION AND ENVELOPE SYSTEMS
================================================================================
ACADEMIC SIGNIFICANCE:
Implements the complete modulation architecture typical of analog synthesizers.
Each component serves specific musical and technical functions in creating
expressive, dynamic sound synthesis.
================================================================================
*/
ADSR envelope;                         // Primary amplitude envelope

/*
FILTER MODULATION SUBSYSTEM
Specialized envelope generator for filter cutoff modulation with velocity
sensitivity and keyboard tracking. Crucial for authentic analog synthesizer
behavior and musical expressiveness.
*/
MoogFilterEnvelope filterEnv(44100.0f); // Filter-specific envelope generator
KeyFollow keyFollow(0.01f);            // Keyboard tracking coefficient
ResonanceRamp resonanceRamp(44100.0f, 50.0f); // Smooth resonance transitions

/*
================================================================================
SIMD-OPTIMIZED FILTER SYSTEM
================================================================================
ACADEMIC SIGNIFICANCE:
Core SIMD-optimized Moog filter implementation using ARM NEON instructions.
Represents cutting-edge approach to real-time audio filter processing in
embedded systems.
================================================================================
*/
MoogFilter moogFilter;                 // NEON SIMD-optimized filter instance

/*
BLOCK PROCESSING BUFFERS
Separate input/output buffers enable efficient SIMD processing by organizing
data in memory-aligned blocks suitable for parallel operations.
*/
float* inputBuffer = nullptr;          // Block processing input buffer
float* outputBuffer = nullptr;         // Block processing output buffer
int bufferSize = 0;                    // Current buffer size

/*
DYNAMIC PARAMETER STORAGE
Base cutoff frequency serves as reference for modulation calculations.
Allows complex modulation routing while maintaining parameter coherence.
*/
float baseCutoffFrequency = 1000.0f;   // Base cutoff for modulation calculations

/*
================================================================================
FUNCTION: setup()
================================================================================
ACADEMIC PURPOSE:
Comprehensive system initialization covering MIDI configuration, filter setup,
buffer allocation, and envelope parameter configuration. Demonstrates best
practices for real-time audio system initialization.

COMPUTATIONAL COMPLEXITY: O(1)
MEMORY ALLOCATION: Linear with buffer size
REAL-TIME SAFETY: Safe (initialization phase only)
================================================================================
*/
bool setup(BelaContext *context, void *userData) {
	
    /*
    MIDI SYSTEM INITIALIZATION
    Configures MIDI input from Ableton Live via USB interface.
    Hardware identifier "hw:0,0" specifies first audio interface, first device.
    */
    midi.readFrom("hw:0,0");           // Configure MIDI input source
    midi.enableParser(true);           // Enable real-time MIDI message parsing
	
	// SAMPLE RATE ACQUISITION
    // Critical for all time-based calculations and coefficient computation
    float sampleRate = context->audioSampleRate;
    
    /*
    SIMD FILTER INITIALIZATION
    Initializes NEON-optimized Moog filter with system sample rate.
    Sample rate parameter essential for accurate coefficient calculation.
    */
    moogFilter = MoogFilter(sampleRate);
    
    /*
    BLOCK PROCESSING BUFFER ALLOCATION
    Allocates separate input/output buffers for efficient SIMD processing.
    Buffer size matches audio callback size for optimal performance.
    */
    bufferSize = context->audioFrames;
    inputBuffer = new float[bufferSize];   // SIMD input buffer allocation
    outputBuffer = new float[bufferSize];  // SIMD output buffer allocation
    
    /*
    FILTER ENVELOPE CONFIGURATION
    ADSR parameters derived from professional synthesizer analysis:
    - Attack: 1ms (immediate response)
    - Decay: 100ms (moderate decay time)
    - Sustain: 75% (sustained level)
    - Release: 200ms (natural release)
    - Envelope Depth: 48 semitones (4 octaves modulation range)
    */
    filterEnv.setADSR(0.001f, 0.1f, 0.75f, 0.2f); // Professional ADSR settings
	filterEnv.setEnvDepth(48.0f);                  // 4-octave modulation range
	resonanceRamp.setTarget(0.5f);                 // Initial moderate resonance

    /*
    AMPLITUDE ENVELOPE CONFIGURATION
    Configured for musical responsiveness with professional timing characteristics:
    - Fast attack for percussive response
    - Quick decay for punch
    - Moderate sustain for held notes
    - Musical release time
    */
	envelope.reset();                              // Clear envelope state
    envelope.setAttackRate(0.01f * sampleRate);    // 10ms attack time
    envelope.setDecayRate(0.012f * sampleRate);    // 12ms decay time
    envelope.setReleaseRate(0.25f * sampleRate);   // 250ms release time
    envelope.setSustainLevel(0.65f);               // 65% sustain level

    /*
    ENVELOPE CURVE SHAPING
    Target ratios control envelope curvature for natural musical response:
    - Attack ratio: 0.3 (moderate exponential curve)
    - Decay/Release ratio: 0.0001 (exponential decay characteristic)
    */
    envelope.setTargetRatioA(0.3f);                // Attack curve shaping
    envelope.setTargetRatioDR(0.0001f);            // Decay/Release curve shaping

    return true;  // Successful initialization
}

/*
================================================================================
FUNCTION: render()
================================================================================
ACADEMIC PURPOSE:
Core audio processing function implementing complete synthesizer voice with
SIMD-optimized filtering. Demonstrates integration of complex modulation systems
with high-performance audio processing techniques.

PROCESSING ARCHITECTURE:
1. MIDI message processing and timing analysis
2. Modulation parameter calculation
3. Audio signal generation
4. Block-based SIMD filtering
5. Audio output distribution

COMPUTATIONAL COMPLEXITY: O(n) + O(m) where n=audioFrames, m=MIDI messages
REAL-TIME CONSTRAINTS: Must complete within buffer period (typically <10ms)
================================================================================
*/
void render(BelaContext *context, void *userData) {
    /*
    TIMING REFERENCE CALCULATION
    Converts audio frame elapsed count to milliseconds for MIDI timing analysis.
    Essential for accurate event scheduling and timing-sensitive processing.
    */
    float currentTimeMs = context->audioFramesElapsed / context->audioSampleRate * 1000.0f;

    /*
    ============================================================================
    MIDI MESSAGE PROCESSING LOOP
    ============================================================================
    Processes all available MIDI messages in current callback.
    Non-blocking design ensures real-time performance while handling
    multiple simultaneous MIDI events.
    */
    while (midi.getParser()->numAvailableMessages() > 0) {
        MidiChannelMessage message = midi.getParser()->getNextChannelMessage();
        
        /*
        NOTE MESSAGE PROCESSING
        Handles note-on and note-off events with velocity information.
        Routes messages through timing analysis system for accurate scheduling.
        */
        if (message.getType() == kmmNoteOn || message.getType() == kmmNoteOff) {
            int note = message.getDataByte(0);         // MIDI note number [0-127]
            int velocity = message.getDataByte(1);     // Velocity value [0-127]
            midiHandler.processMidiMessage(note, velocity, currentTimeMs);
        }
        /*
        CONTINUOUS CONTROLLER (CC) MESSAGE PROCESSING
        Handles real-time parameter modulation via MIDI controllers.
        Enables dynamic control of synthesis parameters during performance.
        */
        else if (message.getType() == kmmControlChange) {
            int controller = message.getDataByte(0);   // CC number [0-127]
            int value = message.getDataByte(1);        // CC value [0-127]
            
            /*
            CUTOFF FREQUENCY CONTROL (CC1 - Modulation Wheel)
            Maps MIDI controller range [0-127] to logarithmic frequency range.
            Logarithmic mapping provides musically natural frequency response.
            Range: 20Hz to 18kHz (full audio spectrum coverage)
            */
            if (controller == 1) {
                // Logarithmic frequency mapping: f = 20 * (900^(CC/127))
                baseCutoffFrequency = 20.0f * powf(900.0f, value / 127.0f);
            }
            /*
            RESONANCE CONTROL (CC11 - Expression Controller)
            Linear mapping from MIDI range to normalized resonance parameter.
            Expression controller provides smooth, continuous modulation.
            */
            else if (controller == 11) {
                float resonanceValue = value / 127.0f; // Normalize to [0,1]
                resonanceRamp.setTarget(resonanceValue);
            }
        }
    }

    /*
    DELAYED MIDI MESSAGE PROCESSING
    Updates internal timing system and processes time-delayed MIDI events.
    Essential for accurate timing and synchronization with external sequencers.
    */
    midiHandler.update(currentTimeMs);

    /*
    ============================================================================
    ADVANCED MIDI EVENT PROCESSING
    ============================================================================
    Processes delayed MIDI messages with sophisticated analysis including
    velocity parsing, portamento detection, and modulation routing.
    */
	while (midiHandler.hasDelayedMessage()) {
		MidiNoteMessage delayedMsg = midiHandler.popDelayedMessage();
        
        /*
        VELOCITY-SENSITIVE NOTE DETECTION
        Analyzes velocity to distinguish true note-on events from note-off.
        Accommodates various MIDI controller behaviors and performance styles.
        */
		bool noteOn = velocityParser.isNoteOn(delayedMsg.velocity);
	
        /*
        PORTAMENTO ANALYSIS
        Determines whether current note should use portamento based on:
        - Previous note status
        - Timing between notes
        - Note transition characteristics
        */
		bool portamento = portamentoFilter.checkPortamento(
			delayedMsg.noteNumber, noteOn, delayedMsg.timestamp
		);
		
        // Normalize velocity for modulation calculations
		float velocityScaled = delayedMsg.velocity / 127.0f;
	
        /*
        NOTE-ON EVENT PROCESSING
        Triggers all synthesis components for new note events:
        - Portamento player for pitch transitions
        - Amplitude envelope for dynamics
        - Filter envelope for timbral evolution
        */
		if (noteOn) {
			portamentoPlayer.noteOn(delayedMsg.noteNumber, portamento);
			envelope.gate(1);                          // Trigger amplitude envelope
			filterEnv.gate(1, velocityScaled);         // Trigger filter envelope with velocity
		} 
        /*
        NOTE-OFF EVENT PROCESSING
        Initiates release phase for all envelope generators while maintaining
        smooth parameter transitions.
        */
        else {
			portamentoPlayer.noteOff();
			envelope.gate(0);                          // Release amplitude envelope
			filterEnv.gate(0, 0.0f);                   // Release filter envelope
		}
		
        /*
        RESONANCE MODULATION EXAMPLE
        Demonstrates additional modulation routing possibilities.
        Can be customized for various expressive control schemes.
        */
    	resonanceRamp.setTarget(0.7f);                 // Example resonance modulation
	}

    /*
    ============================================================================
    AUDIO SIGNAL GENERATION AND MODULATION
    ============================================================================
    Generates audio samples with complete modulation routing including
    envelopes, portamento, and filter parameter calculation.
    */
	for(unsigned int n = 0; n < context->audioFrames; n++) {
		
        /*
        MODULATION PARAMETER CALCULATION
        Calculates all modulation sources for current sample:
        - Amplitude envelope for volume control
        - Portamento for smooth pitch transitions  
        - Key follow for keyboard tracking
        - Filter envelope for cutoff modulation
        - Resonance ramping for smooth parameter changes
        */
		float envValue = envelope.process();           // Amplitude envelope
		float freq = portamentoPlayer.process();       // Current note frequency
		float keyFollowValue = keyFollow.process(portamentoPlayer.getCurrentNote());
    	float filterCutoff = filterEnv.process(baseCutoffFrequency, keyFollowValue);
    	float resonance = resonanceRamp.process();     // Smooth resonance value
		
        /*
        REAL-TIME FILTER PARAMETER UPDATES
        Updates SIMD filter parameters for current sample.
        SIMD filters can handle per-sample parameter changes efficiently.
        */
		moogFilter.setCutoff(filterCutoff);
		moogFilter.setResonance(resonance);
		
        /*
        OSCILLATOR SIGNAL GENERATION
        Generates primary audio signal using phase accumulation oscillator.
        Includes envelope state management for proper voice allocation.
        */
		float oscillatorOut = 0.0f;
		
        /*
        ENVELOPE-CONTROLLED OSCILLATOR
        Only generates audio signal when envelope is active, preventing
        unnecessary computation and maintaining clean voice allocation.
        */
		if(envelope.getState() != env_idle) {
			oscillatorOut = sinf(oscillatorPhase);     // Generate sine wave
			oscillatorPhase += twoPi * freq / context->audioSampleRate;
			if(oscillatorPhase >= twoPi)
				oscillatorPhase -= twoPi;
			
			oscillatorOut *= envValue;                 // Apply amplitude envelope
		} else {
            /*
            IDLE STATE MANAGEMENT
            Resets oscillator phase when envelope is idle to ensure
            consistent behavior for subsequent note events.
            */
			oscillatorPhase = 0.0f;
			oscillatorOut = 0.0f;
		}
	
        /*
        AUDIO LEVEL MANAGEMENT
        Applies comfortable listening level to prevent hearing damage
        and maintain professional audio standards.
        */
		oscillatorOut *= 0.5f;                         // Comfortable listening level
		
        /*
        BLOCK PROCESSING BUFFER FILLING
        Stores processed audio sample in input buffer for subsequent
        SIMD filtering operation.
        */
		inputBuffer[n] = oscillatorOut;
	}
	
    /*
    ============================================================================
    SIMD BLOCK PROCESSING
    ============================================================================
    Processes entire audio buffer through NEON-optimized Moog filter.
    Block processing enables SIMD optimization and cache-efficient operation.
    */
	moogFilter.processBlockSIMD(inputBuffer, outputBuffer, context->audioFrames);
	
    /*
    ============================================================================
    AUDIO OUTPUT DISTRIBUTION
    ============================================================================
    Distributes processed audio to stereo outputs with proper channel mapping.
    */
	for(unsigned int n = 0; n < context->audioFrames; n++) {
    	audioWrite(context, n, 0, outputBuffer[n]);    // Left channel
    	audioWrite(context, n, 1, outputBuffer[n]);    // Right channel
	}
}

/*
================================================================================
FUNCTION: cleanup()
================================================================================
ACADEMIC PURPOSE:
Resource deallocation for SIMD processing buffers. Critical for embedded
systems where memory leaks can cause system instability over extended operation.
================================================================================
*/
void cleanup(BelaContext *context, void *userData) {
    /*
    BUFFER MEMORY DEALLOCATION
    Releases dynamically allocated processing buffers.
    Essential for preventing memory leaks in embedded audio systems.
    */
    delete[] inputBuffer;                              // Release input buffer
    delete[] outputBuffer;                             // Release output buffer
}

/*
================================================================================
PERFORMANCE ANALYSIS AND RESEARCH IMPLICATIONS
================================================================================

SIMD OPTIMIZATION BENEFITS:
1. Theoretical 4x performance improvement through parallel processing
2. Reduced CPU load enabling additional synthesis voices
3. Lower audio latency through efficient processing
4. Better cache utilization through block processing

RESEARCH APPLICATIONS:
1. Performance benchmarking against scalar implementations
2. Power consumption analysis in embedded systems
3. Scalability studies for polyphonic synthesis
4. Real-time constraint analysis under various load conditions

MUSICAL IMPLICATIONS:
1. Enhanced expressiveness through complex modulation routing
2. Professional-grade velocity sensitivity and timing
3. Authentic analog synthesizer behavior emulation
4. Responsive real-time parameter control

ACADEMIC EXTENSIONS:
1. Compare SIMD efficiency across different processor architectures
2. Analyze perceptual differences between optimized and reference implementations
3. Study modulation system complexity vs. musical expressiveness
4. Investigate thermal behavior and power efficiency of SIMD processing

================================================================================
*/

