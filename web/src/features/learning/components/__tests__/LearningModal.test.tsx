import { describe, test, expect, vi } from 'vitest'
import { render, screen } from '@testing-library/react'
import userEvent from '@testing-library/user-event'
import { LearningModal } from '../LearningModal'

describe('LearningModal', () => {
  test('does not render when closed', () => {
    render(
      <LearningModal
        isOpen={false}
        onClose={() => {}}
        onSave={() => {}}
        deviceName="Living Room TV"
      />
    )

    expect(screen.queryByText(/learning mode/i)).not.toBeInTheDocument()
  })

  test('renders when open', () => {
    render(
      <LearningModal
        isOpen={true}
        onClose={() => {}}
        onSave={() => {}}
        deviceName="Living Room TV"
      />
    )

    expect(screen.getByText(/learning mode/i)).toBeInTheDocument()
  })

  test('shows device name in instructions', () => {
    render(
      <LearningModal
        isOpen={true}
        onClose={() => {}}
        onSave={() => {}}
        deviceName="Living Room TV"
      />
    )

    expect(screen.getByText(/living room tv/i)).toBeInTheDocument()
  })

  test('calls onClose when close button clicked', async () => {
    const mockClose = vi.fn()
    const user = userEvent.setup()

    render(
      <LearningModal
        isOpen={true}
        onClose={mockClose}
        onSave={() => {}}
        deviceName="Living Room TV"
      />
    )

    await user.click(screen.getByRole('button', { name: /×/i }))

    expect(mockClose).toHaveBeenCalled()
  })

  test('calls onClose when cancel button clicked', async () => {
    const mockClose = vi.fn()
    const user = userEvent.setup()

    render(
      <LearningModal
        isOpen={true}
        onClose={mockClose}
        onSave={() => {}}
        deviceName="Living Room TV"
      />
    )

    await user.click(screen.getByRole('button', { name: /cancel/i }))

    expect(mockClose).toHaveBeenCalled()
  })

  test('shows waiting state by default', () => {
    render(
      <LearningModal
        isOpen={true}
        onClose={() => {}}
        onSave={() => {}}
        deviceName="Living Room TV"
      />
    )

    expect(screen.getByText(/waiting for ir signal/i)).toBeInTheDocument()
  })
})
