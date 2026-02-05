import { IDeviceRepository } from '@/features/core/types/repositories'
import { Device } from '@/features/core/types'
import {
  collection,
  doc,
  getDocs,
  getDoc,
  setDoc,
  addDoc,
  updateDoc,
  deleteDoc,
  onSnapshot,
  Firestore,
} from 'firebase/firestore'

export class FirestoreDeviceRepository implements IDeviceRepository {
  private collectionName = 'devices'

  constructor(private db: Firestore) {}

  async getAll(): Promise<Device[]> {
    const querySnapshot = await getDocs(collection(this.db, this.collectionName))
    return querySnapshot.docs.map(doc => ({
      id: doc.id,
      ...doc.data(),
    })) as Device[]
  }

  async getById(id: string): Promise<Device | null> {
    const docRef = doc(this.db, this.collectionName, id)
    const docSnap = await getDoc(docRef)
    
    if (!docSnap.exists()) {
      return null
    }
    
    return {
      id: docSnap.id,
      ...docSnap.data(),
    } as Device
  }

  async create(device: Omit<Device, 'id'> & { id?: string }): Promise<Device> {
    let docRef
    if (device.id) {
      // Use provided custom ID
      docRef = doc(this.db, this.collectionName, device.id)
      const { id, ...data } = device
      await setDoc(docRef, data)
    } else {
      // Auto-generate ID
      docRef = await addDoc(collection(this.db, this.collectionName), device)
    }
    return {
      id: docRef.id,
      ...device,
    } as Device
  }

  async update(id: string, updates: Partial<Device>): Promise<void> {
    const docRef = doc(this.db, this.collectionName, id)
    await updateDoc(docRef, updates)
  }

  async delete(id: string): Promise<void> {
    const docRef = doc(this.db, this.collectionName, id)
    await deleteDoc(docRef)
  }

  async setLearningMode(deviceId: string, isLearning: boolean): Promise<void> {
    await this.update(deviceId, { isLearning })
  }

  subscribe(callback: (devices: Device[]) => void): () => void {
    const unsubscribe = onSnapshot(
      collection(this.db, this.collectionName),
      (snapshot) => {
        const devices = snapshot.docs.map(doc => ({
          id: doc.id,
          ...doc.data(),
        })) as Device[]
        callback(devices)
      }
    )
    
    return unsubscribe
  }
}
