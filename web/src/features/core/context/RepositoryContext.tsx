import { createContext, useContext, ReactNode } from 'react'
import { ICommandRepository, IDeviceRepository, IQueueRepository } from '@/features/core/types/repositories'
import { FirestoreCommandRepository } from '@/features/learning/repositories/FirestoreCommandRepository'
import { FirestoreDeviceRepository } from '@/features/learning/repositories/FirestoreDeviceRepository'
import { FirestoreQueueRepository } from '@/features/remote/repositories/FirestoreQueueRepository'
import { useFirebase } from '@/features/core/firebase'

interface RepositoryContextValue {
  commandRepository: ICommandRepository
  deviceRepository: IDeviceRepository
  queueRepository: IQueueRepository
}

const RepositoryContext = createContext<RepositoryContextValue | null>(null)

interface RepositoryProviderProps {
  children: ReactNode
}

export function RepositoryProvider({ children }: RepositoryProviderProps) {
  const { db } = useFirebase()

  const repositories: RepositoryContextValue = {
    commandRepository: new FirestoreCommandRepository(db),
    deviceRepository: new FirestoreDeviceRepository(db),
    queueRepository: new FirestoreQueueRepository(db),
  }

  return (
    <RepositoryContext.Provider value={repositories}>
      {children}
    </RepositoryContext.Provider>
  )
}

export function useRepositories(): RepositoryContextValue {
  const context = useContext(RepositoryContext)
  if (!context) {
    throw new Error('useRepositories must be used within RepositoryProvider')
  }
  return context
}
