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
      value: '16724175',
      bits: 32,
      capturedAt: new Date('2024-01-01'),
    },
    {
      id: 'cmd-2',
      deviceId: 'device-1',
      name: 'Volume Up',
      protocol: 'NEC',
      address: '0x00',
      command: '0x10',
      value: '16712445',
      bits: 32,
      capturedAt: new Date('2024-01-02'),
    },
  ]

  test('shows empty state when no commands', () => {
    render(<CommandList commands={[]} onDelete={() => {}} onEdit={() => {}} />)

    expect(screen.getByText(/no commands learned yet/i)).toBeInTheDocument()
  })

  test('renders all commands', () => {
    render(<CommandList commands={mockCommands} onDelete={() => {}} onEdit={() => {}} />)

    expect(screen.getByText('Power')).toBeInTheDocument()
    expect(screen.getByText('Volume Up')).toBeInTheDocument()
  })

  test('shows protocol information for each command', () => {
    render(<CommandList commands={mockCommands} onDelete={() => {}} onEdit={() => {}} />)

    expect(screen.getByText(/NEC.*0x00.*0x01/)).toBeInTheDocument()
    expect(screen.getByText(/NEC.*0x00.*0x10/)).toBeInTheDocument()
  })

  test('calls onDelete when delete button clicked', async () => {
    const mockDelete = vi.fn()
    const user = userEvent.setup()

    render(<CommandList commands={mockCommands} onDelete={mockDelete} onEdit={() => {}} />)

    const deleteButtons = screen.getAllByRole('button', { name: /delete/i })
    await user.click(deleteButtons[0])

    expect(mockDelete).toHaveBeenCalledWith('cmd-1')
  })

  test('shows command count in header', () => {
    render(<CommandList commands={mockCommands} onDelete={() => {}} onEdit={() => {}} />)

    expect(screen.getByText(/learned commands \(2\)/i)).toBeInTheDocument()
  })

  test('calls onEdit when command name is edited', async () => {
    const mockEdit = vi.fn()
    const user = userEvent.setup()

    render(<CommandList commands={mockCommands} onDelete={() => {}} onEdit={mockEdit} />)

    const editButtons = screen.getAllByRole('button', { name: /edit/i })
    await user.click(editButtons[0])

    const input = screen.getByDisplayValue('Power')
    await user.clear(input)
    await user.type(input, 'Power On/Off')
    
    const saveButton = screen.getByRole('button', { name: /save/i })
    await user.click(saveButton)

    expect(mockEdit).toHaveBeenCalledWith('cmd-1', { name: 'Power On/Off' })
  })

  test('cancels editing without calling onEdit', async () => {
    const mockEdit = vi.fn()
    const user = userEvent.setup()

    render(<CommandList commands={mockCommands} onDelete={() => {}} onEdit={mockEdit} />)

    const editButtons = screen.getAllByRole('button', { name: /edit/i })
    await user.click(editButtons[0])

    const cancelButton = screen.getByRole('button', { name: /cancel/i })
    await user.click(cancelButton)

    expect(mockEdit).not.toHaveBeenCalled()
    expect(screen.queryByDisplayValue('Power')).not.toBeInTheDocument()
  })
})
