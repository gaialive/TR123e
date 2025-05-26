/**
 * @file MoogLadderFilterScalarFixed.h  
 * @brief Fixed/corrected scalar implementation with improved utilities
 * 
 * This header represents a refined version of the scalar Moog ladder filter
 * implementation with improved utility functions, corrected parameter handling,
 * and enhanced numerical stability. It serves as a bridge between the original
 * Gen~ translation and optimized production implementations.
 * 
 * @implementation_improvements
 * This version incorporates several key improvements over the base scalar implementation:
 * 
 * **Enhanced Utility Functions**: Improved clamping and denormal protection with
 * more appropriate thresholds and better numerical stability characteristics.
 * 
 * **Corrected Parameter Semantics**: Fixed parameter mapping and processing order
 * to ensure consistent behavior with the original Gen~ implementation while
 * improving usability and predictability.
 * 
 * **Numerical Stability**: Enhanced denormal number protection with more appropriate
 * thresholds and improved floating-point precision handling for extended operation.
 * 
 * **Interface Consistency**: Standardized method signatures and parameter conventions
 * to match the unified interface established in MoogLadderFilterBase.h.
 * 
 * @utility_function_improvements
 * **Enhanced Clamping**: Uses std::fmin/std::fmax for better IEEE 754 compliance
 * and proper handling of NaN values according to floating-point standards.
 * 
 * **Improved Denormal Protection**: Tighter threshold (1e-15) for more aggressive
 * denormal elimination while preserving musical content, optimized for modern
 * processors with different denormal handling characteristics.
 * 
 * **Inline Optimization**: Strategic use of inline functions to eliminate call
 * overhead while maintaining code organization and readability.
 * 
 * @corrected_parameter_handling
 * - **Direct Resonance Mapping**: Simplified rc parameter assignment without
 *   intermediate processing that could introduce artifacts or inconsistencies
 * - **Unified Mode Selection**: Consistent mode indexing across all implementations
 * - **Proper Input Signal Routing**: Corrected assignment of audio, resonance,
 *   envelope, and noise inputs to appropriate processing stages
 * 
 * @applications
 * - Production audio software requiring reliable scalar processing
 * - Reference implementation for optimization validation
 * - Educational platform with improved numerical characteristics
 * - Foundation for platform-specific optimizations
 * - Research tool with enhanced stability and precision
 */

#pragma once

#include <cmath>        // Mathematical functions for audio processing
#include <algorithm>    // STL algorithms for improved parameter validation

/**
 * @brief Enhanced parameter clamping with IEEE 754 compliance
 * 
 * @param x Input value to constrain
 * @param a Minimum allowed value
 * @param b Maximum allowed value
 * @return Clamped value with proper NaN handling
 * 
 * Improved clamping function using std::fmin/std::fmax for better IEEE 754
 * compliance and proper handling of special floating-point values including
 * NaN and infinity according to standard specifications.
 */
inline float clamp(float x, float a, float b) { 
    return std::fmin(std::fmax(x, a), b); 
}

/**
 * @brief Enhanced denormal number protection with tighter threshold
 * 
 * @param x Input floating-point value to validate
 * @return 0.0f if denormal, otherwise original value
 * 
 * Improved denormal protection with tighter threshold (1e-15) for more
 * aggressive elimination of problematic values while preserving all
 * musically relevant content. Optimized for modern processors with
 * varying denormal handling characteristics.
 */
inline float fixdenorm(float x) { 
    return (std::abs(x) < 1e-15f ? 0.0f : x); 
}

/**
 * @class MoogLadderFilterScalar
 * @brief Enhanced scalar Moog ladder filter with improved stability and interface
 * 
 * This class provides a refined scalar implementation of the Huovilainen Moog
 * ladder filter with improved utility functions, corrected parameter handling,
 * and enhanced numerical stability while maintaining exact algorithmic
 * compatibility with the original Gen~ implementation.
 * 
 * @enhancement_focus
 * - **Numerical Stability**: Improved denormal protection and precision handling
 * - **Interface Consistency**: Standardized parameter mapping and method signatures
 * - **Utility Improvements**: Enhanced clamping and validation functions
 * - **Production Readiness**: Optimized for reliable long-term operation
 * - **Validation Reference**: Serves as corrected reference for optimization verification
 */
class MoogLadderFilterScalar {
public:
    /**
     * @brief Construct enhanced scalar filter with sample rate configuration
     * 
     * @param sampleRate Audio processing sample rate for coefficient calculation
     */
    MoogLadderFilterScalar(float sampleRate);
    
    /**
     * @brief Reset all filter state with enhanced clearing
     * 
     * Improved state clearing that ensures complete elimination of residual
     * content while preparing filter for optimal startup behavior.
     */
    void reset();
    
    /**
     * @brief Update sample rate with enhanced coefficient recalculation
     * 
     * @param sr New sample rate for improved coefficient computation
     */
    void setSampleRate(float sr);
    
    /**
     * @brief Set parameters with corrected semantics and improved validation
     * 
     * @param cutoffHz Cutoff frequency in Hz with enhanced range validation
     * @param resonance Resonance amount [0.0-1.0] with improved scaling
     * @param mode Filter mode selection [0-5] with consistent indexing
     */
    void setParams(float cutoffHz, float resonance, int mode);
    
    /**
     * @brief Process audio with corrected input mapping and enhanced stability
     * 
     * @param in1 Primary audio input with improved conditioning
     * @param in2 Resonance modulation with corrected processing
     * @param in3 Envelope control with enhanced scaling
     * @param in4 Thermal noise with improved integration
     * @return Filtered audio sample with enhanced precision
     */
    float process(float in1, float in2, float in3, float in4);

private:
    /**
     * @brief System and filter parameters with enhanced organization
     */
    float sr;                           ///< Sample rate for coefficient calculation
    float s1, s2, s3, s4, s5, s6, s7, s8; ///< Filter stage states with improved precision
    float slim, previn;                 ///< Saturation and delay states with enhanced stability
    float rc, fc;                       ///< Resonance and frequency parameters with corrected scaling
    float expr1, expr2;                 ///< Sample rate dependent coefficients with improved calculation
    int mode;                           ///< Filter mode selection with consistent indexing
};

