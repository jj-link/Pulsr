import { DeviceLayout, LayoutButton } from '@/features/core/types'

export interface ValidationResult {
  valid: boolean
  error?: string
}

export function validateLayout(layout: DeviceLayout): ValidationResult {
  // Check for duplicate positions
  for (let i = 0; i < layout.buttons.length; i++) {
    for (let j = i + 1; j < layout.buttons.length; j++) {
      if (
        layout.buttons[i].position.row === layout.buttons[j].position.row &&
        layout.buttons[i].position.col === layout.buttons[j].position.col
      ) {
        return { valid: false, error: 'Two buttons cannot occupy the same cell' }
      }
    }
  }

  // Check all buttons within grid bounds
  for (const button of layout.buttons) {
    if (!isWithinBounds(button, layout.gridSize)) {
      return { valid: false, error: 'Button outside grid bounds' }
    }
  }

  return { valid: true }
}

export function isWithinBounds(
  button: LayoutButton,
  gridSize: { rows: number; cols: number }
): boolean {
  return (
    button.position.row >= 0 &&
    button.position.row < gridSize.rows &&
    button.position.col >= 0 &&
    button.position.col < gridSize.cols
  )
}

export function createDefaultLayout(): DeviceLayout {
  return {
    gridSize: { rows: 4, cols: 3 },
    buttons: [],
  }
}

export function resizeGrid(
  layout: DeviceLayout,
  newRows: number,
  newCols: number
): DeviceLayout {
  // Keep only buttons that fit within the new grid size
  const filteredButtons = layout.buttons.filter(
    (btn) => btn.position.row < newRows && btn.position.col < newCols
  )

  return {
    gridSize: { rows: newRows, cols: newCols },
    buttons: filteredButtons,
  }
}
