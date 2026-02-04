import { describe, test, expect, vi } from 'vitest'
import { render, screen } from '@testing-library/react'
import userEvent from '@testing-library/user-event'
import { CommandList } from '../CommandList'
import { IRCommand } from '@/features/core/types'

describe('CommandList', () => {
  const mockCommands: IRCommand[] = [
    {
      id: 'cmd-1',
      deviceId: 'device-1',
      name: 'Power',
      protocol: 'NEC',
      address: '0x00',
      command: '0x01',
      capturedAt: new Date('2024-01-01'),
    },
    {
      id: 'cmd-2',
      deviceId: 'device-1',
      name: 'Volume Up',
      protocol: 'NEC',
      address: '0x00',
      command: '0x10',
      capturedAt: new Date('2024-01-02'),
    },
  ]

  test('shows empty state when no commands', () => {
    render(<CommandList commands={[]} onDelete={() => {}} />)

    expect(screen.getByText(/no commands learned yet/i)).toBeInTheDocument()
  })

  test('renders all commands', () => {
    render(<CommandList commands={mockCommands} onDelete={() => {}} />)

    expect(screen.getByText('Power')).toBeInTheDocument()
    expect(screen.getByText('Volume Up')).toBeInTheDocument()
  })

  test('shows protocol information for each command', () => {
    render(<CommandList commands={mockCommands} onDelete={() => {}} />)

    expect(screen.getByText(/NEC.*0x00.*0x01/)).toBeInTheDocument()
    expect(screen.getByText(/NEC.*0x00.*0x10/)).toBeInTheDocument()
  })

  test('calls onDelete when delete button clicked', async () => {
    const mockDelete = vi.fn()
    const user = userEvent.setup()

    render(<CommandList commands={mockCommands} onDelete={mockDelete} />)

    const deleteButtons = screen.getAllByRole('button')
    await user.click(deleteButtons[0])

    expect(mockDelete).toHaveBeenCalledWith('cmd-1')
  })

  test('shows command count in header', () => {
    render(<CommandList commands={mockCommands} onDelete={() => {}} />)

    expect(screen.getByText(/learned commands \(2\)/i)).toBeInTheDocument()
  })
})
