import { IQueueRepository } from '@/core/types/repositories'
import { QueueItem } from '@/core/types'
import {
  collection,
  addDoc,
  getDocs,
  updateDoc,
  doc,
  query,
  orderBy,
  onSnapshot,
  Firestore,
  Timestamp,
  serverTimestamp,
} from 'firebase/firestore'

export class FirestoreQueueRepository implements IQueueRepository {
  constructor(private db: Firestore) {}

  async getByDevice(deviceId: string): Promise<QueueItem[]> {
    const queueRef = collection(this.db, `devices/${deviceId}/queue`)
    const q = query(queueRef, orderBy('createdAt', 'desc'))
    const snapshot = await getDocs(q)
    return snapshot.docs.map(d => this.mapDoc(d.id, d.data(), deviceId))
  }

  async enqueue(deviceId: string, commandId: string): Promise<QueueItem> {
    const queueRef = collection(this.db, `devices/${deviceId}/queue`)
    const docRef = await addDoc(queueRef, {
      deviceId,
      commandId,
      status: 'pending',
      createdAt: serverTimestamp(),
    })
    return {
      id: `${deviceId}/${docRef.id}`,
      deviceId,
      commandId,
      status: 'pending',
      createdAt: new Date(),
    }
  }

  async updateStatus(id: string, status: QueueItem['status'], error?: string): Promise<void> {
    const [deviceId, queueItemId] = id.split('/')
    const docRef = doc(this.db, `devices/${deviceId}/queue/${queueItemId}`)
    const updates: Record<string, any> = {
      status,
      processedAt: serverTimestamp(),
    }
    if (error) updates.error = error
    await updateDoc(docRef, updates)
  }

  subscribe(deviceId: string, callback: (queue: QueueItem[]) => void): () => void {
    const queueRef = collection(this.db, `devices/${deviceId}/queue`)
    const q = query(queueRef, orderBy('createdAt', 'desc'))

    return onSnapshot(q, snapshot => {
      const items = snapshot.docs.map(d => this.mapDoc(d.id, d.data(), deviceId))
      callback(items)
    })
  }

  private mapDoc(id: string, data: any, deviceId: string): QueueItem {
    return {
      id: `${deviceId}/${id}`,
      deviceId: data.deviceId,
      commandId: data.commandId,
      status: data.status,
      createdAt: data.createdAt instanceof Timestamp
        ? data.createdAt.toDate()
        : new Date(data.createdAt),
      processedAt: data.processedAt instanceof Timestamp
        ? data.processedAt.toDate()
        : data.processedAt ? new Date(data.processedAt) : undefined,
      error: data.error,
    }
  }
}
