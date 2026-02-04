import { describe, test, expect, vi } from 'vitest'
import { render, screen } from '@testing-library/react'
import userEvent from '@testing-library/user-event'
import { DeviceSelector } from '../DeviceSelector'
import { Device } from '@/features/core/types'

describe('DeviceSelector', () => {
  const mockDevices: Device[] = [
    { id: 'device-1', name: 'Living Room TV', isLearning: false, ownerId: 'user1' },
    { id: 'device-2', name: 'Bedroom TV', isLearning: false, ownerId: 'user1' },
  ]

  test('renders device dropdown with all devices', () => {
    render(
      <DeviceSelector
        devices={mockDevices}
        selectedDeviceId={null}
        onSelectDevice={() => {}}
        onCreateDevice={() => {}}
      />
    )

    expect(screen.getByRole('combobox')).toBeInTheDocument()
    expect(screen.getByText('Living Room TV')).toBeInTheDocument()
    expect(screen.getByText('Bedroom TV')).toBeInTheDocument()
  })

  test('shows selected device', () => {
    render(
      <DeviceSelector
        devices={mockDevices}
        selectedDeviceId="device-1"
        onSelectDevice={() => {}}
        onCreateDevice={() => {}}
      />
    )

    const select = screen.getByRole('combobox') as HTMLSelectElement
    expect(select.value).toBe('device-1')
  })

  test('calls onSelectDevice when user selects a device', async () => {
    const mockSelect = vi.fn()
    const user = userEvent.setup()

    render(
      <DeviceSelector
        devices={mockDevices}
        selectedDeviceId={null}
        onSelectDevice={mockSelect}
        onCreateDevice={() => {}}
      />
    )

    await user.selectOptions(screen.getByRole('combobox'), 'device-2')

    expect(mockSelect).toHaveBeenCalledWith('device-2')
  })

  test('calls onCreateDevice when create button is clicked', async () => {
    const mockCreate = vi.fn()
    const user = userEvent.setup()

    render(
      <DeviceSelector
        devices={mockDevices}
        selectedDeviceId={null}
        onSelectDevice={() => {}}
        onCreateDevice={mockCreate}
      />
    )

    await user.click(screen.getByRole('button', { name: /new device/i }))

    expect(mockCreate).toHaveBeenCalled()
  })

  test('shows placeholder when no device selected', () => {
    render(
      <DeviceSelector
        devices={mockDevices}
        selectedDeviceId={null}
        onSelectDevice={() => {}}
        onCreateDevice={() => {}}
      />
    )

    expect(screen.getByText(/select a device/i)).toBeInTheDocument()
  })
})
