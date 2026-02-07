# Firebase Setup Instructions

This guide walks you through setting up Firebase for the Pulsr web app.

## Prerequisites

- Node.js 18+ installed
- npm or yarn package manager
- A Google account

## Step 1: Create Firebase Project

1. Go to [Firebase Console](https://console.firebase.google.com/)
2. Click "Add project"
3. Enter project name: `pulsr-project` (or your preferred name)
4. Disable Google Analytics (optional for this project)
5. Click "Create project"

## Step 2: Enable Required Services

### Enable Firestore

1. In Firebase Console, go to **Build > Firestore Database**
2. Click "Create database"
3. Choose **Start in test mode** (we'll deploy security rules later)
4. Select a Cloud Firestore location (choose closest to your users)
5. Click "Enable"

### Enable Authentication (Optional but Recommended)

1. Go to **Build > Authentication**
2. Click "Get started"
3. Enable **Email/Password** provider
4. Click "Save"

### Enable Hosting

1. Go to **Build > Hosting**
2. Click "Get started"
3. Follow the setup wizard (we'll use CLI later)

### Enable Cloud Functions (For Chatbot)

1. Go to **Build > Functions**
2. Click "Get started"
3. Upgrade to **Blaze (pay-as-you-go)** plan if prompted
   - Required for Cloud Functions
   - Free tier is generous (2M invocations/month)

## Step 3: Get Firebase Configuration

1. In Firebase Console, go to **Project Settings** (gear icon)
2. Scroll down to "Your apps"
3. Click "Add app" → Web icon (`</>`)
4. Register app:
   - App nickname: `Pulsr Web`
   - Don't check "Firebase Hosting" (we'll set up via CLI)
5. Copy the Firebase configuration object

## Step 4: Configure Local Environment

1. **Create `.env` file** in `web/` directory:

```bash
cd web
cp .env.example .env
```

2. **Edit `.env`** with your Firebase config values:

```env
VITE_FIREBASE_API_KEY=AIzaSyXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
VITE_FIREBASE_AUTH_DOMAIN=pulsr-project.firebaseapp.com
VITE_FIREBASE_PROJECT_ID=pulsr-project
VITE_FIREBASE_STORAGE_BUCKET=pulsr-project.appspot.com
VITE_FIREBASE_MESSAGING_SENDER_ID=123456789012
VITE_FIREBASE_APP_ID=1:123456789012:web:abcdef123456
```

> **Note:** These values are safe to use in client-side code. The API key is not a secret - it only identifies your Firebase project.

## Step 5: Update Firebase Project ID

Edit `.firebaserc` in the project root:

```json
{
  "projects": {
    "default": "your-actual-project-id"
  }
}
```

Replace `pulsr-project` with your actual Firebase project ID.

## Step 6: Install Firebase CLI

```bash
# Install globally
npm install -g firebase-tools

# Login to Firebase
firebase login

# Verify installation
firebase projects:list
```

You should see your `pulsr-project` in the list.

## Step 7: Install Dependencies

```bash
# In the web/ directory
cd web
npm install
```

This will install all dependencies from `package.json`, including Firebase SDK.

## Step 8: Deploy Firestore Security Rules

```bash
# From project root
firebase deploy --only firestore:rules
```

This deploys the security rules from `firestore.rules`.

## Step 9: Test Firebase Connection

Start the development server:

```bash
cd web
npm run dev
```

Open browser to `http://localhost:5173` (or the URL shown in terminal).

Check the browser console - you should see:
```
Firebase initialized: { projectId: 'pulsr-project' }
```

## Step 10: Set Up Firebase Emulators (Optional but Recommended)

For local development without hitting production Firebase:

```bash
# From project root
firebase init emulators

# Select:
# - Firestore Emulator
# - Functions Emulator
# - Hosting Emulator

# Use default ports or customize
```

Start emulators:

```bash
firebase emulators:start
```

Update Firebase config to use emulators in development:

```typescript
// web/src/features/core/firebase/config.ts
import { connectFirestoreEmulator } from 'firebase/firestore'
import { connectAuthEmulator } from 'firebase/auth'
import { connectFunctionsEmulator } from 'firebase/functions'

export function initializeFirebase() {
  if (!app) {
    app = initializeApp(firebaseConfig)
    db = getFirestore(app)
    auth = getAuth(app)
    functions = getFunctions(app)
    
    // Connect to emulators in development
    if (import.meta.env.DEV) {
      connectFirestoreEmulator(db, 'localhost', 8080)
      connectAuthEmulator(auth, 'http://localhost:9099')
      connectFunctionsEmulator(functions, 'localhost', 5001)
    }
  }
  return { app, db, auth, functions }
}
```

## Step 11: Deploy to Firebase Hosting (When Ready)

```bash
# Build the app
cd web
npm run build

# Deploy to hosting
firebase deploy --only hosting
```

Your app will be live at `https://pulsr-project.web.app`

## Firestore Data Structure

Your Firestore will have these collections:

```
devices/
  {deviceId}/
    - name: string
    - isLearning: boolean
    - layout: object
    - ownerId: string
    
    commands/
      {commandId}/
        - name: string
        - protocol: string
        - address: string
        - command: string
        - capturedAt: timestamp
    

knowledgeBase/
  {articleId}/
    - title: string
    - content: string
    - keywords: array
    - category: string

chatSessions/
  {sessionId}/
    - messages: array
    - userId: string
    - createdAt: timestamp
```

## Troubleshooting

### "Permission denied" errors

- Check Firestore security rules are deployed
- Verify user is authenticated (if rules require auth)
- Check browser console for detailed error messages

### Build fails with Firebase import errors

- Run `npm install` in `web/` directory
- Verify `firebase` package version matches `package.json`

### Emulator connection refused

- Ensure emulators are running: `firebase emulators:start`
- Check emulator ports match your config
- Verify no other services using the same ports

### Environment variables not loading

- Ensure `.env` file is in `web/` directory
- Verify all variables start with `VITE_` prefix
- Restart dev server after changing `.env`

## Next Steps

- ✅ Firebase configured
- ⏭️ Build React app scaffold (Vite + React Router)
- ⏭️ Implement repository interfaces
- ⏭️ Create mock implementations for development
- ⏭️ Build UI components

## Useful Commands

```bash
# Start development server
npm run dev

# Build for production
npm run build

# Preview production build
npm run preview

# Run tests
npm test

# Deploy everything
firebase deploy

# Deploy specific services
firebase deploy --only firestore:rules
firebase deploy --only hosting
firebase deploy --only functions

# Start emulators
firebase emulators:start

# View logs
firebase functions:log
```

## ESP32 Service Account Setup

For the ESP32 to write to Firestore, you'll need a service account:

1. Go to **Project Settings > Service Accounts**
2. Click "Generate new private key"
3. Download JSON file
4. **NEVER commit this file to git**
5. Configure ESP32 firmware with service account credentials

This will be covered in the ESP32 setup documentation.
