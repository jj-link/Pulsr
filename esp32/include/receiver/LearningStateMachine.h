#ifndef LEARNING_STATE_MACHINE_H
#define LEARNING_STATE_MACHINE_H

#include "ISignalCapture.h"
#include "IProtocolDecoder.h"
#include <functional>

enum class LearningState {
    IDLE,       // Normal operation
    LEARNING,   // Waiting for IR signal
    CAPTURED,   // Signal received
    TIMEOUT     // No signal after timeout period
};

// Callback function type for state changes
using StateChangeCallback = std::function<void(LearningState)>;

// Callback function type for captured signals
using SignalCaptureCallback = std::function<void(const DecodedSignal&)>;

class LearningStateMachine {
public:
    LearningStateMachine(
        ISignalCapture* signalCapture,
        IProtocolDecoder* decoder,
        uint32_t timeoutMs = 30000
    );

    // State management
    void startLearning();
    void stopLearning();
    void update();  // Call this in main loop
    
    // State queries
    LearningState getState() const { return currentState; }
    bool isLearning() const { return currentState == LearningState::LEARNING; }
    
    // Callbacks
    void onStateChange(StateChangeCallback callback) { stateChangeCallback = callback; }
    void onSignalCapture(SignalCaptureCallback callback) { signalCaptureCallback = callback; }

private:
    ISignalCapture* signalCapture;
    IProtocolDecoder* decoder;
    
    LearningState currentState;
    uint32_t timeoutMs;
    unsigned long learningStartTime;
    
    StateChangeCallback stateChangeCallback;
    SignalCaptureCallback signalCaptureCallback;
    
    void setState(LearningState newState);
    void handleLearningState();
};

#endif
