#include "receiver/LearningStateMachine.h"
#include <Arduino.h>

LearningStateMachine::LearningStateMachine(
    ISignalCapture* signalCapture,
    IProtocolDecoder* decoder,
    uint32_t timeoutMs
) : signalCapture(signalCapture),
    decoder(decoder),
    currentState(LearningState::IDLE),
    timeoutMs(timeoutMs),
    learningStartTime(0),
    stateChangeCallback(nullptr),
    signalCaptureCallback(nullptr)
{
}

void LearningStateMachine::startLearning() {
    if (currentState != LearningState::IDLE) {
        return; // Already learning or processing
    }
    
    learningStartTime = millis();
    signalCapture->resume();
    setState(LearningState::LEARNING);
}

void LearningStateMachine::stopLearning() {
    if (currentState == LearningState::LEARNING) {
        setState(LearningState::IDLE);
    }
}

void LearningStateMachine::update() {
    if (currentState == LearningState::LEARNING) {
        handleLearningState();
    }
}

void LearningStateMachine::setState(LearningState newState) {
    if (currentState != newState) {
        currentState = newState;
        
        if (stateChangeCallback) {
            stateChangeCallback(newState);
        }
    }
}

void LearningStateMachine::handleLearningState() {
    // Check for timeout
    if (millis() - learningStartTime > timeoutMs) {
        setState(LearningState::TIMEOUT);
        setState(LearningState::IDLE); // Auto-return to idle after timeout
        return;
    }
    
    // Check for signal
    decode_results results;
    if (signalCapture->decode(&results)) {
        // Signal captured! Decode it
        DecodedSignal signal = decoder->decode(&results);
        
        setState(LearningState::CAPTURED);
        
        // Notify callback
        if (signalCaptureCallback) {
            signalCaptureCallback(signal);
        }
        
        // Resume receiver for next capture
        signalCapture->resume();
        
        // Return to idle
        setState(LearningState::IDLE);
    }
}
