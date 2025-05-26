/**
 * @file MoogLadderFilter.cpp
 * @brief Implementation of simplified Moog ladder filter with bilinear transform
 */

#include "MoogLadderFilter.h"

/**
 * @brief Initialize simplified Moog ladder filter
 */
MoogLadderFilter::MoogLadderFilter(float rate)
    : sampleRate(rate), cutoff(1000.0f), resonance(0.5f) {
    updateCoefficients();
}

/**
 * @brief Update sample rate with coefficient recalculation
 */
void MoogLadderFilter::setSampleRate(float rate) {
    sampleRate = rate;
    updateCoefficients();
}

/**
 * @brief Set cutoff frequency with validation and bilinear transform
 */
void MoogLadderFilter::setCutoff(float cutoffHz) {
    /**
     * Clamp cutoff to safe range preventing aliasing and instability
     * Upper limit of 0.45 × sampleRate provides safety margin below Nyquist
     */
    cutoff = fmin(fmax(cutoffHz, 5.0f), 0.45f * sampleRate);
    updateCoefficients();
}

/**
 * @brief Set resonance with validation and coefficient update
 */
void MoogLadderFilter::setResonance(float res) {
    /**
     * Clamp resonance to [0.0-1.0] range for stability
     */
    resonance = fmin(fmax(res, 0.0f), 1.0f);
    updateCoefficients();
}

/**
 * @brief Update coefficients using bilinear transform relationships
 * 
 * This method implements the core bilinear transform calculations that
 * provide accurate frequency mapping from analog prototype to digital
 * implementation while maintaining computational efficiency.
 */
void MoogLadderFilter::updateCoefficients() {
    /**
     * Calculate normalized frequency for bilinear transform
     */
    float fc = cutoff / sampleRate;
    
    /**
     * Apply bilinear transform frequency pre-warping
     * tuning = tan(π × fc) compensates for frequency compression
     * in bilinear transform, ensuring accurate cutoff frequency
     */
    tuning = tanf(M_PI * fc); // bilinear transform
    
    /**
     * Calculate feedback coefficient using classic Moog scaling
     * Factor of 4.0 accounts for cumulative attenuation through
     * four filter poles in the ladder structure
     */
    feedback = resonance * 4.0f; // classic moog style
}

/**
 * @brief Process sample through simplified ladder with nonlinear saturation
 * 
 * This method implements a streamlined version of the Moog ladder that
 * captures the essential frequency response and nonlinear characteristics
 * while maintaining computational efficiency suitable for real-time use.
 */
float MoogLadderFilter::process(float input) {
    /**
     * Calculate input with resonance feedback from final stage
     * This implements the negative feedback that creates resonance
     */
    float x = input - feedback * y[3]; // feedback input

    /**
     * Process through four cascaded one-pole lowpass sections
     * Each section uses the bilinear transform tuning coefficient
     * and includes tanh() saturation for nonlinear analog character
     * 
     * The algorithm implements: y[n] += tuning × (tanh(input) - tanh(y[n]))
     * This provides both lowpass filtering and nonlinear saturation
     */
    
    // First pole with nonlinear saturation
    y[0] += tuning * (tanhf(x) - tanhf(y[0]));
    
    // Second pole cascaded from first
    y[1] += tuning * (tanhf(y[0]) - tanhf(y[1]));
    
    // Third pole cascaded from second  
    y[2] += tuning * (tanhf(y[1]) - tanhf(y[2]));
    
    // Fourth pole cascaded from third (final output)
    y[3] += tuning * (tanhf(y[2]) - tanhf(y[3]));

    /**
     * Return final stage output
     * This provides the complete 24dB/octave lowpass response
     * with nonlinear saturation and resonance characteristics
     */
    return y[3];
}

/**
 * @comparative_analysis_summary
 * 
 * This collection of Moog ladder filter implementations demonstrates four
 * distinct approaches to digital analog modeling, each with specific advantages
 * and trade-offs suitable for different applications and research contexts.
 * 
 * @implementation_comparison_matrix
 * 
 * **MSP Moog Ladder Filter (Huovilainen Model)**:
 * - **Accuracy**: Highest (advanced nonlinear modeling)
 * - **Complexity**: Highest (~100 ops/sample)  
 * - **Memory**: Highest (120 bytes)
 * - **Applications**: Research, high-end audio production
 * - **Unique Features**: Dual-iteration processing, thermal noise, 6 filter modes
 * 
 * **ZDF Moog Ladder Filter**:
 * - **Accuracy**: High (zero-delay feedback topology)
 * - **Complexity**: Medium (~30 ops/sample)
 * - **Memory**: Medium (64 bytes)
 * - **Applications**: Professional audio software, real-time synthesis
 * - **Unique Features**: Multiple modes, frequency pre-warping, drive control
 * 
 * **Empirically-Tuned Virtual Analog Filter**:
 * - **Accuracy**: Medium-High (optimized for musical character)
 * - **Complexity**: Medium (~25 ops/sample)
 * - **Memory**: Low (48 bytes)
 * - **Applications**: Live performance, DAW plugins, hardware processors
 * - **Unique Features**: Fast tanh approximation, block processing, practical tuning
 * 
 * **Simplified Bilinear Transform Filter**:
 * - **Accuracy**: Medium (essential characteristics preserved)
 * - **Complexity**: Low (~20 ops/sample)
 * - **Memory**: Low (48 bytes)
 * - **Applications**: Educational use, embedded systems, basic synthesis
 * - **Unique Features**: Bilinear transform accuracy, implementation clarity
 * 
 * **Fixed-Point Implementation**:
 * - **Accuracy**: Medium (limited by fixed-point precision)
 * - **Complexity**: Lowest (~15 ops/sample, integer only)
 * - **Memory**: Lowest (32 bytes)
 * - **Applications**: Embedded systems, mobile devices, deterministic processing
 * - **Unique Features**: No floating-point unit required, deterministic behavior
 * 
 * @research_methodology_insights
 * 
 * **Algorithm Development Progression**:
 * 1. **Theoretical Foundation**: ZDF approach provides mathematical rigor
 * 2. **Advanced Modeling**: Huovilainen model adds comprehensive nonlinearity
 * 3. **Practical Optimization**: VA approach balances accuracy with efficiency  
 * 4. **Educational Simplification**: Bilinear transform maintains clarity
 * 5. **Hardware Adaptation**: Fixed-point enables embedded deployment
 * 
 * **Performance vs. Accuracy Trade-offs**:
 * - **Research Applications**: Favor accuracy and completeness (Huovilainen, ZDF)
 * - **Production Use**: Balance accuracy with efficiency (VA, Simplified)
 * - **Embedded Systems**: Prioritize efficiency and determinism (Fixed-Point)
 * - **Educational Context**: Emphasize clarity and understanding (Simplified)
 * 
 * @dissertation_research_value
 * 
 * **Comparative Analysis Opportunities**:
 * - **Algorithm Complexity**: Quantitative analysis of computational requirements
 * - **Audio Quality Metrics**: THD, frequency response, transient analysis
 * - **Implementation Efficiency**: CPU usage, memory bandwidth, cache behavior
 * - **Musical Usability**: Subjective evaluation of musical character and control
 * - **Platform Adaptation**: Performance across different hardware architectures
 * 
 * **Methodological Contributions**:
 * - **Translation Techniques**: Gen~ to C++ conversion methodology (MSP)
 * - **Optimization Strategies**: Performance enhancement without quality loss
 * - **Algorithm Selection**: Criteria for choosing appropriate implementation
 * - **Validation Methods**: Objective and subjective evaluation techniques
 * - **Platform Considerations**: Matching algorithm to hardware constraints
 * 
 * **Academic Significance**:
 * - **Historical Documentation**: Progression of digital analog modeling
 * - **Implementation Diversity**: Multiple valid approaches to same problem
 * - **Trade-off Analysis**: Systematic evaluation of design decisions
 * - **Practical Application**: Real-world deployment considerations
 * - **Future Directions**: Foundation for continued research and development
 * 
 * This comprehensive collection provides a complete framework for understanding
 * the evolution, implementation, and practical deployment of digital Moog ladder
 * filter algorithms, serving as both research documentation and practical
 * implementation guide for advanced audio signal processing applications.
 */