/**
 * @file ResonanceRamp.h
 * @brief Smooth parameter interpolation for filter resonance control
 * 
 * This module implements sophisticated parameter smoothing for filter resonance values,
 * preventing audio artifacts caused by abrupt control changes. The system provides
 * configurable ramping time with sample-accurate interpolation, essential for
 * high-Q filter applications where parameter discontinuities can cause instability,
 * clicks, or unwanted transients in the audio output.
 * 
 * @technical_necessity
 * Digital filters, particularly high-Q resonant designs like the Moog ladder filter,
 * are sensitive to parameter changes that occur faster than their internal time
 * constants. Abrupt resonance changes can cause:
 * 
 * 1. **Audible Clicks**: Discontinuities in filter state variables
 * 2. **Instability**: Sudden parameter jumps may exceed filter stability margins
 * 3. **Aliasing**: Rapid parameter changes can introduce high-frequency artifacts
 * 4. **Musical Disruption**: Harsh transitions that interfere with musical expression
 * 
 * @smoothing_theory
 * The implementation uses linear interpolation in the parameter domain rather than
 * the frequency domain. While this is simpler computationally, it may not provide
 * optimal perceptual smoothing for all applications. The linear approach offers:
 * 
 * - **Computational Efficiency**: Single increment per sample, minimal CPU overhead
 * - **Predictable Behavior**: Linear parameter progression with known completion time
 * - **Numerical Stability**: Simple arithmetic without transcendental functions
 * - **Real-time Safety**: Bounded execution time suitable for audio processing
 * 
 * @mathematical_model
 * Linear interpolation: p(t) = p₀ + (p₁ - p₀) × (t/T)
 * 
 * Discrete implementation: p[n] = p[n-1] + increment_per_sample
 * Where: increment_per_sample = (target - current) / (ramp_time × sample_rate)
 * 
 * @performance_characteristics
 * - Computational complexity: O(1) per sample
 * - Memory footprint: 16 bytes (4 float members)
 * - Convergence time: User-configurable (typically 10-100ms)
 * - Precision: IEEE 754 single precision with automatic completion detection
 * 
 * @author Timothy Paul Read (inferred from project context)
 * @date 2025/03/10 (estimated)
 * @version 1.0
 */

#pragma once

/**
 * @class ResonanceRamp
 * @brief Linear parameter interpolation engine for smooth resonance control
 * 
 * The ResonanceRamp class provides professional-grade parameter smoothing specifically
 * designed for filter resonance control in real-time audio applications. It implements
 * efficient linear interpolation with automatic completion detection and configurable
 * ramping time to prevent audio artifacts while maintaining responsive control.
 * 
 * @design_philosophy
 * - **Simplicity**: Linear interpolation for predictable behavior and minimal CPU overhead
 * - **Reliability**: Automatic completion detection prevents infinite ramping
 * - **Responsiveness**: Configurable ramp time balances smoothness with control latency
 * - **Universality**: Generic parameter smoother suitable for various control applications
 * 
 * @applications
 * - Filter resonance smoothing (primary use case)
 * - Oscillator frequency gliding (alternative to portamento)
 * - Amplitude level transitions
 * - LFO rate smoothing
 * - Any continuous parameter requiring smooth transitions
 * 
 * @usage_example
 * @code
 * ResonanceRamp resonanceSmooth(44100.0f, 50.0f);  // 50ms ramp time
 * 
 * // User changes resonance control:
 * resonanceSmooth.setTarget(0.8f);
 * 
 * // In audio processing loop:
 * for (int i = 0; i < bufferSize; ++i) {
 *     float smoothResonance = resonanceSmooth.process();
 *     filter.setResonance(smoothResonance);
 *     // Continue with audio processing...
 * }
 * @endcode
 */
class ResonanceRamp {
public:
    /**
     * @brief Construct parameter ramp with timing configuration
     * 
     * @param sampleRate Audio processing sample rate in Hz
     *                   Used for accurate timing calculations
     *                   Typical values: 44100, 48000, 96000 Hz
     * 
     * @param rampTimeMs Interpolation duration in milliseconds
     *                   Time required for complete parameter transition
     *                   Default: 50ms (good balance of smoothness and responsiveness)
     *                   Range: [1.0-1000.0]ms practical limits
     * 
     * @complexity O(1) - Constant time initialization
     * @memory 16 bytes object size (4 float members)
     * @thread_safety Safe for concurrent construction
     * 
     * @initialization_behavior
     * - currentValue: 0.5 (middle resonance value, safe default)
     * - targetValue: 0.5 (no initial ramping required)
     * - incrementPerSample: Calculated for specified ramp time
     * 
     * @ramp_time_considerations
     * - Short times (1-20ms): Minimal smoothing, preserves control responsiveness
     * - Medium times (20-100ms): Optimal balance for most musical applications
     * - Long times (100ms+): Heavy smoothing, may feel sluggish but very smooth
     */
    ResonanceRamp(float sampleRate, float rampTimeMs = 50.0f);
    
    /**
     * @brief Set new target value for parameter interpolation
     * 
     * Establishes new target parameter value, triggering smooth interpolation
     * from current value to target over the configured ramp time. Multiple
     * calls to setTarget() before completion will redirect interpolation
     * to the new target, providing responsive control behavior.
     * 
     * @param targetRes New target resonance value
     *                  Typical range: [0.0-1.0] for normalized resonance
     *                  No bounds checking performed for maximum performance
     * 
     * @complexity O(1) - Simple parameter assignment
     * @realtime_safety Real-time safe (no allocation or blocking)
     * @response_behavior Immediate redirection of interpolation target
     * 
     * @interpolation_redirection
     * If called during active interpolation, the system smoothly redirects
     * toward the new target without discontinuities. The interpolation
     * recalculates timing based on remaining distance and original ramp time.
     * 
     * @musical_usage_patterns
     * - Real-time control: Responds to continuous controller movements
     * - Preset changes: Smooth transitions between stored parameter sets
     * - Automation: Gradual parameter changes for musical expression
     * - Safety ramping: Gentle parameter initialization to prevent artifacts
     */
    void setTarget(float targetRes);
    
    /**
     * @brief Process interpolation and return current parameter value
     * 
     * Core processing method that advances the parameter interpolation and
     * returns the current smoothed value. Must be called once per audio sample
     * to maintain proper timing and smooth parameter progression.
     * 
     * @return Current interpolated parameter value
     *         Smoothly progressing between previous and target values
     *         Equals target value when interpolation complete
     * 
     * @complexity O(1) - Constant time processing per sample
     * @determinism Bounded execution time regardless of parameter values
     * @realtime_safety Real-time safe (no allocation or blocking)
     * 
     * @algorithm_behavior
     * 1. **Direction Detection**: Determines upward or downward interpolation
     * 2. **Increment Application**: Applies calculated step size toward target
     * 3. **Completion Detection**: Stops interpolation when target reached
     * 4. **Value Return**: Provides current parameter value for immediate use
     * 
     * @completion_logic
     * Interpolation completes when the remaining distance to target becomes
     * smaller than the increment step size. This ensures completion within
     * one additional sample period regardless of floating-point precision.
     * 
     * @numerical_stability
     * Uses separate handling for positive and negative interpolation directions
     * to ensure symmetric behavior and proper completion detection in both
     * directions without floating-point precision issues.
     * 
     * @call_pattern
     * Must be called exactly once per audio sample:
     * @code
     * for (int sample = 0; sample < bufferSize; ++sample) {
     *     float resonance = ramp.process();
     *     // Use resonance value immediately...
     * }
     * @endcode
     */
    float process();

private:
    /**
     * @brief Audio system sample rate for timing calculations
     * 
     * Cached sample rate used for converting millisecond ramp times to
     * sample counts and calculating per-sample increment values.
     * Essential for sample-accurate parameter timing.
     */
    float sampleRate;
    
    /**
     * @brief Current parameter value during interpolation
     * 
     * Real-time parameter value updated each sample during interpolation.
     * Represents the instantaneous parameter value for immediate use by
     * audio processing components. Progresses linearly toward target value.
     */
    float currentValue;
    
    /**
     * @brief Target parameter value for interpolation destination
     * 
     * Destination value for current interpolation process. Set by setTarget()
     * method and remains constant during interpolation unless updated by
     * subsequent setTarget() calls for responsive control behavior.
     */
    float targetValue;
    
    /**
     * @brief Parameter increment applied per audio sample
     * 
     * Calculated step size for linear interpolation, determined by the
     * distance between current and target values divided by the total
     * number of samples in the ramp time. Provides precise timing control
     * over parameter transitions.
     * 
     * @calculation increment = 1.0 / (rampTimeMs * 0.001 * sampleRate)
     * @units Parameter_units_per_sample (dimensionless ratio)
     * @direction Positive for upward ramps, negative for downward ramps
     */
    float incrementPerSample;
};

