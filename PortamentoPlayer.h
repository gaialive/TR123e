/**
 * @file PortamentoPlayer.h
 * @brief Real-time pitch interpolation engine for smooth frequency transitions
 * 
 * This module implements sophisticated pitch interpolation algorithms for creating
 * smooth frequency transitions between musical notes. The system provides sample-accurate
 * pitch sliding with configurable timing, supporting both linear and exponential
 * interpolation methods appropriate for different musical contexts.
 * 
 * @mathematical_foundation
 * The implementation uses linear frequency interpolation in Hertz domain rather than
 * logarithmic (semitone) domain for computational efficiency. While logarithmic
 * interpolation would be more perceptually accurate, linear interpolation provides
 * acceptable results for typical portamento times (50-500ms) while maintaining
 * real-time performance.
 * 
 * @frequency_conversion_theory
 * MIDI note to frequency conversion follows the equal temperament formula:
 * 
 * f(n) = 440 × 2^((n-69)/12)
 * 
 * Where:
 * - f(n) = frequency in Hz
 * - n = MIDI note number
 * - 440 Hz = A4 reference frequency (MIDI note 69)
 * - 12 = semitones per octave in equal temperament
 * 
 * @interpolation_analysis
 * Linear interpolation formula: f(t) = f₀ + (f₁ - f₀) × (t/T)
 * 
 * Where:
 * - f₀ = starting frequency
 * - f₁ = target frequency  
 * - t = elapsed time
 * - T = total portamento duration
 * 
 * @performance_characteristics
 * - Sample rate independence: Adapts to any audio sample rate
 * - Computational complexity: O(1) per sample processing
 * - Memory footprint: ~32 bytes object size
 * - Numerical stability: IEEE 754 compliant arithmetic
 * 
 * @author Timothy Paul Read
 * @date 2025/03/10
 * @organization AABSTRKT at play via gaialive.com
 * @copyright 2025 Timothy Paul Read
 * @version 1.0
 */

#pragma once

/**
 * @class PortamentoPlayer
 * @brief High-precision frequency interpolation engine for musical portamento
 * 
 * The PortamentoPlayer class provides professional-grade pitch sliding capabilities
 * with sample-accurate timing and configurable interpolation parameters. It handles
 * MIDI note-to-frequency conversion, interpolation timing calculations, and real-time
 * frequency generation suitable for direct oscillator control.
 * 
 * @design_principles
 * - **Sample Accuracy**: All timing calculations performed at audio sample rate
 * - **Musical Precision**: MIDI note conversion maintains standard tuning accuracy
 * - **Performance Oriented**: Optimized for real-time audio processing constraints
 * - **State Consistency**: Maintains frequency continuity across note transitions
 * 
 * @interpolation_characteristics
 * - **Linear Interpolation**: Computationally efficient with acceptable musical results
 * - **Configurable Timing**: User-controlled portamento duration (1ms-10s range)
 * - **Immediate Response**: Optional instant frequency jumps for staccato playing
 * - **Sustain Capability**: Maintains frequency during note release phases
 * 
 * @musical_applications
 * - Monophonic lead synthesizers with expressive pitch control
 * - Bass instruments requiring smooth frequency transitions
 * - Vocal synthesis with natural pitch slides
 * - String instrument emulation with portamento
 * - Theremin-style continuous pitch instruments
 * 
 * @usage_example
 * @code
 * PortamentoPlayer player(44100.0f, 150.0f);  // 150ms default glide
 * 
 * // Note-on with portamento:
 * player.noteOn(60, true);   // C4 with smooth transition
 * 
 * // Audio processing loop:
 * for (int i = 0; i < bufferSize; ++i) {
 *     float frequency = player.process();
 *     float sample = oscillator.process(frequency);
 *     outputBuffer[i] = sample;
 * }
 * @endcode
 */
class PortamentoPlayer {
public:
    /**
     * @brief Construct PortamentoPlayer with audio system parameters
     * 
     * @param sampleRate Audio processing sample rate in Hz
     *                   Used for timing calculations and interpolation precision
     *                   Typical values: 44100, 48000, 96000 Hz
     * 
     * @param defaultPortamentoTimeMs Default portamento duration in milliseconds
     *                               Configurable timing for pitch transitions
     *                               Default: 100ms (musically appropriate for most contexts)
     *                               Range: [1.0-10000.0]ms practical limits
     * 
     * @complexity O(1) - Constant time initialization
     * @memory ~32 bytes object size (platform dependent)
     * @thread_safety Safe for concurrent construction
     * 
     * @implementation_notes
     * Constructor initializes all frequency state to 0.0f, ensuring clean
     * startup behavior without frequency artifacts on first note event.
     */
    PortamentoPlayer(float sampleRate, float defaultPortamentoTimeMs = 100.0f);

    /**
     * @brief Update portamento timing parameter in real-time
     * 
     * @param timeMs New portamento duration in milliseconds
     *               Controls the time required for complete pitch transitions
     *               Affects only future note events (current glides unchanged)
     * 
     * @range [0.1-10000.0]ms practical limits for musical applications
     * @precision Full floating-point precision maintained
     * @realtime_safety Real-time safe (no allocation or blocking)
     * 
     * @musical_considerations
     * - Short times (1-50ms): Rapid pitch slides, good for fast passages
     * - Medium times (50-200ms): Classic portamento feel, most musical
     * - Long times (200ms+): Dramatic pitch sweeps, special effects
     * 
     * @note Changes take effect immediately but only apply to subsequent
     * noteOn() calls. Active portamento continues with original timing.
     */
    void setPortamentoTime(float timeMs);
    
    /**
     * @brief Trigger new note with optional portamento control
     * 
     * Initiates frequency transition to new MIDI note with configurable
     * portamento behavior. Calculates target frequency and interpolation
     * parameters for smooth real-time processing.
     * 
     * @param midiNote Target MIDI note number [0-127]
     *                 Converted to frequency using equal temperament formula
     *                 Note 69 (A4) = 440.0 Hz reference
     * 
     * @param portamentoOn Enable/disable portamento for this transition
     *                     true: Smooth interpolation from current frequency
     *                     false: Immediate frequency jump (staccato)
     * 
     * @complexity O(1) - Constant time note processing
     * @precision IEEE 754 floating-point arithmetic accuracy
     * @realtime_safety Real-time safe (deterministic execution time)
     * 
     * @algorithm_behavior
     * - If portamentoOn == false OR currentFreq == 0.0: Immediate frequency jump
     * - If portamentoOn == true AND currentFreq != 0.0: Calculate interpolation
     * - Interpolation uses linear frequency domain for computational efficiency
     * - Sample-accurate timing based on portamentoTimeMs and sample rate
     * 
     * @state_updates
     * Updates internal state variables:
     * - targetFreq: Set to new note frequency
     * - currentNote: Updated for external state queries
     * - noteIsOn: Set to true for processing control
     * - incrementPerSample: Calculated for linear interpolation
     */
    void noteOn(int midiNote, bool portamentoOn);
    
    /**
     * @brief Signal note release for envelope coordination
     * 
     * Marks note as released while maintaining current frequency for
     * envelope release phase. Frequency continues processing to support
     * amplitude envelope decay without pitch artifacts.
     * 
     * @complexity O(1) - Simple state flag update
     * @realtime_safety Real-time safe (no blocking operations)
     * 
     * @behavioral_notes
     * - Does not immediately stop frequency output
     * - Allows continued frequency generation during envelope release
     * - Frequency freezes when interpolation completes
     * - Enables natural amplitude decay without pitch drift
     */
    void noteOff();
    
    /**
     * @brief Query current MIDI note number
     * 
     * @return Current MIDI note number being processed
     *         Returns last note passed to noteOn() regardless of interpolation state
     * 
     * @complexity O(1) - Direct member variable access
     * @thread_safety Safe for concurrent read access
     * 
     * @use_cases
     * - Keyboard tracking calculations
     * - Filter frequency modulation
     * - Display/UI updates
     * - MIDI echo/monitoring
     */
    int getCurrentNote();
    
    /**
     * @brief Query current instantaneous frequency
     * 
     * @return Current frequency in Hz (may be interpolating)
     *         Represents the exact frequency at current processing moment
     * 
     * @complexity O(1) - Direct member variable access
     * @precision Full floating-point precision
     * @range [0.0, ~13289.75]Hz (MIDI note 0-127 frequency range)
     * 
     * @applications
     * - Frequency display/monitoring
     * - Secondary oscillator synchronization
     * - Frequency-dependent processing (filters, effects)
     * - Analysis and debugging
     */
    float getCurrentFreq();
    
    /**
     * @brief Generate next frequency sample with interpolation processing
     * 
     * Core processing method called once per audio sample to generate
     * current frequency value with smooth interpolation. Handles all
     * interpolation logic, completion detection, and state management.
     * 
     * @return Current frequency in Hz for oscillator control
     *         Smoothly interpolated value between source and target frequencies
     * 
     * @complexity O(1) - Constant time per sample processing
     * @determinism Bounded execution time regardless of interpolation state
     * @realtime_safety Real-time safe (no allocation or blocking)
     * 
     * @processing_logic
     * 1. Check if interpolation is active (noteIsOn OR frequency != target)
     * 2. If active: Call interpolateFrequency() for smooth progression
     * 3. If inactive: Return current frequency (sustain for envelope release)
     * 
     * @state_management
     * - Automatically detects interpolation completion
     * - Maintains frequency during note release phase
     * - Handles edge cases (zero frequency, target reached)
     * - Provides continuous output for audio processing chain
     * 
     * @call_pattern
     * Must be called exactly once per audio sample for proper timing:
     * @code
     * for (int sample = 0; sample < bufferSize; ++sample) {
     *     float freq = player.process();
     *     // Use frequency for oscillator control...
     * }
     * @endcode
     */
    float process();

private:
    /**
     * @brief Convert MIDI note number to frequency using equal temperament
     * 
     * @param midiNote MIDI note number [0-127]
     * @return Corresponding frequency in Hz
     * 
     * Implements standard equal temperament conversion formula with
     * 440 Hz reference for A4 (MIDI note 69). Provides standard musical
     * tuning compatible with conventional instruments and software.
     */
    float midiToFreq(int midiNote);
    
    /**
     * @brief Perform linear frequency interpolation for smooth transitions
     * 
     * @return Current interpolated frequency value
     * 
     * Core interpolation algorithm implementing sample-accurate linear
     * frequency transitions. Handles completion detection and state cleanup
     * when target frequency is reached within numerical precision limits.
     */
    float interpolateFrequency();
    
    /**
     * @brief Current MIDI note number being processed
     * 
     * Stores the most recent MIDI note number passed to noteOn() for
     * external state queries and keyboard tracking applications.
     */
    int currentNote;
    
    /**
     * @brief Audio system sample rate in Hz
     * 
     * Cached sample rate used for timing calculations and interpolation
     * precision. Enables sample-accurate portamento timing independent
     * of audio system configuration.
     */
    float sampleRate;
    
    /**
     * @brief Current instantaneous frequency in Hz
     * 
     * Real-time frequency value updated each sample during interpolation.
     * Represents the exact frequency being output for oscillator control.
     */
    float currentFreq;
    
    /**
     * @brief Target frequency for interpolation in Hz
     * 
     * Destination frequency calculated from MIDI note number using
     * equal temperament conversion. Interpolation progresses toward
     * this value over the configured portamento duration.
     */
    float targetFreq;
    
    /**
     * @brief Frequency increment per audio sample
     * 
     * Calculated interpolation step size for linear frequency progression.
     * Value determined by (targetFreq - currentFreq) / portamentoSamples
     * providing precise timing control over frequency transitions.
     */
    float incrementPerSample;
    
    /**
     * @brief Portamento duration in milliseconds
     * 
     * User-configurable timing parameter controlling the duration of
     * frequency transitions. Converted to sample count for precise
     * interpolation timing calculations.
     */
    float portamentoTimeMs;

    /**
     * @brief Note activity state flag
     * 
     * Boolean indicator of current note status for processing control.
     * Used to determine when frequency generation should continue during
     * envelope release phases after note-off events.
     */
    bool noteIsOn;
};

