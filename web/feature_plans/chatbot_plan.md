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

## Tech Stack

### Frontend
- **React 18** with TypeScript (strict mode)
- **Firebase SDK** - Cloud Functions client
- **shadcn/ui** - Accessible UI components (Radix UI primitives)
- **Lucide React** - Icons
- **TailwindCSS** - Styling
- **Vitest** - Unit tests
- **React Testing Library** - Component tests
- **Playwright** - E2E tests

### Backend (Cloud Function)
- **Firebase Cloud Functions** - Node.js 18+
- **OpenAI SDK** - Primary AI provider (GPT-4 or GPT-3.5-turbo)
- **Anthropic SDK** - Alternative provider (Claude)
- **Firestore** - Knowledge base storage, session storage
- **Express.js** - HTTP endpoint routing

## Implementation Pattern

### Repository Pattern (Frontend)

```typescript
interface IChatRepository {
  sendMessage(sessionId: string, message: string): Promise<ChatResponse>
  getSession(sessionId: string): Promise<ChatSession | null>
  createSession(): Promise<string>
}

class CloudFunctionChatRepository implements IChatRepository {
  async sendMessage(sessionId: string, message: string) {
    const response = await fetch('/api/chat', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ sessionId, message })
    })
    return response.json()
  }
}

class MockChatRepository implements IChatRepository {
  async sendMessage(sessionId: string, message: string) {
    return {
      message: `Mock response to: ${message}`,
      sessionId
    }
  }
}
```

### Custom Hooks

```typescript
// useChat.ts
function useChat(sessionId: string) {
  const [messages, setMessages] = useState<Message[]>([])
  const [loading, setLoading] = useState(false)
  const repo = useChatRepository()
  
  const sendMessage = useCallback(async (text: string) => {
    const userMessage = { role: 'user', content: text, timestamp: new Date() }
    setMessages(prev => [...prev, userMessage])
    setLoading(true)
    
    try {
      const response = await repo.sendMessage(sessionId, text)
      const aiMessage = { 
        role: 'assistant', 
        content: response.message, 
        timestamp: new Date() 
      }
      setMessages(prev => [...prev, aiMessage])
    } catch (error) {
      console.error('Chat error:', error)
    } finally {
      setLoading(false)
    }
  }, [sessionId, repo])
  
  return { messages, sendMessage, loading }
}
```

### AI Provider Abstraction (Backend)

Cloud Function uses provider interfaces for swappable AI backends:

```typescript
// Cloud Function code
interface IAIProvider {
  complete(prompt: string, context: string[]): Promise<string>
}

class OpenAIProvider implements IAIProvider {
  async complete(prompt: string, context: string[]) {
    const response = await openai.chat.completions.create({
      model: 'gpt-4',
      messages: [
        { role: 'system', content: this.buildSystemPrompt(context) },
        { role: 'user', content: prompt }
      ]
    })
    return response.choices[0].message.content
  }
  
  private buildSystemPrompt(context: string[]): string {
    return `You are a helpful assistant for the Pulsr IR remote system.
    
    Context from knowledge base:
    ${context.join('\n\n')}`
  }
}

class AnthropicProvider implements IAIProvider {
  async complete(prompt: string, context: string[]) {
    const response = await anthropic.messages.create({
      model: 'claude-3-sonnet-20240229',
      max_tokens: 1024,
      messages: [{ role: 'user', content: this.buildPrompt(prompt, context) }]
    })
    return response.content[0].text
  }
}
```

### RAG (Retrieval-Augmented Generation)

Knowledge base retrieval for context-aware responses:

```typescript
interface IKnowledgeRetriever {
  retrieve(query: string, limit: number): Promise<string[]>
}

class FirestoreKnowledgeRetriever implements IKnowledgeRetriever {
  async retrieve(query: string, limit: number): Promise<string[]> {
    // Simple keyword-based retrieval
    // In production, use vector embeddings for semantic search
    const snapshot = await db.collection('knowledgeBase')
      .where('keywords', 'array-contains-any', this.extractKeywords(query))
      .limit(limit)
      .get()
    
    return snapshot.docs.map(doc => doc.data().content)
  }
}
```

## Firestore Schema

```
knowledgeBase/{articleId}
  - title: string
  - content: string
  - keywords: string[] (for simple retrieval)
  - category: 'hardware' | 'software' | 'troubleshooting'
  - createdAt: timestamp

chatSessions/{sessionId}
  - messages: [
      { role: 'user' | 'assistant', content: string, timestamp: timestamp }
    ]
  - createdAt: timestamp
  - lastMessageAt: timestamp
```

## Cloud Function Structure

```
functions/
├── src/
│   ├── index.ts                    # Entry point
│   ├── chat/
│   │   ├── chatHandler.ts          # HTTP endpoint
│   │   ├── providers/
│   │   │   ├── IAIProvider.ts
│   │   │   ├── OpenAIProvider.ts
│   │   │   ├── AnthropicProvider.ts
│   │   │   └── MockProvider.ts
│   │   ├── knowledge/
│   │   │   ├── IKnowledgeRetriever.ts
│   │   │   ├── FirestoreKnowledgeRetriever.ts
│   │   │   └── StaticKnowledgeRetriever.ts
│   │   └── __tests__/
│   │       ├── chatHandler.test.ts
│   │       └── providers.test.ts
│   └── types/
│       └── index.ts
├── package.json
├── tsconfig.json
└── .env.local                      # API keys (not committed)
```

## File Structure (Web)

```
features/chatbot/
├── components/
│   ├── ChatWidget.tsx
│   ├── MessageList.tsx
│   ├── InputArea.tsx
│   ├── Message.tsx
│   └── index.ts
├── hooks/
│   ├── useChat.ts
│   ├── useChatSession.ts
│   └── index.ts
├── repositories/
│   ├── IChatRepository.ts
│   ├── CloudFunctionChatRepository.ts
│   ├── MockChatRepository.ts
│   └── index.ts
├── types/
│   └── index.ts
└── __tests__/
    ├── ChatWidget.test.tsx
    ├── MessageList.test.tsx
    ├── useChat.test.ts
    └── repositories.test.ts
```

## Security

### API Keys
- Stored in Cloud Function environment variables
- Never exposed to client
- Accessed via `functions.config()` or `.env`

### Rate Limiting
```typescript
// In Cloud Function
import rateLimit from 'express-rate-limit'

const limiter = rateLimit({
  windowMs: 15 * 60 * 1000, // 15 minutes
  max: 20 // limit each IP to 20 requests per window
})

app.use('/chat', limiter)
```

### Input Sanitization
```typescript
import DOMPurify from 'isomorphic-dompurify'

function sanitizeMessage(message: string): string {
  return DOMPurify.sanitize(message, { 
    ALLOWED_TAGS: [], // No HTML allowed
    ALLOWED_ATTR: [] 
  })
}
```

### Authentication
```typescript
// Require Firebase Auth token
import { auth } from 'firebase-admin'

async function verifyUser(req: Request): Promise<string> {
  const token = req.headers.authorization?.split('Bearer ')[1]
  if (!token) throw new Error('Unauthorized')
  const decodedToken = await auth().verifyIdToken(token)
  return decodedToken.uid
}
```
