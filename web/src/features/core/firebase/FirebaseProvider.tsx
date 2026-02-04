import { createContext, useContext, useEffect, ReactNode } from 'react'
import { FirebaseApp } from 'firebase/app'
import { Firestore } from 'firebase/firestore'
import { Auth } from 'firebase/auth'
import { Functions } from 'firebase/functions'
import { initializeFirebase } from './config'

interface FirebaseContextValue {
  app: FirebaseApp
  db: Firestore
  auth: Auth
  functions: Functions
}

const FirebaseContext = createContext<FirebaseContextValue | null>(null)

interface FirebaseProviderProps {
  children: ReactNode
}

export function FirebaseProvider({ children }: FirebaseProviderProps) {
  const firebase = initializeFirebase()

  useEffect(() => {
    console.log('Firebase initialized:', {
      projectId: firebase.app.options.projectId,
    })
  }, [firebase.app.options.projectId])

  return (
    <FirebaseContext.Provider value={firebase}>
      {children}
    </FirebaseContext.Provider>
  )
}

export function useFirebase(): FirebaseContextValue {
  const context = useContext(FirebaseContext)
  if (!context) {
    throw new Error('useFirebase must be used within FirebaseProvider')
  }
  return context
}

export function useDb(): Firestore {
  const { db } = useFirebase()
  return db
}

export function useAuth(): Auth {
  const { auth } = useFirebase()
  return auth
}

export function useFunctions(): Functions {
  const { functions } = useFirebase()
  return functions
}
