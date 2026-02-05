import { describe, test, expect, vi } from 'vitest'
import { render, screen } from '@testing-library/react'
import userEvent from '@testing-library/user-event'
import { CreateDeviceModal } from '../CreateDeviceModal'

describe('CreateDeviceModal', () => {
  test('does not render when closed', () => {
    render(
      <CreateDeviceModal
        isOpen={false}
        onClose={() => {}}
        onCreate={() => {}}
      />
    )

    expect(screen.queryByText(/create new device/i)).not.toBeInTheDocument()
  })

  test('renders when open', () => {
    render(
      <CreateDeviceModal
        isOpen={true}
        onClose={() => {}}
        onCreate={() => {}}
      />
    )

    expect(screen.getByText(/create new device/i)).toBeInTheDocument()
    expect(screen.getByLabelText(/device name/i)).toBeInTheDocument()
  })

  test('calls onCreate with device name when form submitted', async () => {
    const mockCreate = vi.fn()
    const user = userEvent.setup()

    render(
      <CreateDeviceModal
        isOpen={true}
        onClose={() => {}}
        onCreate={mockCreate}
      />
    )

    await user.type(screen.getByLabelText(/device name/i), 'Kitchen TV')
    await user.type(screen.getByLabelText(/device id/i), 'kitchen-tv-001')
    await user.click(screen.getByRole('button', { name: /create/i }))

    expect(mockCreate).toHaveBeenCalledWith('Kitchen TV', 'kitchen-tv-001')
  })

  test('does not call onCreate if name is empty', async () => {
    const mockCreate = vi.fn()
    const user = userEvent.setup()

    render(
      <CreateDeviceModal
        isOpen={true}
        onClose={() => {}}
        onCreate={mockCreate}
      />
    )

    await user.click(screen.getByRole('button', { name: /create/i }))

    expect(mockCreate).not.toHaveBeenCalled()
  })

  test('calls onClose when cancel button clicked', async () => {
    const mockClose = vi.fn()
    const user = userEvent.setup()

    render(
      <CreateDeviceModal
        isOpen={true}
        onClose={mockClose}
        onCreate={() => {}}
      />
    )

    await user.click(screen.getByRole('button', { name: /cancel/i }))

    expect(mockClose).toHaveBeenCalled()
  })

  test('clears input after successful creation', async () => {
    const mockCreate = vi.fn()
    const user = userEvent.setup()

    const { rerender } = render(
      <CreateDeviceModal
        isOpen={true}
        onClose={() => {}}
        onCreate={mockCreate}
      />
    )

    const nameInput = screen.getByLabelText(/device name/i) as HTMLInputElement
    const idInput = screen.getByLabelText(/device id/i) as HTMLInputElement
    await user.type(nameInput, 'Living Room TV')
    await user.type(idInput, 'living-room-tv-001')
    await user.click(screen.getByRole('button', { name: /create/i }))

    rerender(
      <CreateDeviceModal
        isOpen={true}
        onClose={() => {}}
        onCreate={mockCreate}
      />
    )

    expect(nameInput.value).toBe('')
    expect(idInput.value).toBe('')
  })
})
