# Pulsr Web App - Project Structure

## Directory Layout

```
web/
├── src/
│   ├── features/
│   │   ├── learning/
│   │   ├── remote/
│   │   ├── designer/
│   │   ├── chatbot/
│   │   └── core/
│   ├── App.tsx
│   ├── main.tsx
│   └── router.tsx
├── public/
├── tests/
│   ├── e2e/
│   └── setup.ts
├── package.json
├── tsconfig.json
├── vite.config.ts
├── tailwind.config.js
├── components.json
└── playwright.config.ts
```

## Feature Structure

Each feature follows the same pattern:

```
features/[feature-name]/
├── components/           # React components
│   ├── ComponentName.tsx
│   └── index.ts
├── hooks/               # Custom React hooks
│   ├── useFeature.ts
│   └── index.ts
├── repositories/        # Data layer (Firestore abstraction)
│   ├── IRepository.ts
│   ├── FirestoreRepository.ts
│   ├── InMemoryRepository.ts
│   └── index.ts
├── types/               # TypeScript types/interfaces
│   └── index.ts
└── __tests__/           # Tests for this feature
    ├── components/
    ├── hooks/
    └── repositories/
```

## Core Module

The `core/` module contains shared infrastructure:

```
features/core/
├── firebase/
│   ├── config.ts        # Firebase initialization
│   ├── auth.ts          # Auth helpers
│   └── index.ts
├── ui/                  # Base UI components (shadcn/ui)
│   ├── button.tsx
│   ├── dialog.tsx
│   ├── input.tsx
│   ├── card.tsx
│   ├── badge.tsx
│   └── ...
├── navigation/
│   ├── AppShell.tsx     # Main layout wrapper
│   ├── Sidebar.tsx      # Desktop navigation
│   ├── BottomNav.tsx    # Mobile navigation
│   └── index.ts
└── types/
    └── index.ts         # Shared TypeScript types
```

## Detailed Feature Breakdown

### Learning Feature

```
features/learning/
├── components/
│   ├── LearningModal.tsx          # Modal for learning mode workflow
│   ├── CommandList.tsx            # Display learned commands
│   ├── DeviceSelector.tsx         # Choose device for learning
│   └── index.ts
├── hooks/
│   ├── useCommands.ts             # Fetch/listen to commands
│   ├── useLearningMode.ts         # Manage learning state
│   ├── useDevices.ts              # Fetch devices
│   └── index.ts
├── repositories/
│   ├── ICommandRepository.ts      # Interface
│   ├── IDeviceRepository.ts       # Interface
│   ├── FirestoreCommandRepository.ts
│   ├── FirestoreDeviceRepository.ts
│   ├── InMemoryCommandRepository.ts  # For testing
│   ├── InMemoryDeviceRepository.ts   # For testing
│   └── index.ts
├── types/
│   └── index.ts                   # Command, Device types
└── __tests__/
    ├── LearningModal.test.tsx
    ├── CommandList.test.tsx
    ├── useCommands.test.ts
    └── useLearningMode.test.ts
```

### Remote Feature

```
features/remote/
├── components/
│   ├── RemoteButton.tsx           # Individual IR button
│   ├── RemoteGrid.tsx             # Button layout container
│   ├── QueueMonitor.tsx           # Queue status display
│   └── index.ts
├── hooks/
│   ├── useQueue.ts                # Queue operations
│   ├── useTransmission.ts         # Enqueue commands
│   └── index.ts
├── repositories/
│   ├── IQueueRepository.ts        # Interface
│   ├── FirestoreQueueRepository.ts
│   ├── InMemoryQueueRepository.ts
│   └── index.ts
├── types/
│   └── index.ts                   # QueueItem, TransmissionStatus types
└── __tests__/
    ├── RemoteButton.test.tsx
    ├── useQueue.test.ts
    └── useTransmission.test.ts
```

### Designer Feature

```
features/designer/
├── components/
│   ├── LayoutEditor.tsx           # Drag-and-drop editor
│   ├── ButtonConfigModal.tsx      # Button property editor
│   ├── CommandPicker.tsx          # Select command to assign
│   ├── IconPicker.tsx             # Lucide icon selector
│   ├── ColorPicker.tsx            # Color selector
│   └── index.ts
├── hooks/
│   ├── useLayout.ts               # Load/save layout
│   ├── useDragAndDrop.ts          # DnD state management
│   └── index.ts
├── repositories/
│   ├── ILayoutRepository.ts       # Interface
│   ├── FirestoreLayoutRepository.ts
│   ├── InMemoryLayoutRepository.ts
│   └── index.ts
├── types/
│   └── index.ts                   # Layout, ButtonConfig types
├── utils/
│   ├── layoutValidation.ts        # Layout validation logic
│   └── index.ts
└── __tests__/
    ├── LayoutEditor.test.tsx
    ├── layoutValidation.test.ts
    └── useLayout.test.ts
```

### Chatbot Feature

```
features/chatbot/
├── components/
│   ├── ChatWidget.tsx             # Floating button + panel
│   ├── MessageList.tsx            # Conversation display
│   ├── InputArea.tsx              # Message input
│   └── index.ts
├── hooks/
│   ├── useChat.ts                 # Chat state management
│   ├── useChatAPI.ts              # Cloud Function calls
│   └── index.ts
├── repositories/
│   ├── IChatRepository.ts         # Interface (optional)
│   └── index.ts
├── types/
│   └── index.ts                   # Message, Session types
└── __tests__/
    ├── ChatWidget.test.tsx
    ├── MessageList.test.tsx
    └── useChat.test.ts
```

## Entry Points

### main.tsx
```typescript
import React from 'react'
import ReactDOM from 'react-dom/client'
import { RouterProvider } from 'react-router-dom'
import { router } from './router'
import './index.css'

ReactDOM.createRoot(document.getElementById('root')!).render(
  <React.StrictMode>
    <RouterProvider router={router} />
  </React.StrictMode>,
)
```

### router.tsx
```typescript
import { createBrowserRouter } from 'react-router-dom'
import { AppShell } from './features/core/navigation/AppShell'
import { RemoteView } from './features/remote/components/RemoteView'
import { DeviceList } from './features/designer/components/DeviceList'
import { LayoutEditor } from './features/designer/components/LayoutEditor'
import { LearnModal } from './features/learning/components/LearnModal'

export const router = createBrowserRouter([
  {
    path: '/',
    element: <AppShell />,
    children: [
      { index: true, element: <RemoteView /> },
      { path: 'devices', element: <DeviceList /> },
      { path: 'designer/:id', element: <LayoutEditor /> },
      { path: 'designer/:id/learn', element: <LearnModal /> },
    ],
  },
])
```

### App.tsx
```typescript
import { Outlet } from 'react-router-dom'
import { Sidebar } from './features/core/navigation/Sidebar'
import { BottomNav } from './features/core/navigation/BottomNav'
import { ChatWidget } from './features/chatbot/components/ChatWidget'
import { FirebaseProvider } from './features/core/firebase/FirebaseProvider'

function App() {
  return (
    <FirebaseProvider>
      <div className="app-shell">
        {/* Desktop sidebar */}
        <aside className="hidden md:block">
          <Sidebar />
        </aside>
        
        {/* Main content */}
        <main className="flex-1">
          <Outlet />
        </main>
        
        {/* Mobile bottom nav */}
        <nav className="block md:hidden">
          <BottomNav />
        </nav>
        
        {/* Floating chat widget (all screens) */}
        <ChatWidget />
      </div>
    </FirebaseProvider>
  )
}

export default App
```

## Testing Structure

```
tests/
├── e2e/                          # Playwright E2E tests
│   ├── learning.spec.ts           # Learning workflow
│   ├── remote.spec.ts             # Button press workflow
│   ├── designer.spec.ts          # Layout editing
│   └── chatbot.spec.ts           # Chat interaction
├── fixtures/                     # Shared test data
│   ├── mockCommands.ts
│   ├── mockDevices.ts
│   └── mockLayouts.ts
└── setup.ts                      # Test environment setup
```

## Configuration Files

### tsconfig.json
```json
{
  "compilerOptions": {
    "target": "ES2020",
    "useDefineForClassFields": true,
    "lib": ["ES2020", "DOM", "DOM.Iterable"],
    "module": "ESNext",
    "skipLibCheck": true,
    "moduleResolution": "bundler",
    "allowImportingTsExtensions": true,
    "resolveJsonModule": true,
    "isolatedModules": true,
    "noEmit": true,
    "jsx": "react-jsx",
    "strict": true,
    "noUnusedLocals": true,
    "noUnusedParameters": true,
    "noFallthroughCasesInSwitch": true,
    "paths": {
      "@/*": ["./src/*"],
      "@/features/*": ["./src/features/*"],
      "@/core/*": ["./src/features/core/*"]
    }
  },
  "include": ["src"],
  "references": [{ "path": "./tsconfig.node.json" }]
}
```

### vite.config.ts
```typescript
import { defineConfig } from 'vite'
import react from '@vitejs/plugin-react'
import path from 'path'

export default defineConfig({
  plugins: [react()],
  resolve: {
    alias: {
      '@': path.resolve(__dirname, './src'),
      '@/features': path.resolve(__dirname, './src/features'),
      '@/core': path.resolve(__dirname, './src/features/core'),
    },
  },
  test: {
    globals: true,
    environment: 'jsdom',
    setupFiles: './tests/setup.ts',
  },
})
```

### tailwind.config.js
```javascript
/** @type {import('tailwindcss').Config} */
module.exports = {
  darkMode: ["class"],
  content: [
    './pages/**/*.{ts,tsx}',
    './components/**/*.{ts,tsx}',
    './app/**/*.{ts,tsx}',
    './src/**/*.{ts,tsx}',
  ],
  theme: {
    extend: {
      colors: {
        // shadcn/ui color system
        border: "hsl(var(--border))",
        input: "hsl(var(--input))",
        ring: "hsl(var(--ring))",
        background: "hsl(var(--background))",
        foreground: "hsl(var(--foreground))",
        primary: {
          DEFAULT: "hsl(var(--primary))",
          foreground: "hsl(var(--primary-foreground))",
        },
        // ... more shadcn colors
      },
    },
  },
  plugins: [require("tailwindcss-animate")],
}
```

### components.json (shadcn/ui config)
```json
{
  "$schema": "https://ui.shadcn.com/schema.json",
  "style": "default",
  "rsc": false,
  "tsx": true,
  "tailwind": {
    "config": "tailwind.config.js",
    "css": "src/index.css",
    "baseColor": "slate",
    "cssVariables": true
  },
  "aliases": {
    "components": "@/features/core/ui",
    "utils": "@/features/core/utils"
  }
}
```

## Import Patterns

### Absolute Imports
```typescript
// Use path aliases for cleaner imports
import { Button } from '@/core/ui/button'
import { useCommands } from '@/features/learning/hooks'
import { LearningModal } from '@/features/learning/components'
```

### Barrel Exports
```typescript
// features/learning/components/index.ts
export { LearningModal } from './LearningModal'
export { CommandList } from './CommandList'
export { DeviceSelector } from './DeviceSelector'

// Import from barrel
import { LearningModal, CommandList } from '@/features/learning/components'
```

## File Naming Conventions

- **Components:** PascalCase (`LearningModal.tsx`)
- **Hooks:** camelCase with `use` prefix (`useCommands.ts`)
- **Utilities:** camelCase (`layoutValidation.ts`)
- **Types:** camelCase files, PascalCase exports (`types/index.ts` exports `Command`)
- **Tests:** Same name as file + `.test.tsx/ts`
- **E2E tests:** Feature name + `.spec.ts`

## Code Organization Best Practices

1. **Keep features isolated** - Each feature should work independently
2. **Use barrel exports** - Export public API via `index.ts` files
3. **Co-locate tests** - Tests live with the code they test
4. **Shared code in core/** - Only truly reusable code goes in core
5. **Repository per domain** - Commands, Devices, Queue, Layout repositories
6. **One component per file** - Easier to navigate and maintain
7. **Types at feature level** - Each feature defines its own types
8. **Mock implementations** - Always provide in-memory versions for testing
