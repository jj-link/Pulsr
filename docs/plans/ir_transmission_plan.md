# Plan: IR Transmission

**Goal:** reliable, low-latency control of physical devices via the Web Interface.

## Functional Requirements

1.  **Queue-Based Execution**: Commands sent from the web should be queued to ensure none are lost if the device is momentarily offline.
2.  **Latency Targets**: The time from "Button Press" on a phone to "IR Signal Emission" should be under 500ms for a responsive feel.
3.  **State Tracking**: The user should see the status of a command ("Pending" -> "Sent" or "Failed").
4.  **Security**: The ESP32 must authenticate securely before reading commands to prevent unauthorized control.

## Data Model

```mermaid
classDiagram
    class QueueItem {
        string ID
        string CommandID
        string Status
        timestamp CreatedAt
        timestamp ProcessedAt
    }
    
    class Command {
        string ID
        string Protocol
        string HexCode
    }

    QueueItem --> Command : References
```

### Frontend Architecture

```mermaid
classDiagram
    class RemoteButton {
        +Icon
        +Color
        +onClick()
    }
    class FeedbackToast {
        +Message: "Sent!"
        +Type: Success | Error
    }
    class QueueMonitor {
        +PendingCount
    }

    RemoteButton ..> QueueMonitor : Updates
    RemoteButton --> FeedbackToast : Triggers
    note for RemoteButton "React Components"
```

## System Workflow

```mermaid
sequenceDiagram
    participant User
    participant Web as Web UI
    participant DB as Cloud Queue
    participant ESP as ESP32 Controller
    participant Device as TV/Appliance

    User->>Web: Tap "Power" Button
    Web->>DB: Add Command to Queue
    
    loop Polling Cycle
        ESP->>DB: Check for Pending Items
    end
    
    DB->>ESP: Return Command Details
    ESP->>Device: Transmit IR Signal
    
    alt Success
        ESP->>DB: Mark as "Sent"
        DB->>Web: Update status icon
    else Error
        ESP->>DB: Mark as "Failed"
        DB->>Web: Show error alert
    end
```

## Technical Strategy

-   **Command Queue Pattern**: We will use a First-In-First-Out (FIFO) queue structure in the database. This decouples the web app from the physical device, allowing for better error handling and retry logic.
-   **Polling vs Streaming**: To balance device stability with responsiveness, we will use a "Fast Poll" mechanism or an optimized persistent stream connection from the ESP32 to the Cloud.
-   **Protocol Support**: The transmitter will support dynamic protocol switching (swapping between NEC, Sony, etc.) based on the metadata stored during the Learning phase.

## Verification

-   **Latency Measurement**: Automated logging to measure round-trip time.
-   **Stress Test**: Rapidly firing 10 commands to ensure they are executed in the correct order without dropping any.
-   **Range Test**: Verify the IR emitter strength is sufficient to control devices from across the room.
