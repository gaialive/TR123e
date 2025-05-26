/**
 * @file VelocityParser.h
 * @brief MIDI velocity analysis and note-on/note-off discrimination
 * 
 * This module implements intelligent MIDI velocity parsing to distinguish between
 * note-on and note-off events based on configurable velocity thresholds. The system
 * addresses ambiguities in MIDI protocol where velocity-zero note-on messages serve
 * as note-off events, and provides hysteresis-based velocity discrimination for
 * improved performance reliability with electronic controllers.
 * 
 * @midi_protocol_background
 * The MIDI specification defines two methods for note termination:
 * 
 * 1. **Explicit Note-Off**: Dedicated note-off message (status byte 8x)
 * 2. **Velocity-Zero Note-On**: Note-on message with velocity = 0 (status byte 9x)
 * 
 * Many MIDI controllers and software implementations use method 2 for efficiency,
 * sending only note-on messages with velocity 0 for note release. Additionally,
 * some controllers may send very low velocity values that should be interpreted
 * as note-off events to prevent unwanted note triggering.
 * 
 * @threshold_DISCRIMINATION_THEORY
 * Electronic keyboards and controllers often exhibit velocity sensing variations
 * due to mechanical tolerances, sensor calibration, and playing technique. A
 * configurable threshold provides:
 * 
 * 1. **False Trigger Prevention**: Eliminates unwanted notes from light key touches
 * 2. **Performance Consistency**: Ensures reliable note triggering across devices
 * 3. **Musical Expression**: Maintains velocity sensitivity above threshold
 * 4. **Compatibility**: Adapts to different controller characteristics
 * 
 * @implementation_philosophy
 * The implementation prioritizes simplicity and reliability over complex heuristics.
 * A single threshold value provides effective discrimination while maintaining
 * predictable behavior across diverse MIDI sources and performance styles.
 * 
 * @performance_characteristics
 * - Computational complexity: O(1) - Single comparison operation
 * - Memory footprint: 4 bytes (single integer threshold)
 * - Real-time safety: No allocation or blocking operations
 * - Deterministic behavior: Consistent output for identical inputs
 * 
 * @author Timothy Paul Read
 * @date 2025/03/10
 * @organization AABSTRKT at play via gaialive.com
 * @copyright 2025 Timothy Paul Read
 * @version 1.0
 * 
 * @license
 * This source code is provided as is, without warranty.
 * You may copy and distribute verbatim copies of this document.
 * You may modify and use this source code to create binary code 
 * for your own purposes, free or commercial.
 */

#pragma once

/**
 * @class VelocityParser
 * @brief MIDI velocity threshold processor for note-on/note-off discrimination
 * 
 * The VelocityParser class provides intelligent MIDI velocity analysis with
 * configurable threshold-based discrimination between note-on and note-off events.
 * It handles MIDI protocol ambiguities and controller variations to ensure reliable
 * note triggering behavior across diverse hardware and software configurations.
 * 
 * @design_principles
 * - **Simplicity**: Single threshold comparison for predictable behavior
 * - **Configurability**: User-adjustable threshold for different controllers
 * - **Reliability**: Consistent discrimination across velocity ranges
 * - **Performance**: Minimal computational overhead for real-time processing
 * 
 * @threshold_selection_guidelines
 * Optimal threshold values depend on MIDI controller characteristics:
 * 
 * - **Conservative (32-48)**: Prevents false triggers, may reduce sensitivity
 * - **Balanced (49-80)**: Good compromise for most controllers and playing styles
 * - **Sensitive (81-100)**: Maximum expression, may allow false triggers
 * - **Aggressive (101-127)**: Only strong velocities trigger notes
 * 
 * @musical_applications
 * - Monophonic synthesizers requiring reliable note triggering
 * - Drum machines with velocity-sensitive sample triggering
 * - Educational software with consistent note response
 * - Live performance systems requiring predictable behavior
 * - MIDI recording applications with clean note discrimination
 * 
 * @usage_example
 * @code
 * VelocityParser parser(64);  // Moderate threshold for balanced response
 * 
 * // In MIDI message processing:
 * if (messageType == kmmNoteOn) {
 *     bool isNoteOn = parser.isNoteOn(velocity);
 *     if (isNoteOn) {
 *         synthesizer.noteOn(noteNumber, velocity);
 *     } else {
 *         synthesizer.noteOff(noteNumber);
 *     }
 * }
 * @endcode
 */
class VelocityParser {
public:
    /**
     * @brief Construct velocity parser with configurable threshold
     * 
     * @param threshold Velocity threshold for note-on discrimination
     *                  Values above threshold trigger note-on events
     *                  Values at or below threshold are interpreted as note-off
     *                  Default: 64 (middle MIDI range, balanced sensitivity)
     *                  Range: [1-126] practical limits for musical applications
     * 
     * @complexity O(1) - Constant time initialization
     * @memory 4 bytes object size (single integer member)
     * @thread_safety Safe for concurrent construction
     * 
     * @threshold_rationale
     * Default value of 64 provides balanced behavior suitable for most
     * musical applications and MIDI controllers. This represents 50% of
     * the MIDI velocity range, offering good sensitivity while preventing
     * false triggering from light touches or controller noise.
     * 
     * @boundary_considerations
     * - Threshold = 0: All non-zero velocities trigger notes (maximum sensitivity)
     * - Threshold = 127: No velocities trigger notes (effectively disabled)
     * - Threshold = 126: Only maximum velocity triggers notes (minimum sensitivity)
     */
    VelocityParser(int threshold = 64);
    
    /**
     * @brief Analyze velocity value and determine note-on status
     * 
     * Performs threshold-based analysis of MIDI velocity values to determine
     * whether the velocity represents a note-on or note-off event. Provides
     * consistent discrimination behavior essential for reliable musical performance.
     * 
     * @param velocity MIDI velocity value for analysis
     *                 Range: [0-127] per MIDI specification
     *                 Values outside range processed without validation
     * 
     * @return true if velocity indicates note-on event (velocity > threshold)
     *         false if velocity indicates note-off event (velocity ≤ threshold)
     * 
     * @complexity O(1) - Single integer comparison
     * @determinism Identical output for identical inputs
     * @realtime_safety Real-time safe (no allocation or blocking)
     * 
     * @discrimination_logic
     * The method implements strict threshold comparison:
     * - velocity > threshold → note-on event
     * - velocity ≤ threshold → note-off event
     * 
     * This provides clean binary discrimination without hysteresis or
     * complex state tracking, ensuring predictable behavior for musical
     * applications requiring consistent note triggering.
     * 
     * @musical_implications
     * Threshold-based discrimination affects musical expression:
     * - Lower thresholds: Greater velocity sensitivity, potential false triggers
     * - Higher thresholds: Reduced sensitivity, consistent triggering
     * - Balanced thresholds: Optimal compromise for most musical contexts
     * 
     * @edge_case_behavior
     * - velocity = 0: Always returns false (standard MIDI note-off)
     * - velocity = threshold: Returns false (at-threshold interpreted as off)
     * - velocity = threshold + 1: Returns true (above-threshold interpreted as on)
     * 
     * @controller_compatibility
     * Different MIDI controllers exhibit varying velocity characteristics:
     * - Weighted keyboards: Generally consistent velocity curves
     * - Synth-action keys: May have different velocity response
     * - Drum pads: Often require lower thresholds for sensitivity
     * - Wind controllers: Continuous velocity, may need special handling
     */
    bool isNoteOn(int velocity);

private:
    /**
     * @brief Velocity threshold for note-on discrimination
     * 
     * Integer threshold value used for binary velocity discrimination.
     * Values above this threshold are interpreted as note-on events,
     * while values at or below are interpreted as note-off events.
     * 
     * @range [0-127] per MIDI velocity specification
     * @default 64 (balanced sensitivity for most applications)
     * @units MIDI velocity units (dimensionless integer)
     */
    int velocityThreshold;
};

