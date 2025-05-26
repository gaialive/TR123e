/**
 * @file PortamentoFilter.h
 * @brief Legato playing technique detection and portamento triggering logic
 * 
 * This module implements sophisticated analysis of MIDI note patterns to determine
 * when portamento (smooth pitch transitions) should be applied in monophonic synthesis.
 * The system distinguishes between legato and staccato playing techniques through
 * temporal analysis of note overlap patterns, providing musically intelligent
 * portamento behavior that responds to performer intent.
 * 
 * @musical_theory_background
 * Portamento is a musical ornament involving a continuous slide between two pitches,
 * commonly used in string, vocal, and synthesizer performance. In traditional acoustic
 * instruments, portamento occurs naturally during legato playing when the performer
 * transitions between notes without releasing the first. Electronic synthesizers
 * must analyze playing technique to determine when this effect should be applied.
 * 
 * @detection_algorithm
 * The system employs a state machine approach to analyze note timing patterns:
 * 
 * 1. **Overlap Detection**: Identifies when new note-on occurs before previous note-off
 * 2. **Pitch Analysis**: Ensures portamento only triggers for different note values
 * 3. **State Tracking**: Maintains previous note information for comparison
 * 
 * This approach provides musically intelligent behavior that matches performer expectations
 * from acoustic instruments and traditional analog synthesizers.
 * 
 * @performance_characteristics
 * - Computational complexity: O(1) for all operations
 * - Memory footprint: 12 bytes (3 member variables)
 * - Real-time safety: No dynamic allocation or blocking operations
 * - Deterministic execution: Bounded processing time for all inputs
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
 * 
 * @references
 * - "The MIDI Manual" by David Miles Huber
 * - "Synthesizer Programming" by Jim Aikin
 * - MIDI 1.0 Detailed Specification, Section 4.2 (Note Messages)
 * - "Computer Music: Synthesis, Composition, and Performance" by Dodge & Jerse
 */

#pragma once

/**
 * @class PortamentoFilter
 * @brief Musical technique analyzer for intelligent portamento triggering
 * 
 * The PortamentoFilter class implements real-time analysis of MIDI note patterns
 * to determine when portamento should be applied based on legato playing technique
 * detection. This provides musically intelligent behavior that responds appropriately
 * to different performance styles without requiring explicit control messages.
 * 
 * @design_philosophy
 * The implementation prioritizes musical accuracy over technical complexity,
 * using simple but effective heuristics that match performer expectations
 * from acoustic instruments. The algorithm errs toward conservative portamento
 * triggering to avoid unwanted pitch slides during fast passages.
 * 
 * @state_machine_design
 * ```
 * [IDLE] --note_on--> [NOTE_ACTIVE]
 *   |                       |
 *   |                   note_on(different) --> [PORTAMENTO_TRIGGER]
 *   |                       |
 *   +--note_off<------------+
 * ```
 * 
 * @musical_applications
 * - Lead synthesizer voices requiring expressive pitch slides
 * - Bass lines with smooth frequency transitions
 * - Vocal synthesis emulation with natural portamento
 * - Educational software teaching legato technique
 * 
 * @usage_example
 * @code
 * PortamentoFilter filter;
 * 
 * // In MIDI processing loop:
 * bool usePortamento = filter.checkPortamento(noteNumber, isNoteOn, timestamp);
 * if (usePortamento) {
 *     player.noteOn(noteNumber, true);  // Enable smooth transition
 * } else {
 *     player.noteOn(noteNumber, false); // Immediate frequency jump
 * }
 * @endcode
 */
class PortamentoFilter {
public:
    /**
     * @brief Initialize portamento filter with default state
     * 
     * Constructs filter in idle state with no previous note information.
     * All state variables are initialized to safe default values that
     * prevent spurious portamento triggering on first note.
     * 
     * @complexity O(1) - Constant time initialization
     * @memory 12 bytes total object size (platform dependent)
     * @thread_safety Safe for concurrent construction
     */
    PortamentoFilter();
    
    /**
     * @brief Analyze note event and determine portamento requirement
     * 
     * Processes incoming MIDI note events to determine whether portamento
     * should be applied based on legato playing technique detection.
     * The algorithm analyzes note overlap patterns and pitch relationships
     * to provide musically intelligent portamento behavior.
     * 
     * @param newNote MIDI note number [0-127] for current note event
     * @param noteOn Boolean indicating note-on (true) or note-off (false)
     * @param currentTimeMs Current timestamp in milliseconds for temporal analysis
     * 
     * @return true if portamento should be applied for this note transition
     *         false if note should start immediately without pitch slide
     * 
     * @complexity O(1) - Constant time analysis
     * @determinism Bounded execution time regardless of input values
     * @realtime_safety Real-time safe (no blocking operations)
     * 
     * @algorithm_logic
     * For note-on events:
     * 1. Check if previous note is still active (overlap condition)
     * 2. Verify new note differs from previous (pitch change requirement)
     * 3. Update internal state with new note information
     * 4. Return portamento decision based on overlap AND pitch change
     * 
     * For note-off events:
     * 1. Mark previous note as inactive
     * 2. Record note-off timestamp for future temporal analysis
     * 3. Return false (portamento not applicable to note-off)
     * 
     * @musical_considerations
     * - Portamento only triggers for overlapping notes with different pitches
     * - Same-note retriggering does not activate portamento (trill protection)
     * - Note-off events update state but never trigger portamento
     * - First note after silence never triggers portamento (no source pitch)
     * 
     * @edge_cases
     * - Rapid note repetition: Prevented by pitch difference requirement
     * - Simultaneous note-on/note-off: Handled by state precedence rules
     * - Invalid MIDI note numbers: Processed without validation for performance
     */
    bool checkPortamento(int newNote, bool noteOn, float currentTimeMs);

private:
    /**
     * @brief Previous MIDI note number for comparison analysis
     * 
     * Stores the MIDI note number of the most recently active note for
     * pitch comparison with incoming notes. Used to determine if portamento
     * should be applied based on pitch interval requirements.
     * 
     * @range [0-127] for valid MIDI notes, -1 for uninitialized state
     * @initialization -1 (no previous note available)
     * @update_policy Updated on every note-on event
     */
    int previousNote;
    
    /**
     * @brief Previous note activity state indicator
     * 
     * Boolean flag indicating whether the previous note is still conceptually
     * "active" from a musical perspective. This determines overlap conditions
     * for legato detection and portamento triggering logic.
     * 
     * @states 
     * - true: Previous note still active (sustaining or in release phase)
     * - false: No active previous note (idle or after note-off)
     * 
     * @musical_significance
     * Represents the performer's intent regarding note connection. Active
     * state indicates potential for legato playing and portamento application.
     */
    bool previousNoteActive;
    
    /**
     * @brief Timestamp of most recent note-off event
     * 
     * Records the precise timing of the last note-off event for potential
     * future use in temporal analysis algorithms. Currently used for state
     * tracking but provides foundation for advanced timing-based heuristics.
     * 
     * @units Milliseconds (float precision for sub-millisecond accuracy)
     * @range [0.0, ∞) - Monotonically increasing timestamps
     * @precision Limited by system timer resolution and float representation
     * 
     * @future_applications
     * - Gap-based portamento decisions (short gaps = portamento)
     * - Velocity-sensitive portamento intensity
     * - Timing-based performance style analysis
     */
    float previousNoteOffTime;
};

