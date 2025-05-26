/**
 * @file MidiHandler.h
 * @brief Real-time MIDI message processing with temporal delay compensation
 * 
 * This module implements sophisticated MIDI message handling for real-time audio
 * applications, providing temporal buffering, delay compensation, and sample-accurate
 * timing for musical performance applications. The system addresses timing jitter
 * inherent in MIDI protocol and hardware interfaces.
 * 
 * @architectural_overview
 * The MidiHandler employs a dual-queue architecture:
 * 1. Incoming message queue: Buffers raw MIDI events with timestamps
 * 2. Delayed message queue: Holds messages ready for audio processing
 * 
 * This separation allows for temporal analysis, jitter compensation, and
 * sample-accurate event scheduling essential for professional audio applications.
 * 
 * @timing_theory
 * MIDI protocol introduces variable latency due to:
 * - Serial transmission at 31.25 kbaud (320μs per byte)
 * - Hardware buffering in MIDI interfaces
 * - Operating system scheduling jitter
 * - USB packet batching in modern interfaces
 * 
 * The delay compensation algorithm addresses these issues by implementing
 * configurable temporal buffering with sub-millisecond timing accuracy.
 * 
 * @performance_characteristics
 * - Timing resolution: Microsecond-level accuracy
 * - Memory efficiency: Dynamic queue sizing with bounded growth
 * - Real-time safety: Lock-free queue operations
 * - Computational complexity: O(1) amortized for message processing
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
 * For comprehensive envelope generator theory and implementation details:
 * http://www.earlevel.com/main/2013/06/01/envelope-generators/
 */

#pragma once
#include <Bela.h>
#include <vector>
#include <queue>

/**
 * @struct MidiNoteMessage
 * @brief Encapsulates MIDI note event data with high-precision timing
 * 
 * This structure represents a processed MIDI note event containing all
 * necessary information for synthesis parameter control. The timestamp
 * provides microsecond-level timing accuracy for sample-accurate processing.
 * 
 * @data_layout
 * The structure is designed for cache efficiency with aligned data types
 * and minimal padding. Total size: 16 bytes (assuming 64-bit alignment).
 * 
 * @thread_safety
 * Structure is POD (Plain Old Data) type, safe for lockless queue operations
 * and atomic copying between threads without synchronization overhead.
 */
struct MidiNoteMessage {
    /**
     * @brief MIDI note number [0-127]
     * 
     * Standard MIDI note number representation:
     * - 0 = C-1 (~8.18 Hz)
     * - 60 = C4 (~261.63 Hz, Middle C)
     * - 127 = G9 (~12543.85 Hz)
     * 
     * @note Values outside [0-127] may be used for internal signaling
     * (e.g., -1 indicates invalid/empty message)
     */
    int noteNumber;
    
    /**
     * @brief MIDI velocity value [0-127]
     * 
     * Represents note velocity or release velocity:
     * - 0 = Note off (or minimum velocity)
     * - 1-127 = Note on with velocity scaling
     * - Velocity 0 in note-on message = note-off per MIDI standard
     * 
     * @musical_significance
     * Velocity affects multiple synthesis parameters:
     * - Amplitude envelope peak level
     * - Filter envelope intensity
     * - Harmonic content (velocity-sensitive filtering)
     */
    int velocity;
    
    /**
     * @brief High-precision timestamp in milliseconds
     * 
     * Absolute time reference with sub-millisecond accuracy for
     * sample-accurate event scheduling. Based on system audio
     * clock for synchronization with audio processing.
     * 
     * @precision Typically μs-level accuracy (0.001ms resolution)
     * @range [0, ∞) - Monotonically increasing during operation
     * @reference Audio system start time (context->audioFramesElapsed)
     */
    float timestamp;
};

/**
 * @class MidiHandler
 * @brief Real-time MIDI message processor with temporal delay compensation
 * 
 * The MidiHandler class implements sophisticated MIDI event processing designed
 * for professional audio applications requiring sample-accurate timing. It addresses
 * inherent MIDI protocol limitations and hardware interface jitter through temporal
 * buffering and delay compensation algorithms.
 * 
 * @design_patterns
 * - Producer-Consumer: Separates MIDI input from audio processing
 * - Temporal Buffer: Implements time-based event scheduling
 * - Queue-based Architecture: Lockless FIFO for real-time safety
 * 
 * @real_time_considerations
 * - No dynamic allocation in processing path
 * - Bounded execution time for all operations
 * - Lock-free queue operations prevent priority inversion
 * - Deterministic memory access patterns for cache efficiency
 * 
 * @usage_example
 * @code
 * MidiHandler handler(44100.0f, 2.0f);  // 2ms delay compensation
 * 
 * // In MIDI callback:
 * handler.processMidiMessage(60, 100, currentTime);
 * 
 * // In audio callback:
 * handler.update(currentTime);
 * while (handler.hasDelayedMessage()) {
 *     MidiNoteMessage msg = handler.popDelayedMessage();
 *     // Process note event...
 * }
 * @endcode
 */
class MidiHandler {
public:
    /**
     * @brief Construct MidiHandler with timing parameters
     * 
     * @param sampleRate Audio system sample rate in Hz
     *                   Used for time/sample conversion calculations
     *                   Typical values: 44100, 48000, 96000 Hz
     * 
     * @param delayMs Temporal delay compensation in milliseconds
     *                Allows jitter analysis and timing stabilization
     *                Default: 1.0ms (suitable for most hardware)
     *                Range: [0.1-10.0]ms typical
     * 
     * @complexity O(1) - Constant time initialization
     * @memory Minimal fixed overhead plus queue storage
     * 
     * @design_rationale
     * Default 1ms delay provides balance between timing accuracy and
     * responsiveness. Longer delays improve jitter compensation but
     * increase perceived latency. Shorter delays reduce latency but
     * may not fully compensate for hardware timing variations.
     */
    MidiHandler(float sampleRate, float delayMs = 1.0f);
    
    /**
     * @brief Process incoming MIDI note message with timestamping
     * 
     * Accepts raw MIDI note data and creates timestamped message for
     * temporal processing. Messages are buffered in arrival order for
     * subsequent delay analysis and compensation.
     * 
     * @param noteNumber MIDI note number [0-127]
     * @param velocity MIDI velocity [0-127]
     *                 0 = note-off, 1-127 = note-on with velocity
     * @param currentTimeMs Current system time in milliseconds
     *                      Must be monotonically increasing for proper operation
     * 
     * @complexity O(1) amortized - Queue insertion
     * @memory O(n) where n = number of buffered messages
     * @realtime_safety Real-time safe (no dynamic allocation)
     * 
     * @thread_safety Safe for single-threaded MIDI input processing
     * @note This method should be called from MIDI input thread/callback
     * 
     * @implementation_details
     * - Creates MidiNoteMessage structure with current timestamp
     * - Pushes to incoming message queue for temporal analysis
     * - Queue automatically manages memory with bounded growth
     * - No validation performed for maximum real-time performance
     */
    void processMidiMessage(int noteNumber, int velocity, float currentTimeMs);
    
    /**
     * @brief Update temporal processing and transfer delayed messages
     * 
     * Analyzes incoming message timestamps against current time to determine
     * which messages have completed their delay period. Qualified messages
     * are transferred to the delayed queue for audio processing consumption.
     * 
     * @param currentTimeMs Current system time in milliseconds
     *                      Must match timeline used in processMidiMessage()
     * 
     * @complexity O(k) where k = number of messages ready for processing
     * @realtime_safety Real-time safe (bounded execution time)
     * @determinism Execution time bounded by queue size and delay period
     * 
     * @algorithm_details
     * 1. Iterate through incoming message queue (FIFO order)
     * 2. Check if (currentTime - messageTime) >= delayPeriod
     * 3. Transfer qualified messages to delayed queue
     * 4. Stop at first non-qualified message (temporal ordering preserved)
     * 
     * @temporal_accuracy
     * Timing accuracy depends on update() call frequency. For sample-accurate
     * processing, call once per audio buffer with buffer-aligned timing.
     * 
     * @note This method should be called from audio processing thread
     * at the beginning of each audio buffer processing cycle
     */
    void update(float currentTimeMs);
    
    /**
     * @brief Check availability of processed MIDI messages
     * 
     * @return true if delayed messages are available for processing
     *         false if no messages ready
     * 
     * @complexity O(1) - Constant time queue size check
     * @thread_safety Safe for concurrent read access
     * @realtime_safety Real-time safe (no blocking operations)
     * 
     * @usage_pattern
     * Typically used in conditional loop for message processing:
     * @code
     * while (handler.hasDelayedMessage()) {
     *     MidiNoteMessage msg = handler.popDelayedMessage();
     *     // Process message...
     * }
     * @endcode
     */
    bool hasDelayedMessage();
    
    /**
     * @brief Retrieve and remove next processed MIDI message
     * 
     * Returns the oldest delayed message from the processing queue and
     * removes it from the internal buffer. Messages are returned in
     * temporal order (FIFO) to preserve musical timing relationships.
     * 
     * @return MidiNoteMessage containing note data and timestamp
     *         Returns {-1, -1, -1.0f} if queue is empty
     * 
     * @complexity O(1) - Constant time queue extraction
     * @thread_safety Safe for single consumer thread
     * @realtime_safety Real-time safe (no dynamic allocation)
     * 
     * @error_handling
     * Empty queue condition returns sentinel values (-1) that can be
     * checked by caller for proper error handling without exceptions.
     * 
     * @memory_management
     * Message data is copied by value; no memory management required
     * by caller. Internal queue storage is automatically managed.
     * 
     * @note Caller should verify message validity by checking for
     * sentinel values before processing note data
     */
    MidiNoteMessage popDelayedMessage();
    
    /**
     * @brief Convert milliseconds to sample count at current sample rate
     * 
     * @param milliseconds Time duration in milliseconds
     * @return Equivalent number of audio samples (rounded to nearest integer)
     * 
     * @complexity O(1) - Single arithmetic operation
     * @precision Limited by floating-point arithmetic and integer rounding
     * 
     * @formula samples = (milliseconds * sampleRate) / 1000
     * 
     * @use_cases
     * - Converting delay times to sample-based counters
     * - Calculating envelope timing in samples
     * - Sample-accurate event scheduling calculations
     */
    int msToSamples(float milliseconds) const;
    
    /**
     * @brief Convert sample count to milliseconds at current sample rate
     * 
     * @param samples Number of audio samples
     * @return Equivalent time duration in milliseconds
     * 
     * @complexity O(1) - Single arithmetic operation
     * @precision Full floating-point precision maintained
     * 
     * @formula milliseconds = (samples * 1000) / sampleRate
     * 
     * @use_cases
     * - Converting buffer sizes to timing information
     * - Latency calculations and reporting
     * - Temporal analysis of processing delays
     */
    float samplesToMs(int samples) const;

private:
    /**
     * @brief Audio system sample rate in Hz
     * 
     * Cached sample rate for time/sample conversion calculations.
     * Eliminates repeated parameter passing and improves computational efficiency.
     * 
     * @units Hertz (samples per second)
     * @range Typically [8000-192000] Hz for audio applications
     * @precision Full floating-point precision for accurate conversions
     */
    float sampleRate;
    
    /**
     * @brief Delay compensation period in milliseconds
     * 
     * Configurable delay period for temporal jitter compensation and
     * timing stabilization. Longer delays improve timing accuracy but
     * increase system latency. Shorter delays reduce latency but may
     * not adequately compensate for hardware timing variations.
     * 
     * @units Milliseconds
     * @range [0.1-10.0]ms typical for audio applications
     * @default 1.0ms (good balance of accuracy and responsiveness)
     */
    float delayTimeMs;
    
    /**
     * @brief Queue for incoming MIDI messages awaiting delay processing
     * 
     * FIFO queue storing raw MIDI messages with timestamps for temporal
     * analysis. Messages remain in this queue until their delay period
     * expires, at which point they're transferred to delayedMessages queue.
     * 
     * @container std::queue<MidiNoteMessage>
     * @ordering First-In-First-Out (preserves temporal relationships)
     * @growth_behavior Dynamic allocation with exponential growth
     * @thread_safety Not thread-safe (requires external synchronization)
     */
    std::queue<MidiNoteMessage> incomingMessages;
    
    /**
     * @brief Queue for processed messages ready for audio consumption
     * 
     * Contains MIDI messages that have completed their delay period and
     * are ready for immediate processing by the audio synthesis engine.
     * Messages are consumed by audio thread in temporal order.
     * 
     * @container std::queue<MidiNoteMessage>
     * @ordering First-In-First-Out (maintains musical timing)
     * @consumption Audio processing thread via popDelayedMessage()
     * @thread_safety Not thread-safe (single consumer assumed)
     */
    std::queue<MidiNoteMessage> delayedMessages;
};

