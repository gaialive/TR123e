/**
 * @file MoogFilterEnvelope.cpp
 * @brief Implementation of specialized filter envelope generator
 * 
 * This implementation provides a complete filter envelope system optimized for
 * classic analog synthesizer filter frequency modulation. The system combines
 * proven ADSR envelope algorithms with filter-specific enhancements for
 * professional music synthesis applications.
 */

#include "MoogFilterEnvelope.h"

/**
 * @brief Initialize filter envelope with musically appropriate defaults
 * 
 * @param sampleRate Audio system sample rate for timing calculations
 * 
 * Constructs envelope with conservative default parameters suitable for
 * general musical applications without requiring immediate adjustment.
 * The defaults provide classic analog synthesizer character while
 * maintaining timbral stability across diverse musical contexts.
 */
MoogFilterEnvelope::MoogFilterEnvelope(float sampleRate) : envDepth(1.0f) {
    /**
     * Initialize envelope to idle state
     * Ensures clean startup without residual envelope activity
     */
    envelope.reset();
    
    /**
     * Configure default envelope timing (sample-based)
     * Values chosen for musical utility and analog synthesizer character
     */
    envelope.setAttackRate(0.01f * sampleRate);    // 10ms attack - quick response
    envelope.setDecayRate(0.1f * sampleRate);      // 100ms decay - moderate character
    envelope.setSustainLevel(0.75f);               // 75% sustain - bright sustain
    envelope.setReleaseRate(0.2f * sampleRate);    // 200ms release - smooth fade
}

/**
 * @brief Configure ADSR timing parameters with seconds-to-samples conversion
 * 
 * @param attackSec Attack time in seconds
 * @param decaySec Decay time in seconds  
 * @param sustainLvl Sustain level [0.0-1.0]
 * @param releaseSec Release time in seconds
 * 
 * @bug_notice The implementation uses hardcoded 44100.0f sample rate conversion
 * instead of the sampleRate parameter from constructor. This causes timing
 * inaccuracies at other sample rates and should be corrected to use the
 * stored sample rate value for accurate timing across all audio configurations.
 * 
 * @correct_implementation_should_be
 * @code
 * envelope.setAttackRate(attackSec * sampleRate);
 * envelope.setDecayRate(decaySec * sampleRate);
 * envelope.setReleaseRate(releaseSec * sampleRate);
 * @endcode
 */
void MoogFilterEnvelope::setADSR(float attackSec, float decaySec, float sustainLvl, float releaseSec) {
    envelope.setAttackRate(attackSec * 44100.0f);   // BUG: Should use actual sampleRate
    envelope.setDecayRate(decaySec * 44100.0f);     // BUG: Should use actual sampleRate
    envelope.setSustainLevel(sustainLvl);           // Correct: dimensionless level
    envelope.setReleaseRate(releaseSec * 44100.0f); // BUG: Should use actual sampleRate
}

/**
 * @brief Update envelope modulation depth parameter
 * 
 * @param depth New modulation depth value
 * 
 * Simple parameter update providing immediate effect on subsequent
 * process() calls. No validation performed for maximum performance.
 */
void MoogFilterEnvelope::setEnvDepth(float depth) {
    envDepth = depth;
}

/**
 * @brief Trigger envelope gate state change
 * 
 * @param gateState Gate control (1=on, 0=off)
 * @param velocity Velocity parameter (currently unused)
 * 
 * Direct delegation to underlying ADSR envelope for gate control.
 * Velocity parameter provided for interface consistency but not
 * currently implemented for velocity-sensitive envelope behavior.
 */
void MoogFilterEnvelope::gate(int gateState, float velocity) {
    envelope.gate(gateState);
}

/**
 * @brief Generate filter cutoff frequency with envelope modulation
 * 
 * @param cutoffBase Base filter frequency
 * @param keyFollowValue Keyboard tracking contribution
 * @return Combined filter cutoff frequency
 * 
 * @algorithm_steps
 * 1. Process ADSR envelope to get current modulation value [0.0-1.0]
 * 2. Scale by envelope depth for configurable modulation intensity
 * 3. Add to base frequency and keyboard tracking for final result
 * 
 * @performance_analysis
 * - Single envelope.process() call: O(1) complexity
 * - Two floating-point additions: ~1-2 CPU cycles
 * - One floating-point multiplication: ~1 CPU cycle
 * - Total: ~3-5 CPU cycles per sample (excluding envelope processing)
 */
float MoogFilterEnvelope::process(float cutoffBase, float keyFollowValue) {
    /**
     * Process underlying envelope generator
     * Returns normalized envelope value [0.0-1.0]
     */
    float envOut = envelope.process();
    
    /**
     * Debug output (commented for production performance)
     * Uncomment for envelope depth analysis during development
     */
    //rt_printf("ENV DEPTH OUT: %f", envOut * envDepth);
    
    /**
     * Combine all frequency components using additive synthesis
     * Linear frequency addition provides predictable filter behavior
     */
    return (cutoffBase + keyFollowValue) + (envOut * envDepth);
}

