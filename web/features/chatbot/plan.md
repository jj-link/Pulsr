# Chatbot - Web UI Implementation

**Purpose:** Floating chat widget for AI-powered troubleshooting assistance.

## Components

### ChatWidget
Floating button and chat panel.

**Responsibilities:**
- Toggle chat panel open/close
- Floating position (bottom-right)
- Unread message indicator
- Minimize/expand animation

### MessageList
Display conversation history.

**Responsibilities:**
- Render user and AI messages
- Auto-scroll to latest message
- Loading indicator during AI response
- Message timestamps

### InputArea
Text input for user messages.

**Responsibilities:**
- Text input with send button
- Enter to send (Shift+Enter for newline)
- Character limit indicator
- Disabled during AI response

## Backend Integration

### Cloud Function
Firebase Cloud Function handles AI requests.

**Endpoint:** `POST /chat`

**Request:**
```json
{
  "sessionId": "string",
  "message": "string"
}
```

**Response:**
```json
{
  "message": "string",
  "sessionId": "string"
}
```

**Responsibilities:**
- Retrieve knowledge base context (RAG)
- Construct prompt with context
- Call AI provider (OpenAI/Anthropic)
- Return response
- Store conversation history

## Testing Strategy

### Unit Tests (TDD)
- Message list rendering
- Input validation
- Session management

### Integration Tests
- Mock Cloud Function
- Test message flow end-to-end

### E2E Tests (Playwright)
- User can open chat widget
- User can send message
- AI response appears
- Conversation persists across sessions

## Dependencies

- React
- Firebase SDK (Cloud Functions)
- UI library (shadcn/ui)
- TailwindCSS

## Backend Dependencies

- Firebase Cloud Functions
- OpenAI or Anthropic SDK
- Firestore (knowledge base, session storage)

## Security

- API keys stored in Cloud Function environment
- Rate limiting on Cloud Function
- Message sanitization (XSS prevention)
