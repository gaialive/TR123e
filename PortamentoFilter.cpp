/**
 * @file PortamentoFilter.cpp
 * @brief Implementation of legato detection and portamento triggering logic
 * 
 * This implementation provides efficient, real-time analysis of MIDI note
 * patterns to enable musically intelligent portamento behavior. The algorithm
 * focuses on simplicity and reliability rather than complex heuristics,
 * ensuring predictable behavior across diverse musical performance styles.
 */

#include "PortamentoFilter.h"

/**
 * @brief Initialize portamento filter to safe default state
 * 
 * Sets all state variables to values that ensure proper behavior on
 * first note event. The initialization prevents spurious portamento
 * triggering when no valid previous note context exists.
 * 
 * @implementation_details
 * - previousNote = -1: Indicates no valid previous note (outside MIDI range)
 * - previousNoteActive = false: Ensures first note won't trigger portamento
 * - previousNoteOffTime = 0.0f: Safe default timestamp value
 */
PortamentoFilter::PortamentoFilter() {
    previousNote = -1;
    previousNoteActive = false;
    previousNoteOffTime = 0.0f;
}

/**
 * @brief Analyze note event and determine portamento application
 * 
 * @param newNote MIDI note number for analysis
 * @param noteOn Note event type (true=on, false=off)
 * @param currentTimeMs Current system timestamp
 * @return Portamento decision for this note transition
 * 
 * @algorithm_implementation
 * The algorithm implements a simple but musically effective state machine
 * that tracks note overlap conditions and pitch relationships to determine
 * when smooth pitch transitions are appropriate.
 * 
 * @performance_analysis
 * - No loops or recursion: O(1) guaranteed execution time
 * - Minimal branching: CPU branch predictor friendly
 * - No function calls: Inline operations for maximum efficiency
 * - Cache efficient: All data in single object instance
 */
bool PortamentoFilter::checkPortamento(int newNote, bool noteOn, float currentTimeMs) {
    /**
     * Local variable for portamento decision
     * Initialized to false (conservative default)
     */
    bool triggerPortamento = false;

    /**
     * Process note-on events for legato analysis
     */
    if (noteOn) {
        /**
         * Check for legato condition: overlapping notes with different pitches
         * 
         * Conditions for portamento triggering:
         * 1. previousNoteActive: Previous note still sustaining (overlap)
         * 2. previousNote != newNote: Different pitch (slide required)
         * 
         * This logic ensures portamento only occurs for musically appropriate
         * transitions where the performer intends smooth pitch connection.
         */
        if (previousNoteActive && previousNote != newNote) {
            /**
             * Notes overlap and are different: trigger portamento
             * This represents classic legato playing technique where
             * the performer connects notes smoothly without gaps.
             */
            triggerPortamento = true;
        } else {
            /**
             * Either no overlap or same note: no portamento
             * Cases covered:
             * - First note (no previous active note)
             * - Staccato playing (gap between notes)
             * - Note retriggering (same pitch, no slide needed)
             */
            triggerPortamento = false;
        }
        
        /**
         * Update state for next iteration
         * Mark new note as active and store its pitch for future comparison
         */
        previousNoteActive = true;
        previousNote = newNote;
    } else {
        /**
         * Process note-off events: update state without triggering portamento
         * 
         * Note-off events never trigger portamento but update internal state
         * to track note activity and timing for subsequent analysis.
         */
        previousNoteActive = false;
        previousNoteOffTime = currentTimeMs;
    }

    return triggerPortamento;
}

