/**
 * @file ResonanceRamp.cpp
 * @brief Implementation of linear parameter interpolation system
 * 
 * This implementation provides efficient, real-time parameter smoothing with
 * automatic completion detection and configurable timing. The system is
 * optimized for filter resonance control but suitable for any continuous
 * parameter requiring smooth transitions in audio applications.
 */

#include "ResonanceRamp.h"

/**
 * @brief Initialize parameter ramp with timing configuration
 * 
 * @param rate Audio sample rate for timing calculations
 * @param rampTimeMs Interpolation duration in milliseconds
 * 
 * @implementation_details
 * Calculates increment per sample based on ramp time and sample rate.
 * The increment represents the reciprocal of total samples in ramp period,
 * providing normalized step size for linear interpolation algorithms.
 * 
 * @mathematical_derivation
 * Total samples in ramp = rampTimeMs × 0.001 × sampleRate
 * Increment per sample = 1.0 / total_samples
 * 
 * This gives a normalized increment that, when applied over the ramp duration,
 * will complete exactly one full parameter transition from 0.0 to 1.0.
 */
ResonanceRamp::ResonanceRamp(float rate, float rampTimeMs)
    : sampleRate(rate), currentValue(0.5f), targetValue(0.5f) {
    /**
     * Calculate increment per sample for linear interpolation
     * Formula: 1.0 / (rampTimeMs * 0.001 * sampleRate)
     * 
     * The 0.001 factor converts milliseconds to seconds for sample rate multiplication
     * The reciprocal provides step size for normalized [0.0-1.0] parameter range
     */
    incrementPerSample = 1.0f / (rampTimeMs * 0.001f * sampleRate);
}

/**
 * @brief Update interpolation target value
 * 
 * @param targetRes New target parameter value
 * 
 * Simple parameter assignment that immediately redirects interpolation
 * toward new target. No recalculation of increment required since the
 * system uses normalized step sizes with directional logic in process().
 */
void ResonanceRamp::setTarget(float targetRes) {
    targetValue = targetRes;
}

/**
 * @brief Execute linear parameter interpolation with completion detection
 * 
 * @return Current interpolated parameter value
 * 
 * @algorithm_implementation
 * The method uses bidirectional interpolation logic with automatic completion
 * detection. Separate handling for upward and downward transitions ensures
 * symmetric behavior and numerical stability across parameter ranges.
 * 
 * @performance_analysis
 * - Two floating-point comparisons (direction and completion)
 * - One floating-point addition or subtraction (increment application)
 * - Branch prediction friendly (consistent direction during ramps)
 * - Total execution: ~3-5 CPU cycles per sample
 * 
 * @numerical_precision
 * Completion detection uses direct comparison rather than threshold-based
 * detection, relying on overshoot correction for exact target achievement.
 * This approach ensures precise completion regardless of increment size
 * or floating-point precision limitations.
 */
float ResonanceRamp::process() {
    /**
     * Handle upward interpolation (current < target)
     */
    if (currentValue < targetValue) {
        /**
         * Apply positive increment toward target
         */
        currentValue += incrementPerSample;
        
        /**
         * Check for overshoot and correct to exact target
         * Ensures precise completion without oscillation
         */
        if (currentValue > targetValue)
            currentValue = targetValue;
    } 
    /**
     * Handle downward interpolation (current > target)
     */
    else if (currentValue > targetValue) {
        /**
         * Apply negative increment toward target
         */
        currentValue -= incrementPerSample;
        
        /**
         * Check for undershoot and correct to exact target
         * Maintains symmetrical completion behavior
         */
        if (currentValue < targetValue)
            currentValue = targetValue;
    }
    
    /**
     * Return current parameter value
     * (No action needed when currentValue == targetValue)
     */
    return currentValue;
}

