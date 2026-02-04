import { RouterProvider } from 'react-router-dom'
import { FirebaseProvider } from '@/features/core/firebase'
import { router } from '@/features/core/navigation/router'

function App() {
  return (
    <FirebaseProvider>
      <RouterProvider router={router} />
    </FirebaseProvider>
  )
}

export default App
