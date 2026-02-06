import { ILayoutRepository } from '@/features/core/types/repositories'
import { DeviceLayout } from '@/features/core/types'
import {
  doc,
  getDoc,
  updateDoc,
  onSnapshot,
  Firestore,
} from 'firebase/firestore'

export class FirestoreLayoutRepository implements ILayoutRepository {
  constructor(private db: Firestore) {}

  async getByDevice(deviceId: string): Promise<DeviceLayout | null> {
    const docRef = doc(this.db, 'devices', deviceId)
    const docSnap = await getDoc(docRef)

    if (!docSnap.exists()) {
      return null
    }

    const data = docSnap.data()
    return data.layout || null
  }

  async save(deviceId: string, layout: DeviceLayout): Promise<void> {
    const docRef = doc(this.db, 'devices', deviceId)
    await updateDoc(docRef, { layout })
  }

  subscribe(deviceId: string, callback: (layout: DeviceLayout | null) => void): () => void {
    const docRef = doc(this.db, 'devices', deviceId)

    const unsubscribe = onSnapshot(docRef, (docSnap) => {
      if (!docSnap.exists()) {
        callback(null)
        return
      }
      const data = docSnap.data()
      callback(data.layout || null)
    })

    return unsubscribe
  }
}
