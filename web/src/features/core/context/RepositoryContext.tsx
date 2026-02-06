import { createContext, useContext, ReactNode } from 'react'
import { ICommandRepository, IDeviceRepository, IQueueRepository, ILayoutRepository } from '@/features/core/types/repositories'
import { FirestoreCommandRepository } from '@/features/learning/repositories/FirestoreCommandRepository'
import { FirestoreDeviceRepository } from '@/features/learning/repositories/FirestoreDeviceRepository'
import { FirestoreQueueRepository } from '@/features/remote/repositories/FirestoreQueueRepository'
import { FirestoreLayoutRepository } from '@/features/designer/repositories/FirestoreLayoutRepository'
import { useFirebase } from '@/features/core/firebase'

interface RepositoryContextValue {
  commandRepository: ICommandRepository
  deviceRepository: IDeviceRepository
  queueRepository: IQueueRepository
  layoutRepository: ILayoutRepository
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
    layoutRepository: new FirestoreLayoutRepository(db),
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
