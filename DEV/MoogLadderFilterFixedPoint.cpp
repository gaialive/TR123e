/**
 * @file MoogLadderFilterFixedPoint.cpp
 * @brief Implementation of high-performance fixed-point Moog ladder filter
 */

#include "MoogLadderFilterFixedPoint.h"

/**
 * @brief Initialize fixed-point Moog filter with comprehensive setup
 */
MoogLadderFilterFixedPoint::MoogLadderFilterFixedPoint(int sampleRate) {
    this->sampleRate = sampleRate;
    reset();
}

/**
 * @brief Configure cutoff frequency with fixed-point coefficient calculation
 * 
 * This method implements frequency-to-coefficient conversion using fixed-point
 * arithmetic while maintaining adequate precision for musical applications.
 * The algorithm carefully manages bit precision to prevent overflow while
 * maximizing useful dynamic range.
 */
void MoogLadderFilterFixedPoint::setCutoff(int frequency) {
    /**
     * Validate and clamp frequency to safe operating range
     */
    fc = clamp_int(frequency, 20, sampleRate / 2);
    
    /**
     * Convert to normalized frequency using Q16 fixed-point arithmetic
     * Formula: normFreq = (fc << 16) / sampleRate
     * This shifts fc left by 16 bits then divides, effectively creating Q16 format
     */
    int normFreq = (fc << 16) / sampleRate; // Q16 fixed-point
    
    /**
     * Calculate filter coefficient 'g' using fixed-point multiplication
     * Formula: g = normFreq² (in Q16 format)
     * Q16 * Q16 >> 16 maintains Q16 format while preventing overflow
     */
    int g = (normFreq * normFreq) >> 16;    // Q16 * Q16 >> 16 = Q16
    
    /**
     * Calculate alpha coefficient for ladder implementation
     * Formula: alpha = g / (1 + g)
     * This provides the fundamental filter coefficient for each pole
     */
    alpha = g / (1 + g);
    
    /**
     * Calculate feedback amount for resonance implementation
     * Formula: feedbackAmount = alpha²
     * Provides the scaling factor for resonance feedback around ladder
     */
    feedbackAmount = (alpha * alpha) >> 16;
}

/**
 * @brief Configure resonance with 8-bit precision scaling
 * 
 * Converts 8-bit resonance control to internal Q8 fixed-point format
 * optimized for feedback calculations in the ladder structure.
 */
void MoogLadderFilterFixedPoint::setResonance(int resonance) {
    /**
     * Clamp resonance to 8-bit range and scale to Q8 format
     * >> 2 operation converts from 8-bit to 6-bit range for stability
     */
    rc = clamp_int(resonance, 0, 255) >> 2;  // Scale to Q8
}

/**
 * @brief Clear all filter state for clean initialization
 * 
 * Essential for preventing artifacts from previous audio content
 * and ensuring predictable filter behavior from first sample.
 */
void MoogLadderFilterFixedPoint::reset() {
    prevIn = 0;
    for (int i = 0; i < 4; ++i)
        s[i] = 0;
}

/**
 * @brief Process audio sample through fixed-point ladder algorithm
 * 
 * Implements the complete ladder filter using fixed-point arithmetic with
 * careful attention to overflow prevention and precision management.
 */
int MoogLadderFilterFixedPoint::process(int in) {
    /**
     * Apply anti-denormalization offset to prevent filter stasis
     * Small offset ensures filter continues processing during silence
     */
    int input = in + 1;  // Anti-denormalization offset

    /**
     * Calculate nonlinear feedback using tanh approximation
     * Provides the resonance effect through controlled feedback
     */
    int feedback = tanh_approx(rc * (s[3] - input));

    /**
     * Process through four ladder stages with fixed-point arithmetic
     * Each stage implements: output = (input * alpha + state * feedback) >> 16
     * The >> 16 operation maintains Q16 format after multiplication
     */
    int s1 = (((input - feedback) * alpha) >> 16) + ((feedbackAmount * s[0]) >> 16);
    int s2 = ((s1 * alpha) >> 16) + ((feedbackAmount * s[1]) >> 16);
    int s3 = ((s2 * alpha) >> 16) + ((feedbackAmount * s[2]) >> 16);
    int s4 = ((s3 * alpha) >> 16) + ((feedbackAmount * s[3]) >> 16);

    /**
     * Update filter state variables for next sample
     * These maintain the temporal memory of each filter pole
     */
    s[0] = s1;
    s[1] = s2;
    s[2] = s3;
    s[3] = s4;

    return s4;
}

/**
 * @brief Fast tanh approximation using quadratic polynomial
 * 
 * Provides efficient nonlinear processing suitable for fixed-point systems
 * where transcendental function evaluation would be prohibitively expensive.
 */
int MoogLadderFilterFixedPoint::tanh_approx(int x) {
    /**
     * Clamp input to prevent overflow in subsequent operations
     */
    x = clamp_int(x, -32768, 32767);
    
    /**
     * Quadratic approximation: tanh(x) ≈ x - x²/65536
     * The >> 16 operation provides the division by 65536 in fixed-point
     */
    return x - ((x * x) >> 16);  // Quadratic approximation
}

