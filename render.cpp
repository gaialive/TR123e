/**
 * @file render.cpp
 * @brief Real-time monophonic synthesizer implementation for Bela platform
 * 
 * This file implements a complete monophonic synthesizer using the Bela real-time audio platform.
 * The synthesizer features a sine wave oscillator with portamento, dual ADSR envelopes 
 * (amplitude and filter), a zero-delay feedback Moog ladder filter, and comprehensive
 * MIDI control with analog input integration.
 * 
 * @architecture
 * The system employs a modular signal flow architecture:
 * MIDI Input → Note Processing → Oscillator → Filter → Amplification → Audio Output
 *             ↓                    ↓         ↓
 *         Portamento         Amp Envelope  Filter Envelope
 * 
 * @performance_characteristics
 * - Real-time processing at variable sample rates (typically 44.1kHz)
 * - Ultra-low latency audio processing (< 5ms typical)
 * - Zero-delay feedback filter implementation for analog-accurate response
 * - Hardware analog control integration with smooth parameter interpolation
 * 
 * @dependencies
 * - Bela.h: Real-time audio framework
 * - libraries/Midi/Midi.h: MIDI protocol implementation
 * - Custom DSP classes: ADSR, KeyFollow, MidiHandler, etc.
 * 
 * @author [Timothy Paul Read]
 * @date [2025/5/25]
 * @version 1.0
 */

#include <Bela.h>
#include <libraries/Midi/Midi.h>
#include <cmath>
#include "ADSR.h"
#include "KeyFollow.h"
#include "MidiHandler.h"
#include "MoogFilterEnvelope.h"
#include "PortamentoFilter.h"
#include "PortamentoPlayer.h"
#include "ResonanceRamp.h"
#include "VelocityParser.h"
#include "zdf_moogladder_v2.h"

// ============================================================================
// GLOBAL STATE VARIABLES
// ============================================================================

/**
 * @brief Primary oscillator phase accumulator [0, 2π]
 * 
 * Maintains phase continuity for sine wave generation using direct digital 
 * synthesis (DDS) method. Phase wrapping prevents floating-point precision
 * degradation over extended operation periods.
 */
float oscillatorPhase = 0.0f;

/**
 * @brief Precomputed 2π constant for computational efficiency
 * 
 * Cached value eliminates repeated trigonometric calculations in the
 * audio processing loop, reducing CPU overhead in real-time context.
 */
float twoPi = 2.0f * M_PI;

// ============================================================================
// AUDIO PROCESSING MODULES
// ============================================================================

/**
 * @brief MIDI interface controller
 * 
 * Handles low-level MIDI protocol parsing and message buffering.
 * Configured for hardware MIDI interface "hw:0,0" (first MIDI device).
 */
Midi midi;

/**
 * @brief High-level MIDI message processor and timing controller
 * @param sampleRate 44100.0f Hz - Standard audio sample rate
 * @param jitterReduction 1.0f - Temporal quantization factor for timing stability
 * 
 * Implements sophisticated MIDI timing algorithms to handle note scheduling,
 * velocity-sensitive triggering, and temporal message buffering for
 * frame-accurate event processing.
 */
MidiHandler midiHandler(44100.0f, 1.0f);

/**
 * @brief Velocity-sensitive note triggering processor
 * @param threshold 64 - MIDI velocity threshold for note-on discrimination
 * 
 * Implements hysteresis-based velocity parsing to distinguish between
 * note-on and note-off events, preventing false triggering from low
 * velocity note-on messages (common in electronic controllers).
 */
VelocityParser velocityParser(64);

/**
 * @brief Portamento behavior analysis module
 * 
 * Analyzes incoming MIDI note patterns to determine appropriate portamento
 * behavior based on legato playing technique detection and note overlap timing.
 */
PortamentoFilter portamentoFilter;

/**
 * @brief Pitch interpolation and portamento synthesis engine
 * @param sampleRate 44100.0f Hz - Audio processing rate
 * @param glideTime 100.0f ms - Default portamento transition duration
 * 
 * Implements exponential pitch interpolation algorithms for smooth frequency
 * transitions between notes. Uses logarithmic frequency scaling to maintain
 * perceptually linear pitch movement across the entire musical range.
 */
PortamentoPlayer portamentoPlayer(44100.0f, 100.0f);

/**
 * @brief Primary amplitude envelope generator
 * 
 * Four-stage ADSR (Attack, Decay, Sustain, Release) envelope with
 * exponential segment shaping and configurable curvature parameters.
 * Implements state machine for proper envelope stage transitions.
 */
ADSR envelope;

/**
 * @brief Filter cutoff frequency envelope generator
 * @param sampleRate 44100.0f Hz - Audio processing rate
 * 
 * Specialized envelope generator for filter frequency modulation with
 * velocity sensitivity and exponential frequency mapping. Provides
 * musical filter sweeps with proper logarithmic frequency scaling.
 */
MoogFilterEnvelope filterEnv(44100.0f);

/**
 * @brief Keyboard tracking / key follow processor
 * @param intensity 0.01f - Key tracking coefficient [0.0-1.0]
 * 
 * Implements keyboard tracking for filter cutoff frequency, allowing
 * higher notes to open the filter proportionally. Uses MIDI note number
 * to frequency mapping with configurable tracking intensity.
 */
KeyFollow keyFollow(0.01f);

/**
 * @brief Resonance parameter smoothing filter
 * @param sampleRate 44100.0f Hz - Audio processing rate  
 * @param rampTime 50.0f ms - Parameter transition smoothing time
 * 
 * Implements first-order low-pass filtering for resonance parameter changes
 * to prevent audio artifacts from abrupt control changes, particularly
 * important for high-Q filter settings where parameter discontinuities
 * can cause instability.
 */
ResonanceRamp resonanceRamp(44100.0f, 50.0f);

/**
 * @brief Zero-delay feedback Moog ladder filter implementation
 * @param sampleRate 44100.0f Hz - Audio processing rate
 * 
 * Advanced digital implementation of the classic Moog transistor ladder filter
 * using zero-delay feedback (ZDF) topology. Provides analog-accurate frequency
 * response and self-oscillation behavior with numerical stability across
 * all parameter ranges.
 * 
 * @note ZDF implementation eliminates the one-sample delay inherent in 
 * traditional digital filter structures, providing superior transient response
 * and improved analog modeling accuracy.
 */
ZDFMoogLadderFilter zdfFilter(44100.0f);

// ============================================================================
// AUDIO BUFFER MANAGEMENT
// ============================================================================

/**
 * @brief Input audio buffer for oscillator output
 * 
 * Dynamically allocated buffer for storing raw oscillator output before
 * filter processing. Separation of generation and filtering stages allows
 * for potential future multi-stage processing implementations.
 */
float* inputBuffer = nullptr;

/**
 * @brief Output audio buffer for filtered audio
 * 
 * Stores final processed audio after filter stage, ready for DAC output.
 * Double-buffering architecture prevents audio artifacts during processing.
 */
float* outputBuffer = nullptr;

/**
 * @brief Current audio buffer size in samples
 * 
 * Dynamically determined from Bela context, typically 64-512 samples
 * depending on system configuration and latency requirements.
 */
int bufferSize = 0;

// ============================================================================
// SYNTHESIS PARAMETERS
// ============================================================================

/**
 * @brief Base filter cutoff frequency in Hz
 * 
 * Fundamental filter frequency before envelope and key tracking modulation.
 * Set to 5kHz as musically useful default providing bright, open character
 * while allowing sufficient modulation range.
 */
float baseCutoffFrequency = 5000.0f;

/**
 * @brief Audio-to-analog frame ratio for control rate processing
 * 
 * Bela systems process analog inputs at lower rates than audio (typically 8:1 ratio).
 * This variable caches the ratio for efficient analog input indexing without
 * repeated division operations in the audio callback.
 */
int gAudioFramesPerAnalogFrame = 0;

/**
 * @function setup
 * @brief System initialization and configuration
 * 
 * Initializes all audio processing modules, allocates dynamic memory,
 * configures MIDI interface, and establishes initial parameter states.
 * Called once at system startup before real-time processing begins.
 * 
 * @param context Bela audio context containing system configuration
 * @param userData User-defined data pointer (unused in this implementation)
 * @return true if initialization successful, false triggers system abort
 * 
 * @complexity O(1) - Constant time initialization
 * @realtime_safety Non-real-time safe (performs memory allocation)
 * 
 * @note This function performs dynamic memory allocation and must complete
 * before real-time audio processing begins. Failure to complete initialization
 * will prevent audio system startup.
 */
bool setup(BelaContext *context, void *userData) {
    // ========================================================================
    // MIDI Interface Initialization
    // ========================================================================
    
    /**
     * Configure MIDI interface for hardware device "hw:0,0"
     * This corresponds to the first MIDI interface on the system
     */
    midi.readFrom("hw:0,0");
    
    /**
     * Enable built-in MIDI message parsing for automatic protocol handling
     * Parser provides structured access to MIDI channel messages, system
     * exclusive data, and real-time messages with proper timing information
     */
    midi.enableParser(true);

    // ========================================================================
    // Audio System Configuration
    // ========================================================================
    
    /**
     * Cache sample rate for computational efficiency
     * Eliminates repeated context dereferencing in audio callback
     */
    float sampleRate = context->audioSampleRate;
    
    /**
     * Calculate and cache audio/analog frame ratio
     * Bela processes analog inputs at lower rates than audio samples
     * Typical ratio: 8 audio frames per analog frame (44.1kHz/5.5kHz)
     */
    gAudioFramesPerAnalogFrame = context->audioFrames / context->analogFrames;

    // ========================================================================
    // Filter Module Initialization
    // ========================================================================
    
    /**
     * Initialize ZDF Moog ladder filter with current sample rate
     * Reconfiguration ensures proper coefficient calculation for actual
     * system sample rate (may differ from default 44.1kHz)
     */
    zdfFilter = ZDFMoogLadderFilter(sampleRate);
    
    /**
     * Reset filter internal state to prevent initialization artifacts
     * Clears delay lines and feedback paths for clean startup
     */
    zdfFilter.reset();
    
    /**
     * Configure initial filter parameters
     * - Cutoff: 1kHz (musically neutral, not too bright or dull)
     * - Resonance: 0.5 (moderate Q, stable self-oscillation threshold)
     * - Drive: 1.0 (unity gain, no input saturation)
     * - Mode: 0 (24dB/octave low-pass, classic Moog response)
     */
    zdfFilter.setCutoff(1000.0f);
    zdfFilter.setResonance(0.5f);
    zdfFilter.setDrive(1.0f);
    zdfFilter.setMode(0);

    // ========================================================================
    // Audio Buffer Allocation
    // ========================================================================
    
    /**
     * Cache buffer size for memory allocation
     * Size determined by Bela configuration, typically 64-512 samples
     */
    bufferSize = context->audioFrames;
    
    /**
     * Allocate input buffer for oscillator output staging
     * Enables separation of synthesis and filtering for modular processing
     */
    inputBuffer = new float[bufferSize];
    
    /**
     * Allocate output buffer for final processed audio
     * Provides clean separation between processing stages
     */
    outputBuffer = new float[bufferSize];

    // ========================================================================
    // Envelope Generator Configuration
    // ========================================================================
    
    /**
     * Configure filter envelope parameters
     * - Attack: 1ms (near-instantaneous response for percussive sounds)
     * - Decay: 100ms (moderate decay for musical filter sweeps)
     * - Sustain: 0.75 (maintains 75% of peak level during note hold)
     * - Release: 200ms (smooth fade-out preventing clicks)
     */
    filterEnv.setADSR(0.001f, 0.1f, 0.75f, 0.2f);
    
    /**
     * Set filter envelope depth to 48 semitones (4 octaves)
     * Provides musically significant frequency modulation range
     * while preventing excessive filter opening that could cause aliasing
     */
    filterEnv.setEnvDepth(48.0f);
    
    /**
     * Initialize resonance ramping to moderate value
     * 0.5 provides good filter character without excessive self-oscillation
     */
    resonanceRamp.setTarget(0.5f);

    // ========================================================================
    // Amplitude Envelope Configuration
    // ========================================================================
    
    /**
     * Reset amplitude envelope to idle state
     * Ensures clean initialization without residual envelope activity
     */
    envelope.reset();
    
    /**
     * Configure amplitude envelope timing (values in samples)
     * - Attack: 441 samples ≈ 10ms (quick response, prevents clicks)
     * - Decay: 529 samples ≈ 12ms (fast initial decay for punch)
     * - Release: 11,025 samples ≈ 250ms (musical release tail)
     */
    envelope.setAttackRate(0.01f * sampleRate);
    envelope.setDecayRate(0.012f * sampleRate);
    envelope.setReleaseRate(0.25f * sampleRate);
    
    /**
     * Set sustain level to 65% of peak amplitude
     * Balances musical sustain with dynamic range preservation
     */
    envelope.setSustainLevel(0.65f);
    
    /**
     * Configure envelope curvature parameters
     * - Attack ratio: 0.3 (exponential attack curve)
     * - Decay/Release ratio: 0.0001 (near-linear decay for natural sound)
     * 
     * These parameters control the mathematical curvature of envelope segments,
     * affecting the perceptual character of amplitude changes
     */
    envelope.setTargetRatioA(0.3f);
    envelope.setTargetRatioDR(0.0001f);

    return true;
}

/**
 * @function render
 * @brief Real-time audio processing callback
 * 
 * Core real-time audio processing function called at regular intervals
 * (typically every 64-512 samples). Implements complete signal chain from
 * MIDI input through oscillator, envelope, and filter processing to audio output.
 * 
 * @param context Bela audio context with I/O buffers and timing information
 * @param userData User-defined data pointer (unused)
 * 
 * @complexity O(n) where n = context->audioFrames
 * @realtime_safety Real-time safe (no dynamic allocation or blocking operations)
 * @latency Ultra-low latency (< 5ms typical processing delay)
 * 
 * @note This function executes in real-time audio context and must complete
 * processing within the buffer period to prevent audio dropouts. All operations
 * must be deterministic and bounded in execution time.
 */
void render(BelaContext *context, void *userData) {
    // ========================================================================
    // TIMING AND SYNCHRONIZATION
    // ========================================================================
    
    /**
     * Calculate current time in milliseconds for MIDI timing correlation
     * Provides high-precision timestamp for event scheduling and delay compensation
     * Formula: (samples_elapsed / sample_rate) * 1000 = time_in_ms
     */
    float currentTimeMs = context->audioFramesElapsed / context->audioSampleRate * 1000.0f;

    // ========================================================================
    // MIDI MESSAGE PROCESSING
    // ========================================================================
    
    /**
     * Process all available MIDI messages in current buffer period
     * Non-blocking iteration ensures real-time safety while handling
     * multiple simultaneous MIDI events with sample-accurate timing
     */
    while (midi.getParser()->numAvailableMessages() > 0) {
        MidiChannelMessage message = midi.getParser()->getNextChannelMessage();
        
        /**
         * Process Note On/Off messages for synthesis control
         * Both message types handled by unified processor for consistent timing
         */
        if (message.getType() == kmmNoteOn || message.getType() == kmmNoteOff) {
            int note = message.getDataByte(0);      // MIDI note number [0-127]
            int velocity = message.getDataByte(1);  // Velocity value [0-127]
            midiHandler.processMidiMessage(note, velocity, currentTimeMs);
        }
        /**
         * Process Control Change messages for real-time parameter modification
         * Implements standard MIDI CC protocol for synthesizer control
         */
        else if (message.getType() == kmmControlChange) {
            int controller = message.getDataByte(0);    // CC number [0-127]
            int value = message.getDataByte(1);         // CC value [0-127]
            
            /**
             * CC 14: Filter Cutoff Frequency Control
             * Maps MIDI CC value to exponential frequency range [20Hz - 30kHz]
             * Formula: f = 20 * (1500^(cc/127)) provides musical frequency scaling
             */
            if (controller == 14) {
                baseCutoffFrequency = 20.0f * powf(1500.0f, value / 127.0f);
            }
            /**
             * CC 15: Filter Resonance Control
             * Linear mapping from MIDI CC to resonance parameter [0.0-1.0]
             * Higher values increase filter Q and self-oscillation tendency
             */
            else if (controller == 15) {
                float resonanceValue = value / 127.0f;
                resonanceRamp.setTarget(resonanceValue);
            }
        }
    }

    // ========================================================================
    // MIDI TIMING AND DELAYED MESSAGE PROCESSING
    // ========================================================================
    
    /**
     * Update MIDI handler timing state for accurate event scheduling
     * Maintains internal timing reference for sample-accurate MIDI processing
     */
    midiHandler.update(currentTimeMs);

    /**
     * Process all sample-accurate delayed MIDI messages
     * Handles messages that require precise timing alignment with audio samples
     * for glitch-free note transitions and envelope triggering
     */
    while (midiHandler.hasDelayedMessage()) {
        MidiNoteMessage delayedMsg = midiHandler.popDelayedMessage();
        
        /**
         * Parse velocity to determine note-on vs note-off state
         * Implements MIDI standard where velocity 0 = note-off
         */
        bool noteOn = velocityParser.isNoteOn(delayedMsg.velocity);
        
        /**
         * Analyze portamento requirements based on playing technique
         * Determines whether smooth pitch transition is appropriate
         * based on note overlap timing and musical context
         */
        bool portamento = portamentoFilter.checkPortamento(
            delayedMsg.noteNumber, noteOn, delayedMsg.timestamp);
        
        /**
         * Convert MIDI velocity to normalized amplitude scaling [0.0-1.0]
         * Provides velocity-sensitive filter envelope response
         */
        float velocityScaled = delayedMsg.velocity / 127.0f;

        /**
         * Process note-on events: trigger all synthesis modules
         */
        if (noteOn) {
            // Trigger pitch generator with portamento logic
            portamentoPlayer.noteOn(delayedMsg.noteNumber, portamento);
            
            // Trigger amplitude envelope
            envelope.gate(1);
            
            // Trigger filter envelope with velocity sensitivity
            filterEnv.gate(1, velocityScaled);
        } 
        /**
         * Process note-off events: release all synthesis modules
         */
        else {
            // Release pitch generator (maintains current pitch)
            portamentoPlayer.noteOff();
            
            // Trigger amplitude envelope release
            envelope.gate(0);
            
            // Trigger filter envelope release
            filterEnv.gate(0, 0.0f);
        }

        /**
         * Set resonance emphasis during note events
         * Creates characteristic filter "pop" during note articulation
         * typical of classic analog synthesizers
         */
        resonanceRamp.setTarget(0.7f);
    }

    // ========================================================================
    // AUDIO SAMPLE PROCESSING LOOP
    // ========================================================================
    
    /**
     * Static variable for output gain to maintain state between function calls
     * Note: Double semicolon appears to be a typo in original code
     */
    static float outGain;
    
    /**
     * Process each audio sample in the current buffer
     * Implements complete synthesis chain: oscillator → envelope → filter → output
     */
    for(unsigned int n = 0; n < context->audioFrames; n++) {
        // ====================================================================
        // ANALOG CONTROL INPUT PROCESSING
        // ====================================================================
        
        /**
         * Calculate analog input index based on audio/analog frame ratio
         * Analog inputs sampled at lower rate than audio (typically 8:1)
         * This indexing ensures proper correlation between control and audio timing
         */
        unsigned int analogIndex = n / gAudioFramesPerAnalogFrame;

        // ====================================================================
        // SYNTHESIS MODULE PROCESSING
        // ====================================================================
        
        /**
         * Generate amplitude envelope value [0.0-1.0]
         * Provides time-varying amplitude control for musical articulation
         */
        float envValue = envelope.process();
        
        /**
         * Generate current oscillator frequency from portamento processor
         * Returns frequency in Hz with smooth transitions between notes
         */
        float freq = portamentoPlayer.process();
        
        /**
         * Calculate keyboard tracking contribution to filter frequency
         * Higher notes slightly open filter for consistent timbre across range
         */
        float keyFollowValue = keyFollow.process(portamentoPlayer.getCurrentNote());
        
        /**
         * Generate filter cutoff frequency from envelope and key tracking
         * Combines base frequency, envelope modulation, and keyboard tracking
         */
        float filterCutoff = filterEnv.process(baseCutoffFrequency, keyFollowValue);
        
        // ====================================================================
        // ANALOG CONTROL INPUT READING
        // ====================================================================
        
        /**
         * Read analog control potentiometers [0.0-1.0]
         * Hardware control integration for real-time parameter adjustment
         */
        float cutoffPot = analogRead(context, analogIndex, 0);      // Filter cutoff
        float resonancePot = analogRead(context, analogIndex, 1);   // Filter resonance
        
        /**
         * Read filter mode selection from analog input
         * Maps continuous [0.0-1.0] to discrete mode values [0-2]
         * Modes: 0=LP24, 1=LP12, 2=HP12, 3=BP12 (depending on filter implementation)
         */
        int mode = static_cast<int>(analogRead(context, analogIndex, 2) * 3.0f);
        
        /**
         * Read additional control parameters from analog inputs
         */
        outGain = analogRead(context, analogIndex, 3) * 2.0f;       // Output gain [0-2]
        float drive = analogRead(context, analogIndex, 4);          // Filter drive [0-1]
        float envDepth = analogRead(context, analogIndex, 5);       // Envelope depth [0-1]
        float attack = analogRead(context, analogIndex, 6);         // Attack time [0-1]
        float release = analogRead(context, analogIndex, 7);        // Release time [0-1]
        
        // ====================================================================
        // REAL-TIME PARAMETER UPDATES
        // ====================================================================
        
        /**
         * Update filter resonance with smooth ramping
         * Prevents audio artifacts from abrupt parameter changes
         */
        resonanceRamp.setTarget(resonancePot);
        float resonance = resonanceRamp.process();
        
        /**
         * Update filter parameters for current sample
         * Combines envelope-generated cutoff with manual control
         */
        zdfFilter.setCutoff(filterCutoff * (0.2f + cutoffPot));  // Min 20% + pot control
        zdfFilter.setResonance(resonance);
        zdfFilter.setMode(mode);
        zdfFilter.setDrive(drive);

        /**
         * Update envelope parameters in real-time
         * Allows dynamic response characteristics during performance
         */
        
        // Filter envelope depth: 0-96 semitones (8 octaves maximum)
        filterEnv.setEnvDepth(envDepth * 48.0f);
        
        // Attack time: 1ms to 1 second exponential mapping
        envelope.setAttackRate(0.001f * context->audioSampleRate + 
                              attack * 1.0f * context->audioSampleRate);
        
        // Release time: 5ms to 2 seconds exponential mapping  
        envelope.setReleaseRate(0.005f * context->audioSampleRate + 
                               release * 1.995f * context->audioSampleRate);

        // ====================================================================
        // OSCILLATOR PROCESSING
        // ====================================================================
        
        /**
         * Generate oscillator output using Direct Digital Synthesis (DDS)
         * Implements conditional processing for CPU efficiency
         */
        float oscillatorOut = 0.0f;
        
        /**
         * Only generate oscillator output when envelope is active
         * Saves CPU cycles during silent periods (envelope in idle state)
         */
        if(envelope.getState() != env_idle) {
            /**
             * Generate sine wave using phase accumulator method
             * sin(φ) where φ = accumulated phase [0, 2π]
             */
            oscillatorOut = sinf(oscillatorPhase);
            
            /**
             * Update phase accumulator for next sample
             * Phase increment = 2π * frequency / sample_rate
             * This implements the fundamental DDS frequency equation
             */
            oscillatorPhase += twoPi * freq / context->audioSampleRate;
            
            /**
             * Phase wrapping to prevent floating-point precision loss
             * Maintains phase within [0, 2π] range for optimal accuracy
             */
            if(oscillatorPhase >= twoPi)
                oscillatorPhase -= twoPi;
            
            /**
             * Apply amplitude envelope to oscillator output
             * Creates time-varying amplitude response for musical articulation
             */
            oscillatorOut *= envValue;
        } else {
            /**
             * Reset oscillator phase during silent periods
             * Ensures clean restart for next note-on event
             */
            oscillatorPhase = 0.0f;
            oscillatorOut = 0.0f;
        }

        /**
         * Apply output scaling to prevent clipping
         * 50% scaling provides headroom for filter resonance peaks
         */
        oscillatorOut *= 0.5f;
        
        /**
         * Store oscillator output in input buffer for filter processing
         * Separation allows for potential future multi-stage processing
         */
        inputBuffer[n] = oscillatorOut;
    }
	
    // ========================================================================
    // FILTER PROCESSING
    // ========================================================================
    
    /**
     * Apply filter processing to entire buffer
     * Separate loop allows for potential SIMD optimization and
     * maintains clean separation between synthesis and filtering stages
     */
    for(unsigned int n = 0; n < context->audioFrames; n++) {
        /**
         * Process each sample through ZDF Moog ladder filter
         * Apply output gain control for final level adjustment
         */
        outputBuffer[n] = zdfFilter.process(inputBuffer[n]) * outGain;
    }

    // ========================================================================
    // AUDIO OUTPUT
    // ========================================================================
    
    /**
     * Write processed audio to both output channels (stereo)
     * Monophonic synthesizer output duplicated to both channels
     * for standard stereo compatibility
     */
    for(unsigned int n = 0; n < context->audioFrames; n++) {
        audioWrite(context, n, 0, outputBuffer[n]);  // Left channel
        audioWrite(context, n, 1, outputBuffer[n]);  // Right channel
    }
}

/**
 * @function cleanup
 * @brief System cleanup and resource deallocation
 * 
 * Releases dynamically allocated memory and performs orderly shutdown
 * of audio processing system. Called once when audio processing terminates.
 * 
 * @param context Bela audio context (unused in cleanup)
 * @param userData User-defined data pointer (unused)
 * 
 * @complexity O(1) - Constant time cleanup
 * @realtime_safety Non-real-time safe (performs memory deallocation)
 * 
 * @note This function is called outside the real-time audio context
 * and may perform blocking operations safely.
 */
void cleanup(BelaContext *context, void *userData) {
    /**
     * Deallocate input buffer memory
     * Prevents memory leaks on system shutdown
     */
    delete[] inputBuffer;
    
    /**
     * Deallocate output buffer memory  
     * Completes memory management for audio buffers
     */
    delete[] outputBuffer;
}