# Chatbot — v1 Progress

**Last Updated:** 2026-02-09  
**Phase:** Not Started

## Checklist

### Backend
- [ ] Initialize Firebase Cloud Functions project (`functions/`)
- [ ] Create chat function — receives message + history, returns AI response
- [ ] Write system prompt for Pulsr troubleshooting context
- [ ] Deploy and verify function works via curl/Postman

### Frontend
- [ ] ChatWidget component — floating button + expandable panel
- [ ] useChat hook — messages state, loading state, send function
- [ ] IChatRepository + CloudFunctionChatRepository
- [ ] MockChatRepository for dev/testing
- [ ] Wire ChatWidget into the app layout

### Testing
- [ ] useChat hook unit tests (with MockChatRepository)
- [ ] ChatWidget component tests
- [ ] Playwright E2E: open → send → receive response

## Next Steps

1. Set up Cloud Functions project
2. Get a basic round-trip working (hardcoded response, no AI)
3. Add AI API call
4. Build the chat UI
