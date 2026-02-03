# Plan: Virtual Remote Designer

**Goal:** A flexible, user-friendly interface that allows users to create custom remote layouts that map to their physical devices.

## Functional Requirements

1.  **Custom Layouts**: Users should not be restricted to a generic grid; they should be able to arrange buttons to match their preference (e.g., D-Pad on the left, Volume on the right).
2.  **Device Management**: Users can create multiple "Virtual Remotes" (e.g., "Living Room TV", "Bedroom AC").
3.  **Button Customization**: Each button can be customized with a label, an icon (e.g., Power, Volume, Mute), and a color.
4.  **Editing Mode**: A dedicated "Edit Mode" prevents accidental clicks while rearranging the interface.

## Data Model

```mermaid
classDiagram
    class Device {
        string ID
        string Name
    }
    class Layout {
        string ID
        List Buttons
    }
    class Button {
        string ID
        string Icon
        string Color
        Coordinate Position
        Link CommandID
    }

    Device "1" -- "1" Layout
    Layout "1" -- "many" Button
```

## Frontend Architecture

```mermaid
classDiagram
    class RemoteGrid {
        +Buttons[]
        +isEditMode
        +onButtonMove()
    }
    class EditToolbar {
        +onAddButton()
        +onSave()
        +onCancel()
    }
    class ButtonComponent {
        +Icon
        +Color
        +Position
        +onClick()
        +onDrag()
    }
    class CommandPicker {
        +AvailableCommands[]
        +onSelect()
    }

    RemoteGrid --> ButtonComponent
    EditToolbar ..> RemoteGrid : Modifies
    EditToolbar --> CommandPicker : Opens
    note for RemoteGrid "React Components"
```

## System Workflow

```mermaid
sequenceDiagram
    participant User
    participant Editor as Layout Editor
    participant Library as Command Library
    participant DB as Cloud Database

    User->>Editor: Enter "Edit Mode"
    Editor->>Library: Fetch Available Commands
    
    User->>Library: Drag "Volume Up"
    Library->>Editor: Drop onto Grid
    Editor->>Editor: Update Local Layout State
    
    User->>Editor: Click "Save Layout"
    Editor->>DB: Write Layout JSON
    DB->>Editor: Confirm Save
    Editor->>User: "Layout Saved"
```

## Technical Strategy

-   **Responsive Grid**: The design will use a responsive grid system that adapts to mobile screens (single column) and desktop screens (multi-column/free canvas).
-   **Optimistic UI**: When a user clicks a button, the UI will immediately show a "Pressed" state before waiting for cloud confirmation, making the app feel native and snappy.
-   **Context-Aware Configuration**: The editor will only show commands available for the specific device being edited (e.g., TV commands won't show up when editing current AC remote).

## Verification

-   **Usability Testing**: Ensure users can easily add and move buttons on a phone screen.
-   **Data Integrity**: Verify that a layout saved on desktop renders correctly on mobile.
