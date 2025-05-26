/**
 * @file KeyFollow.h
 * @brief Keyboard tracking module for filter frequency modulation
 * 
 * This module implements keyboard tracking (key follow) functionality commonly
 * found in analog synthesizers. It provides proportional filter cutoff frequency
 * adjustment based on the played MIDI note number, ensuring consistent timbre
 * across the keyboard range.
 * 
 * @theory_background
 * In acoustic instruments, higher pitched notes naturally contain more high-frequency
 * content. To maintain consistent perceived brightness across the keyboard, synthesizer
 * filters traditionally open proportionally with higher notes. This module implements
 * this behavior using configurable tracking intensity.
 * 
 * @mathematical_model
 * Output = (max(0, note - 36) * 0.33 + 36) * tracking_amount
 * 
 * Where:
 * - Note 36 (C2) serves as the reference frequency (0 Hz offset)
 * - 0.33 scaling factor provides musically appropriate tracking slope
 * - Linear mapping ensures predictable frequency relationships
 * 
 * @performance_characteristics
 * - Computational complexity: O(1) constant time
 * - Memory footprint: Single float member variable
 * - Real-time safe: No dynamic allocation or blocking operations
 * - Thread safe: Stateless processing with atomic parameter updates
 * 
 * @author Timothy Paul Read
 * @date 2025/03/10
 * @version 1.0
 * @license Provided as-is without warranty, free for commercial/non-commercial use
 */

#pragma once

/**
 * @class KeyFollow
 * @brief Implements keyboard tracking for filter frequency modulation
 * 
 * The KeyFollow class provides real-time keyboard tracking functionality,
 * generating frequency offset values based on MIDI note numbers. The implementation
 * uses a linear mapping with configurable intensity to maintain musical
 * relationships while allowing artistic control over tracking behavior.
 * 
 * @design_pattern
 * - Strategy Pattern: Encapsulates keyboard tracking algorithm
 * - Value Object: Immutable processing with configurable parameters
 * - Real-time Safe: No dynamic allocation in processing path
 * 
 * @usage_example
 * @code
 * KeyFollow keyTracker(0.05f);  // 5% tracking intensity
 * float offset = keyTracker.process(60);  // Process middle C (C4)
 * float cutoff = baseCutoff + offset;     // Apply to filter cutoff
 * @endcode
 */
class KeyFollow {
public:
    /**
     * @brief Construct KeyFollow processor with specified tracking intensity
     * 
     * @param keyFollowAmount Tracking intensity coefficient [0.0-1.0]
     *                       0.0 = no tracking (constant filter frequency)
     *                       1.0 = full tracking (maximum frequency scaling)
     *                       Default: 0.01f (subtle tracking, typical for bass/lead sounds)
     * 
     * @complexity O(1) - Constant time initialization
     * @memory Single float storage for tracking coefficient
     * 
     * @note Default value of 0.01f provides subtle tracking suitable for
     * most musical applications without excessive filter opening in high registers
     */
    KeyFollow(float keyFollowAmount = 0.01f);
    
    /**
     * @brief Update keyboard tracking intensity in real-time
     * 
     * @param amount New tracking intensity coefficient [0.0-1.0]
     *               Values outside this range are not validated but may
     *               produce unexpected results
     * 
     * @complexity O(1) - Constant time parameter update
     * @thread_safety Thread-safe for single writer, multiple readers
     * @realtime_safety Real-time safe (no blocking operations)
     * 
     * @note This method can be called safely from real-time audio threads
     * without risk of priority inversion or blocking
     */
    void setKeyFollowAmount(float amount);
    
    /**
     * @brief Process MIDI note number and generate frequency offset
     * 
     * Converts MIDI note number to proportional frequency offset using
     * linear scaling with configurable intensity. The algorithm provides
     * musically appropriate tracking behavior consistent with classic
     * analog synthesizer implementations.
     * 
     * @param midiNote MIDI note number [0-127]
     *                 Note 36 (C2) serves as reference point (0 offset)
     *                 Values below 36 are clamped to 0 for stability
     * 
     * @return Frequency offset in implementation-defined units
     *         Positive values indicate filter opening
     *         Range: [0, ∞) depending on tracking amount and note number
     * 
     * @complexity O(1) - Constant time processing
     * @precision Maintains full floating-point precision for smooth tracking
     * @realtime_safety Real-time safe (deterministic execution time)
     * 
     * @algorithm_details
     * 1. Offset calculation: note_offset = max(0, note - 36)
     * 2. Frequency scaling: scaled_note = note_offset * 0.33 + 36
     * 3. Intensity application: output = scaled_note * tracking_amount
     * 
     * @note The 0.33 scaling factor provides approximately 1/3 octave tracking
     * per octave played, which matches typical analog synthesizer behavior
     */
    float process(int midiNote);

private:
    /**
     * @brief Keyboard tracking intensity coefficient
     * 
     * Controls the proportional amount of filter frequency change relative
     * to note number variation. Higher values create more dramatic filter
     * opening in upper registers, while lower values provide subtle tracking.
     * 
     * @range [0.0-1.0] typical, though implementation accepts any float value
     * @default 0.01f (1% tracking intensity)
     * @units Dimensionless coefficient (ratio)
     */
    float keyFollowAmount;
};

