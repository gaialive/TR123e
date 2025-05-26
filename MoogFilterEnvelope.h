/**
 * @file MoogFilterEnvelope.h
 * @brief Specialized envelope generator for filter frequency modulation in Moog-style synthesis
 * 
 * This module implements a dedicated envelope generator optimized for filter cutoff frequency
 * modulation in classic analog synthesizer architectures. The system combines ADSR envelope
 * generation with velocity sensitivity, envelope depth control, and keyboard tracking integration
 * to provide musically expressive filter frequency sweeps characteristic of vintage Moog synthesizers.
 * 
 * @historical_context
 * The Moog filter envelope represents a fundamental component of the classic analog synthesizer
 * architecture pioneered by Robert Moog in the 1960s. This design pattern became the template
 * for virtually all subsequent subtractive synthesizers, establishing the paradigm of separate
 * amplitude and filter envelopes for independent control of loudness and timbre evolution.
 * 
 * @synthesis_theory
 * Filter envelope modulation serves multiple musical functions:
 * 
 * 1. **Timbral Animation**: Time-varying filter sweeps create evolving harmonic content
 * 2. **Articulation Control**: Attack and decay phases shape note beginnings and transitions
 * 3. **Velocity Response**: Dynamic filter opening provides expressive velocity sensitivity
 * 4. **Musical Phrasing**: Sustained filter levels maintain consistent timbre during note holds
 * 
 * @technical_architecture
 * The implementation employs a wrapper pattern around a generic ADSR envelope generator,
 * adding filter-specific functionality:
 * 
 * - **Envelope Depth Scaling**: Configurable modulation intensity for artistic control
 * - **Additive Frequency Mapping**: Linear frequency addition for predictable filter behavior
 * - **Keyboard Tracking Integration**: Seamless combination with key-follow algorithms
 * - **Velocity Sensitivity**: Optional velocity-to-envelope-intensity mapping
 * 
 * @mathematical_model
 * Filter frequency calculation: f_filter = f_base + f_keytrack + (envelope × depth)
 * 
 * Where:
 * - f_base: Base cutoff frequency setting
 * - f_keytrack: Keyboard tracking contribution
 * - envelope: ADSR envelope output [0.0-1.0]
 * - depth: Envelope depth parameter (typically in semitones or Hz)
 * 
 * @performance_characteristics
 * - Computational complexity: O(1) per sample (delegates to ADSR implementation)
 * - Memory footprint: ~40 bytes (ADSR envelope plus scalar parameters)
 * - Real-time safety: Inherits real-time properties from underlying ADSR
 * - Numerical precision: IEEE 754 single precision floating-point
 * 
 * @author Timothy Paul Read
 * @date 2025/03/10
 * @organization AABSTRKT at play via gaialive.com
 * @copyright 2025 Timothy Paul Read
 * @version 1.0
 * 
 * @license
 * This source code is provided as is, without warranty.
 * You may copy and distribute verbatim copies of this document.
 * You may modify and use this source code to create binary code 
 * for your own purposes, free or commercial.
 * 
 * @references
 * - "Analog Days: The Invention and Impact of the Moog Synthesizer" by Trevor Pinch & Frank Trocco
 * - "The Moog Book" by Andy Mackay
 * - "Sound Synthesis and Sampling" by Martin Russ
 * - "Computer Music: Synthesis, Composition, and Performance" by Dodge & Jerse
 */

#pragma once
#include "ADSR.h"

/**
 * @class MoogFilterEnvelope
 * @brief Specialized filter envelope generator with velocity sensitivity and depth control
 * 
 * The MoogFilterEnvelope class provides a complete filter modulation system combining
 * ADSR envelope generation with filter-specific functionality. It implements the classic
 * analog synthesizer paradigm of independent filter envelope control with configurable
 * depth, velocity sensitivity, and keyboard tracking integration.
 * 
 * @design_patterns
 * - **Adapter Pattern**: Adapts generic ADSR for filter-specific applications
 * - **Facade Pattern**: Simplifies complex envelope+scaling operations
 * - **Composition**: Contains ADSR instance rather than inheriting
 * 
 * @musical_characteristics
 * - **Classic Analog Response**: Emulates vintage Moog synthesizer filter behavior
 * - **Velocity Sensitivity**: Dynamic response to playing intensity
 * - **Configurable Depth**: Artistic control over modulation intensity
 * - **Seamless Integration**: Compatible with keyboard tracking and base frequency
 * 
 * @typical_applications
 * - Lead synthesizer voices with expressive filter sweeps
 * - Bass sounds with percussive filter attacks
 * - Pad sounds with slow filter evolution
 * - Rhythmic sequences with synchronized filter movement
 * - Educational software demonstrating classic synthesis techniques
 * 
 * @usage_example
 * @code
 * MoogFilterEnvelope filterEnv(44100.0f);
 * filterEnv.setADSR(0.01f, 0.1f, 0.7f, 0.3f);  // Fast attack, moderate decay/release
 * filterEnv.setEnvDepth(48.0f);                 // 4 octaves modulation range
 * 
 * // In note-on processing:
 * filterEnv.gate(1, velocity / 127.0f);
 * 
 * // In audio processing loop:
 * float cutoff = filterEnv.process(baseCutoff, keyTrackingValue);
 * filter.setCutoff(cutoff);
 * @endcode
 */
class MoogFilterEnvelope {
public:
    /**
     * @brief Construct filter envelope with audio system sample rate
     * 
     * Initializes the internal ADSR envelope generator with musically appropriate
     * default parameters optimized for filter frequency modulation. The default
     * settings provide a classic analog synthesizer response suitable for most
     * musical applications without requiring immediate parameter adjustment.
     * 
     * @param sampleRate Audio processing sample rate in Hz
     *                   Used for accurate envelope timing calculations
     *                   Typical values: 44100, 48000, 96000 Hz
     * 
     * @complexity O(1) - Constant time initialization
     * @memory ~40 bytes object size (ADSR envelope plus parameters)
     * @thread_safety Safe for concurrent construction
     * 
     * @default_parameters
     * - Attack: 441 samples (≈10ms at 44.1kHz) - Quick response, prevents clicks
     * - Decay: 4410 samples (≈100ms at 44.1kHz) - Moderate decay for musical character
     * - Sustain: 0.75 (75% level) - High sustain for consistent filter opening
     * - Release: 8820 samples (≈200ms at 44.1kHz) - Smooth release, prevents artifacts
     * - Envelope Depth: 1.0 (unity gain) - Conservative default modulation depth
     * 
     * @musical_rationale
     * Default parameters provide a balanced filter response suitable for lead voices,
     * bass sounds, and general-purpose synthesis without excessive filter movement
     * that could cause timbral instability or frequency content aliasing.
     */
    MoogFilterEnvelope(float sampleRate);
    
    /**
     * @brief Configure ADSR envelope timing parameters
     * 
     * Sets the four-stage envelope timing with sample-accurate precision.
     * All timing parameters are specified in seconds for intuitive musical
     * control, then converted to sample counts for precise real-time processing.
     * 
     * @param attackSec Attack time in seconds [0.001-10.0 typical]
     *                  Time to reach peak envelope level from gate-on
     *                  Shorter times: punchy, percussive character
     *                  Longer times: smooth, gradual filter opening
     * 
     * @param decaySec Decay time in seconds [0.01-10.0 typical]
     *                 Time to fall from peak to sustain level
     *                 Controls initial filter sweep character and musical punch
     * 
     * @param sustainLvl Sustain level [0.0-1.0]
     *                   Envelope level maintained during note hold phase
     *                   Higher values: brighter sustained timbre
     *                   Lower values: darker sustained timbre
     * 
     * @param releaseSec Release time in seconds [0.01-30.0 typical]
     *                   Time to fall from sustain to zero after gate-off
     *                   Controls filter closure speed and note tail character
     * 
     * @complexity O(1) - Direct parameter assignment
     * @precision Sample-accurate timing (limited by sample rate quantization)
     * @realtime_safety Real-time safe (no allocation or blocking)
     * 
     * @note The implementation currently uses a hardcoded 44.1kHz conversion factor
     * which may cause timing inaccuracies at other sample rates. This appears to be
     * a bug that should use the sampleRate parameter passed to constructor.
     * 
     * @musical_guidelines
     * - Lead sounds: Fast attack (1-10ms), medium decay (50-200ms), high sustain (70-90%)
     * - Bass sounds: Very fast attack (1-5ms), fast decay (20-100ms), medium sustain (50-70%)
     * - Pad sounds: Slow attack (100-1000ms), slow decay (200-2000ms), high sustain (80-95%)
     * - Percussive sounds: Fast attack (1-5ms), fast decay (10-50ms), low sustain (0-30%)
     */
    void setADSR(float attackSec, float decaySec, float sustainLvl, float releaseSec);
    
    /**
     * @brief Set envelope modulation depth for filter frequency control
     * 
     * Controls the intensity of envelope modulation applied to filter cutoff frequency.
     * Higher values create more dramatic filter sweeps, while lower values provide
     * subtle timbral animation. The depth parameter scales the envelope output
     * before adding to the base filter frequency.
     * 
     * @param depth Modulation depth in implementation-defined units
     *              Typical range: [0.0-96.0] representing semitones
     *              0.0: No envelope modulation (static filter)
     *              12.0: One octave maximum modulation range
     *              48.0: Four octaves maximum modulation range
     *              96.0: Eight octaves maximum modulation range
     * 
     * @complexity O(1) - Simple parameter assignment
     * @range [0.0-∞) theoretically, [0.0-96.0] musically practical
     * @precision Full floating-point precision maintained
     * @realtime_safety Real-time safe (immediate parameter update)
     * 
     * @musical_considerations
     * - Subtle depth (1-12 semitones): Natural timbral animation without obvious sweeps
     * - Moderate depth (12-24 semitones): Classic analog synthesizer filter character
     * - High depth (24-48 semitones): Dramatic sweeps for lead lines and special effects
     * - Extreme depth (48+ semitones): Sound design applications, may cause aliasing
     * 
     * @frequency_mapping
     * The depth value is added linearly to the filter cutoff frequency:
     * final_cutoff = base_cutoff + keytrack + (envelope_output × depth)
     * 
     * This linear mapping provides predictable behavior but may not match the
     * logarithmic frequency perception of human hearing. Future implementations
     * might benefit from exponential frequency mapping for more natural sweeps.
     */
    void setEnvDepth(float depth);
    
    /**
     * @brief Trigger envelope gate with optional velocity sensitivity
     * 
     * Controls envelope gate state for note-on and note-off events. The gate
     * mechanism provides musical control over envelope triggering and release,
     * matching the behavior of traditional analog synthesizer gate inputs.
     * 
     * @param gateState Gate control signal
     *                  1 (or non-zero): Trigger envelope attack phase (note-on)
     *                  0: Trigger envelope release phase (note-off)
     * 
     * @param velocity MIDI velocity value for future velocity sensitivity
     *                 Currently unused but provided for interface consistency
     *                 Range: [0.0-1.0] normalized velocity
     *                 Future implementations may use for envelope scaling
     * 
     * @complexity O(1) - Direct delegation to ADSR envelope
     * @determinism Deterministic gate state transition
     * @realtime_safety Real-time safe (no blocking operations)
     * 
     * @behavioral_notes
     * - Gate-on (1): Immediately triggers attack phase regardless of current envelope state
     * - Gate-off (0): Triggers release phase from current envelope level
     * - Retriggering: Multiple gate-on events restart attack from beginning
     * - Velocity parameter: Reserved for future velocity-sensitive implementations
     * 
     * @envelope_state_transitions
     * ```
     * gate(1) → Attack → Decay → Sustain (held while gate=1)
     * gate(0) → Release → Idle (envelope complete)
     * ```
     * 
     * @musical_applications
     * Proper gate timing is crucial for musical expression:
     * - Legato playing: Overlapping gates create smooth envelope transitions
     * - Staccato playing: Short gate pulses create percussive envelope shapes
     * - Sustained playing: Long gates maintain sustain phase for harmonic stability
     */
    void gate(int gateState, float velocity);
    
    /**
     * @brief Process envelope and generate filter cutoff frequency
     * 
     * Core processing method that generates the final filter cutoff frequency by
     * combining base frequency, keyboard tracking, and envelope modulation. This
     * method should be called once per audio sample to maintain proper envelope
     * timing and smooth filter frequency control.
     * 
     * @param cutoffBase Base filter cutoff frequency in Hz
     *                   Fundamental filter setting before modulation
     *                   Typical range: [20.0-20000.0] Hz
     * 
     * @param keyFollowValue Keyboard tracking contribution in Hz
     *                       Additional frequency offset based on note number
     *                       Provides consistent timbre across keyboard range
     * 
     * @return Final filter cutoff frequency in Hz
     *         Combined result of base frequency, key tracking, and envelope modulation
     *         Range: [0.0-∞) Hz, practical limit depends on audio system Nyquist frequency
     * 
     * @complexity O(1) - Single envelope processing call plus arithmetic
     * @precision IEEE 754 single precision floating-point
     * @realtime_safety Real-time safe (deterministic execution time)
     * 
     * @algorithm_implementation
     * 1. Process underlying ADSR envelope to get current envelope value [0.0-1.0]
     * 2. Scale envelope output by configured depth parameter
     * 3. Add base cutoff frequency and keyboard tracking contribution
     * 4. Return combined frequency for immediate filter control
     * 
     * @mathematical_formula
     * output_frequency = cutoffBase + keyFollowValue + (envelope_output × envDepth)
     * 
     * @frequency_bounds_checking
     * The implementation does not perform bounds checking on output frequency.
     * Callers should ensure reasonable input parameters to prevent:
     * - Negative frequencies (causes filter instability)
     * - Frequencies above Nyquist limit (causes aliasing)
     * - Excessively high frequencies (may cause numerical overflow)
     * 
     * @call_pattern
     * Must be called exactly once per audio sample for proper timing:
     * @code
     * for (int sample = 0; sample < bufferSize; ++sample) {
     *     float cutoff = filterEnv.process(baseCutoff, keyTracking);
     *     filter.setCutoff(cutoff);
     *     // Continue with audio processing...
     * }
     * @endcode
     * 
     * @debug_instrumentation
     * Contains commented rt_printf() for real-time debugging of envelope depth output.
     * Uncomment for development but remove in production due to potential timing impact.
     */
    float process(float cutoffBase, float keyFollowValue);

private:
    /**
     * @brief Internal ADSR envelope generator instance
     * 
     * Core envelope processing engine providing four-stage ADSR envelope generation
     * with configurable timing, curvature, and gate control. All envelope-specific
     * functionality is delegated to this component for modularity and code reuse.
     * 
     * @composition_rationale
     * Uses composition rather than inheritance to maintain clear separation between
     * generic envelope generation and filter-specific functionality. This design
     * allows independent evolution of envelope algorithms while preserving the
     * filter-specific interface and behavior.
     */
    ADSR envelope;
    
    /**
     * @brief Envelope modulation depth scaling factor
     * 
     * Controls the intensity of envelope modulation applied to filter frequency.
     * Acts as a multiplier for the envelope output before addition to base frequency,
     * providing artistic control over filter sweep intensity while maintaining
     * envelope timing characteristics.
     * 
     * @units Implementation-defined (typically semitones or Hz)
     * @range [0.0-∞) theoretically, [0.0-96.0] musically practical
     * @default 1.0 (unity gain, conservative modulation depth)
     */
    float envDepth;
};

