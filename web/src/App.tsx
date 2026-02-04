import { RouterProvider } from 'react-router-dom'
import { FirebaseProvider } from '@/features/core/firebase'
import { RepositoryProvider } from '@/features/core/context/RepositoryContext'
import { router } from '@/features/core/navigation/router'

function App() {
  return (
    <FirebaseProvider>
      <RepositoryProvider>
        <RouterProvider router={router} />
      </RepositoryProvider>
    </FirebaseProvider>
  )
}

export default App
