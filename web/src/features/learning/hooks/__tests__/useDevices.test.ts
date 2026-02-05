import { describe, test, expect, beforeEach } from 'vitest'
import { renderHook, waitFor } from '@testing-library/react'
import { useDevices } from '../useDevices'
import { InMemoryDeviceRepository } from '../../repositories/InMemoryDeviceRepository'

describe('useDevices', () => {
  let repository: InMemoryDeviceRepository

  beforeEach(() => {
    repository = new InMemoryDeviceRepository()
  })

  test('loads all devices', async () => {
    await repository.create({
      name: 'Living Room TV',
      isLearning: false,
      ownerId: 'user1',
    })

    const { result } = renderHook(() => useDevices(repository))

    await waitFor(() => {
      expect(result.current.loading).toBe(false)
    })

    expect(result.current.devices).toHaveLength(1)
    expect(result.current.devices[0].name).toBe('Living Room TV')
  })

  test('updates in real-time when device is created', async () => {
    const { result } = renderHook(() => useDevices(repository))

    await waitFor(() => {
      expect(result.current.loading).toBe(false)
    })

    expect(result.current.devices).toHaveLength(0)

    await repository.create({
      name: 'Bedroom TV',
      isLearning: false,
      ownerId: 'user1',
    })

    await waitFor(() => {
      expect(result.current.devices).toHaveLength(1)
    })
  })

  test('createDevice creates a new device', async () => {
    const { result } = renderHook(() => useDevices(repository))

    await waitFor(() => {
      expect(result.current.loading).toBe(false)
    })

    const device = await result.current.createDevice('Kitchen TV', 'kitchen-tv-001', 'user1')

    expect(device.name).toBe('Kitchen TV')
    expect(device.ownerId).toBe('user1')
    expect(device.isLearning).toBe(false)

    await waitFor(() => {
      expect(result.current.devices).toHaveLength(1)
    })
  })

  test('setLearningMode updates device learning state', async () => {
    const device = await repository.create({
      name: 'Living Room TV',
      isLearning: false,
      ownerId: 'user1',
    })

    const { result } = renderHook(() => useDevices(repository))

    await waitFor(() => {
      expect(result.current.devices[0].isLearning).toBe(false)
    })

    await result.current.setLearningMode(device.id, true)

    await waitFor(() => {
      expect(result.current.devices[0].isLearning).toBe(true)
    })
  })

  test('deleteDevice removes device from list', async () => {
    const device = await repository.create({
      name: 'Living Room TV',
      isLearning: false,
      ownerId: 'user1',
    })

    const { result } = renderHook(() => useDevices(repository))

    await waitFor(() => {
      expect(result.current.devices).toHaveLength(1)
    })

    await result.current.deleteDevice(device.id)

    await waitFor(() => {
      expect(result.current.devices).toHaveLength(0)
    })
  })
})
