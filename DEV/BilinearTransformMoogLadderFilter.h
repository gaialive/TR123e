/**
 * @file MoogLadderFilter.h (Inferred) 
 * @brief Simplified Moog ladder filter with bilinear transform implementation
 * 
 * This implementation represents a streamlined approach to Moog ladder filter
 * modeling that emphasizes simplicity and computational efficiency over complex
 * nonlinear modeling. It uses the bilinear transform for frequency warping
 * compensation and implements a straightforward four-pole cascade structure.
 * 
 * @design_philosophy
 * - **Simplicity**: Minimal complexity while maintaining essential Moog character
 * - **Efficiency**: Optimized for real-time processing with minimal overhead
 * - **Stability**: Robust behavior across parameter ranges
 * - **Clarity**: Clean algorithm suitable for educational applications
 * - **Predictability**: Consistent behavior without complex nonlinear interactions
 * 
 * @implementation_approach
 * This filter uses a simplified ladder topology that captures the essential
 * frequency response characteristics of the Moog ladder while avoiding the
 * computational complexity of advanced nonlinear modeling. The approach
 * provides excellent CPU efficiency while maintaining musical utility.
 * 
 * @bilinear_transform_application
 * The implementation uses the bilinear transform for frequency warping:
 * - **Frequency Pre-warping**: tan(π × fc / fs) compensation
 * - **S-plane to Z-plane**: Accurate analog-to-digital mapping
 * - **Frequency Accuracy**: Maintains cutoff precision across sample rates
 * - **Phase Response**: Preserves analog-like phase characteristics
 */

#include <cmath>

/**
 * @class MoogLadderFilter
 * @brief Simplified Moog ladder filter with bilinear transform accuracy
 * 
 * This class provides a straightforward implementation of the Moog ladder filter
 * using bilinear transform frequency mapping and simplified cascade topology.
 * The design prioritizes computational efficiency and implementation clarity
 * while maintaining the essential musical characteristics of the Moog sound.
 * 
 * @key_features
 * - **Bilinear Transform**: Accurate frequency mapping from analog prototype
 * - **Four-Pole Cascade**: Traditional ladder structure with resonance feedback
 * - **Nonlinear Saturation**: tanh() saturation at each stage for analog character
 * - **Simple Topology**: Streamlined algorithm for educational and practical use
 * - **Parameter Validation**: Automatic range checking for stability
 * 
 * @computational_characteristics
 * - **Complexity**: ~20 floating-point operations per sample
 * - **Memory**: ~48 bytes (12 float members)
 * - **Stability**: Robust across all parameter ranges
 * - **Precision**: Single-precision floating-point throughout
 * - **Efficiency**: Optimized for real-time processing
 */
class MoogLadderFilter {
public:
    /**
     * @brief Construct simplified Moog ladder filter
     * 
     * @param rate Sample rate for bilinear transform calculations
     * 
     * Initializes filter with default parameters and computes initial
     * coefficients using bilinear transform frequency mapping.
     */
    MoogLadderFilter(float rate);
    
    /**
     * @brief Update sample rate and recalculate coefficients
     * 
     * @param rate New sample rate in Hz
     * 
     * Allows dynamic sample rate changes with automatic coefficient
     * recalculation to maintain frequency accuracy.
     */
    void setSampleRate(float rate);
    
    /**
     * @brief Set cutoff frequency with bilinear transform compensation
     * 
     * @param cutoffHz Cutoff frequency in Hz
     * 
     * Configures the filter cutoff frequency using bilinear transform
     * for accurate frequency mapping from analog prototype to digital
     * implementation.
     * 
     * @range [5Hz, 0.45 × sampleRate] automatically enforced
     * @transform Uses tan(π × fc / fs) for frequency pre-warping
     */
    void setCutoff(float cutoffHz);
    
    /**
     * @brief Set resonance amount with classic Moog scaling
     * 
     * @param res Resonance amount [0.0-1.0]
     * 
     * Configures filter resonance using classic Moog-style scaling
     * where feedback = resonance × 4.0 to account for the four-pole
     * attenuation in the ladder structure.
     * 
     * @range [0.0-1.0] automatically enforced
     * @scaling feedback = res × 4.0 (classic Moog relationship)
     */
    void setResonance(float res);
    
    /**
     * @brief Process single audio sample through simplified ladder
     * 
     * @param input Input audio sample
     * @return Filtered audio sample
     * 
     * Core processing method implementing the four-pole cascade with
     * nonlinear saturation and resonance feedback. Uses simplified
     * topology for computational efficiency.
     * 
     * @algorithm_steps
     * 1. Calculate input with resonance feedback
     * 2. Process through four cascaded poles with tanh() saturation
     * 3. Update state variables for next sample
     * 4. Return final stage output
     */
    float process(float input);

private:
    /**
     * @brief Audio system sample rate
     */
    float sampleRate;
    
    /**
     * @brief Current cutoff frequency in Hz
     */
    float cutoff;
    
    /**
     * @brief Current resonance amount [0.0-1.0]
     */
    float resonance;
    
    /**
     * @brief Bilinear transform tuning coefficient
     * 
     * Computed as tan(π × cutoff / sampleRate) for frequency
     * pre-warping compensation in bilinear transform.
     */
    float tuning;
    
    /**
     * @brief Resonance feedback coefficient
     * 
     * Calculated as resonance × 4.0 using classic Moog scaling
     * to account for four-pole attenuation in feedback path.
     */
    float feedback;
    
    /**
     * @brief Filter stage state variables [4 elements]
     * 
     * Internal state of each filter pole in the cascade structure.
     * These maintain the temporal memory of the filter.
     */
    float y[4] = {0.0f};
    
    /**
     * @brief Update filter coefficients when parameters change
     * 
     * Recalculates tuning and feedback coefficients using bilinear
     * transform relationships when cutoff or resonance changes.
     */
    void updateCoefficients();
};

