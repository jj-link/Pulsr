import { ICommandRepository } from '@/core/types/repositories'
import { IRCommand } from '@/core/types'
import {
  collection,
  doc,
  getDocs,
  getDoc,
  addDoc,
  updateDoc,
  deleteDoc,
  query,
  orderBy,
  onSnapshot,
  Firestore,
  Timestamp,
} from 'firebase/firestore'

export class FirestoreCommandRepository implements ICommandRepository {
  constructor(private db: Firestore) {}

  async getByDevice(deviceId: string): Promise<IRCommand[]> {
    const commandsRef = collection(this.db, `devices/${deviceId}/commands`)
    const q = query(commandsRef, orderBy('capturedAt', 'desc'))
    const snapshot = await getDocs(q)
    return snapshot.docs.map(doc => this.mapDoc(doc.id, doc.data(), deviceId))
  }

  async getById(id: string): Promise<IRCommand | null> {
    const parts = id.split('/')
    if (parts.length !== 2) return null
    const [deviceId, commandId] = parts
    const docRef = doc(this.db, `devices/${deviceId}/commands/${commandId}`)
    const snapshot = await getDoc(docRef)
    if (!snapshot.exists()) return null
    return this.mapDoc(snapshot.id, snapshot.data(), deviceId)
  }

  async create(command: Omit<IRCommand, 'id' | 'capturedAt'>): Promise<IRCommand> {
    const commandsRef = collection(this.db, `devices/${command.deviceId}/commands`)
    const docRef = await addDoc(commandsRef, {
      ...command,
      capturedAt: Timestamp.now(),
    })
    const snapshot = await getDoc(docRef)
    return this.mapDoc(snapshot.id, snapshot.data()!, command.deviceId)
  }

  async update(id: string, updates: Partial<IRCommand>): Promise<void> {
    const parts = id.split('/')
    if (parts.length !== 2) throw new Error('Invalid command ID')
    const [deviceId, commandId] = parts
    const docRef = doc(this.db, `devices/${deviceId}/commands/${commandId}`)
    await updateDoc(docRef, updates as any)
  }

  async delete(id: string): Promise<void> {
    const parts = id.split('/')
    if (parts.length !== 2) throw new Error('Invalid command ID')
    const [deviceId, commandId] = parts
    const docRef = doc(this.db, `devices/${deviceId}/commands/${commandId}`)
    await deleteDoc(docRef)
  }

  subscribe(deviceId: string, callback: (commands: IRCommand[]) => void): () => void {
    const commandsRef = collection(this.db, `devices/${deviceId}/commands`)
    const q = query(commandsRef, orderBy('capturedAt', 'desc'))
    
    return onSnapshot(q, snapshot => {
      const commands = snapshot.docs.map(doc => this.mapDoc(doc.id, doc.data(), deviceId))
      callback(commands)
    })
  }

  private mapDoc(id: string, data: any, deviceId?: string): IRCommand {
    return {
      id: deviceId ? `${deviceId}/${id}` : id,
      deviceId: data.deviceId,
      name: data.name,
      protocol: data.protocol,
      address: data.address,
      command: data.command,
      capturedAt: data.capturedAt instanceof Timestamp 
        ? data.capturedAt.toDate() 
        : new Date(data.capturedAt),
    }
  }
}
