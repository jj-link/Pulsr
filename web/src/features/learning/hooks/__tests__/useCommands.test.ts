import { describe, test, expect, beforeEach } from 'vitest'
import { renderHook, waitFor } from '@testing-library/react'
import { useCommands } from '../useCommands'
import { InMemoryCommandRepository } from '../../repositories/InMemoryCommandRepository'

describe('useCommands', () => {
  let repository: InMemoryCommandRepository

  beforeEach(() => {
    repository = new InMemoryCommandRepository()
  })

  test('returns empty array when deviceId is null', () => {
    const { result } = renderHook(() => useCommands(repository, null))
    
    expect(result.current.commands).toEqual([])
    expect(result.current.loading).toBe(false)
  })

  test('loads commands for a device', async () => {
    const deviceId = 'device-1'
    
    await repository.create({
      deviceId,
      name: 'Power',
      protocol: 'NEC',
      address: '0x00',
      command: '0x01',
      value: '16724175',
      bits: 32,
    })

    const { result } = renderHook(() => useCommands(repository, deviceId))

    await waitFor(() => {
      expect(result.current.loading).toBe(false)
    })

    expect(result.current.commands).toHaveLength(1)
    expect(result.current.commands[0].name).toBe('Power')
  })

  test('updates commands in real-time when repository changes', async () => {
    const deviceId = 'device-1'
    
    const { result } = renderHook(() => useCommands(repository, deviceId))

    await waitFor(() => {
      expect(result.current.loading).toBe(false)
    })

    expect(result.current.commands).toHaveLength(0)

    await repository.create({
      deviceId,
      name: 'Volume Up',
      protocol: 'NEC',
      address: '0x00',
      command: '0x10',
      value: '16712445',
      bits: 32,
    })

    await waitFor(() => {
      expect(result.current.commands).toHaveLength(1)
    })
  })

  test('deleteCommand removes command from list', async () => {
    const deviceId = 'device-1'
    
    const command = await repository.create({
      deviceId,
      name: 'Power',
      protocol: 'NEC',
      address: '0x00',
      command: '0x01',
      value: '16724175',
      bits: 32,
    })

    const { result } = renderHook(() => useCommands(repository, deviceId))

    await waitFor(() => {
      expect(result.current.commands).toHaveLength(1)
    })

    await result.current.deleteCommand(command.id)

    await waitFor(() => {
      expect(result.current.commands).toHaveLength(0)
    })
  })
})
