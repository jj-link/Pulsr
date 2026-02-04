import { FirebaseProvider } from '@/features/core/firebase'

function App() {
  return (
    <FirebaseProvider>
      <div>
        <h1>Pulsr</h1>
        <p>Firebase connection test</p>
        <p style={{ fontSize: '0.9em', color: '#888' }}>
          Check browser console for Firebase initialization message
        </p>
      </div>
    </FirebaseProvider>
  )
}

export default App
