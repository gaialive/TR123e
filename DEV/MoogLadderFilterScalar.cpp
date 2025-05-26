/**
 * @file MoogLadderFilterScalar.cpp
 * @brief Implementation of direct Gen~ code translation
 * 
 * This implementation provides exact scalar replication of the Max/MSP Gen~
 * moogLadderFilter code, serving as the reference standard for all optimized
 * versions while maintaining perfect algorithmic fidelity to the original.
 */

#include "MoogLadderFilterBase.h"
#include <cmath>

/**
 * @brief Initialize scalar filter with Gen~ compatible setup
 */
MoogLadderFilterScalar::MoogLadderFilterScalar(float sampleRate) {
    setSampleRate(sampleRate);
    reset();
}

/**
 * @brief Reset all state variables to exact Gen~ defaults
 * 
 * Implements exact state clearing as performed by Gen~ reset,
 * ensuring identical initialization behavior across implementations.
 */
void MoogLadderFilterScalar::reset() {
    s1 = s2 = s3 = s4 = s5 = s6 = s7 = s8 = 0.0f;
    slim = 0.0f;
    previn = 0.0f;
    rc = 0.0f;
    fc = 1.0f;
    mode = 0;
}

/**
 * @brief Update sample rate with exact Gen~ coefficient calculation
 * 
 * Implements the exact mathematical operations performed in Gen~
 * for sample rate dependent coefficient generation.
 */
void MoogLadderFilterScalar::setSampleRate(float sr_) {
    sr = sr_;
    expr1 = sqrtf(clamp(12.5f / sr, 0.0001f, 1.0f));
    expr2 = -logf(expr1);
}

/**
 * @brief Set parameters with exact Gen~ parameter interpretation
 * 
 * Maintains exact parameter processing including range validation
 * and internal format conversion as performed in original Gen~ code.
 */
void MoogLadderFilterScalar::setParams(float cutoffHz, float resonanceVal, int modeSel) {
    mode = modeSel;
    rc = clamp(resonanceVal, 0.0f, 1.0f);
    fc = fixdenorm(cutoffHz);
}

/**
 * @brief Process sample with complete Gen~ algorithm implementation
 * 
 * This method implements every mathematical operation from the original
 * Gen~ code in exactly the same order with identical coefficients,
 * ensuring bit-exact compatibility with the visual programming version.
 */
float MoogLadderFilterScalar::process(float in1, float in2, float in3, float in4) {
    /**
     * SECTION 1: INPUT CONDITIONING (Exact Gen~ sequence)
     * Add thermal noise and process envelope control exactly as in Gen~
     */
    float expr10 = in1 + (1e-11f * in4);

    /**
     * SECTION 2: RESONANCE MODULATION (Gen~ envelope shaping)
     * Process resonance control with envelope modulation using exact coefficients
     */
    float rc_mod = rc + clamp(((1.05f * std::max(in3, 1e-5f)) - rc) / 4.0f, -1.0f, 1.0f);

    /**
     * SECTION 3: COEFFICIENT CALCULATION (Exact Gen~ polynomial)
     * Calculate filter coefficients using exact polynomial approximation
     * from original Huovilainen model as implemented in Gen~
     */
    float expr3 = fc * fc;
    float expr4 = expr3 * (1.0f - rc_mod);
    float expr5 = expr3 + (expr4 * expr4);
    float expr6 = (1.25f + ((-0.74375f + (0.3f * expr5)) * expr5)) * expr5;
    float expr7 = rc_mod * (1.4f + ((0.108f + ((-0.164f - 0.069f * expr6) * expr6)) * expr6));
    float expr8 = 0.18f + 0.25f * (expr7 * expr7);
    float rsub9 = 1.0f - expr6;

    /**
     * SECTION 4: FIRST ITERATION (Gen~ initial processing)
     * Implement first pass through ladder with nonlinear saturation
     * using exact saturation curve and coefficient application
     */
    float expr12 = fixdenorm(previn) * expr8 - expr7 * s5;
    float expr13 = clamp(((0.062f * expr12) * expr12 + (0.993f * slim)), -1.0f, 1.0f);
    float expr14 = expr12 * ((1.0f - expr13) + ((0.5f * expr13) * expr13));
    float expr15 = expr14 * expr6 + rsub9 * s1;
    float add22 = expr15 + s1 * 0.3f;
    float expr23 = add22 * expr6 + rsub9 * s2;
    float add27 = expr23 + s2 * 0.3f;
    float clamp28 = clamp(add27, -1.0f, 1.0f);
    float expr29 = clamp28 * (1.0f - (0.3333333f * clamp28 * clamp28));
    float expr30 = expr29 * expr6 + rsub9 * s3;
    float add31 = expr30 + s3 * 0.3f;
    float expr32 = add31 * expr6 + rsub9 * s4;
    float add33 = expr32 + s4 * 0.3f;

    /**
     * SECTION 5: SECOND ITERATION (Gen~ accuracy refinement)
     * Implement second pass for improved nonlinear accuracy
     * using refined feedback and state calculations
     */
    float expr34 = expr10 * expr8 - expr7 * add33;
    float expr35 = clamp(((0.062f * expr34) * expr34 + (0.993f * expr13)), -1.0f, 1.0f);
    float expr36 = expr34 * ((1.0f - expr35) + ((0.5f * expr35) * expr35));
    float expr37 = expr36 * expr6 + rsub9 * expr15;
    float add38 = expr37 + expr15 * 0.3f;
    float expr39 = add38 * expr6 + rsub9 * expr23;
    float add40 = expr39 + expr23 * 0.3f;
    float clamp42 = clamp(add40, -1.0f, 1.0f);
    float expr43 = clamp42 * (1.0f - (0.3333333f * clamp42 * clamp42));
    float expr44 = expr43 * expr6 + rsub9 * expr30;
    float add48 = expr44 + expr30 * 0.3f;
    float expr49 = add48 * expr6 + rsub9 * expr32;
    float add50 = expr49 + expr32 * 0.3f;

    /**
     * SECTION 6: MODE CALCULATIONS (Complete Gen~ mode set)
     * Calculate all six filter modes using exact Gen~ coefficients
     * and signal combination methods
     */
    float expr51 = (0.19f * (add50 + s8)) + (0.57f * (add33 + s7)) - (0.52f * s6);
    float expr52 = (expr14 - 4.0f * (add38 + add48) + 6.0f * add40) + expr51;
    float expr53 = 4.0f * (add40 + expr51) - 8.0f * add48;
    float expr45 = (expr14 - 2.0f * add38) + add40;
    float expr41 = 2.0f * (add38 - add40);

    /**
     * SECTION 7: MODE SELECTION (Gen~ switch implementation)
     * Select appropriate output based on mode parameter
     * using exact Gen~ mode indexing and default behavior
     */
    float output;
    switch(mode) {
        case 0: output = expr51; break;  // LP24: 24dB/octave lowpass
        case 1: output = expr52; break;  // HP24: 24dB/octave highpass
        case 2: output = expr53; break;  // BP24: 24dB/octave bandpass
        case 3: output = add40; break;   // LP18: 18dB/octave lowpass
        case 4: output = expr45; break;  // BP18: 18dB/octave bandpass
        case 5: output = expr41; break;  // HP6: 6dB/octave highpass
        default: output = add40; break;  // Default fallback
    }

    /**
     * SECTION 8: STATE UPDATES (Exact Gen~ state management)
     * Update all state variables in exact order as performed
     * by Gen~ history operator updates
     */
    previn = expr10;
    slim = expr35;
    s1 = expr15; s2 = expr39; s3 = expr44; s4 = expr49;
    s5 = add50; s6 = expr51; s7 = add50; s8 = add33;

    return output;
}

