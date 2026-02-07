import { initializeApp, FirebaseApp } from 'firebase/app'
import { getFirestore, Firestore } from 'firebase/firestore'
import { getDatabase, Database } from 'firebase/database'
import { getAuth, Auth } from 'firebase/auth'
import { getFunctions, Functions } from 'firebase/functions'

const firebaseConfig = {
  apiKey: import.meta.env.VITE_FIREBASE_API_KEY,
  authDomain: import.meta.env.VITE_FIREBASE_AUTH_DOMAIN,
  databaseURL: import.meta.env.VITE_FIREBASE_DATABASE_URL,
  projectId: import.meta.env.VITE_FIREBASE_PROJECT_ID,
  storageBucket: import.meta.env.VITE_FIREBASE_STORAGE_BUCKET,
  messagingSenderId: import.meta.env.VITE_FIREBASE_MESSAGING_SENDER_ID,
  appId: import.meta.env.VITE_FIREBASE_APP_ID,
}

let app: FirebaseApp
let db: Firestore
let rtdb: Database
let auth: Auth
let functions: Functions

export function initializeFirebase() {
  if (!app) {
    app = initializeApp(firebaseConfig)
    db = getFirestore(app)
    rtdb = getDatabase(app)
    auth = getAuth(app)
    functions = getFunctions(app)
  }
  return { app, db, rtdb, auth, functions }
}

export function getFirebaseApp(): FirebaseApp {
  if (!app) {
    throw new Error('Firebase not initialized. Call initializeFirebase() first.')
  }
  return app
}

export function getDb(): Firestore {
  if (!db) {
    throw new Error('Firebase not initialized. Call initializeFirebase() first.')
  }
  return db
}

export function getFirebaseAuth(): Auth {
  if (!auth) {
    throw new Error('Firebase not initialized. Call initializeFirebase() first.')
  }
  return auth
}

export function getFirebaseFunctions(): Functions {
  if (!functions) {
    throw new Error('Firebase not initialized. Call initializeFirebase() first.')
  }
  return functions
}

export function getRealtimeDb(): Database {
  if (!rtdb) {
    throw new Error('Firebase not initialized. Call initializeFirebase() first.')
  }
  return rtdb
}
