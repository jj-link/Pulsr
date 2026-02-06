import { describe, it, expect } from 'vitest'
import {
  validateLayout,
  isWithinBounds,
  createDefaultLayout,
  resizeGrid,
} from '../utils/layoutValidation'
import { DeviceLayout, LayoutButton } from '@/features/core/types'

describe('layoutValidation', () => {
  describe('validateLayout', () => {
    it('returns valid for empty layout', () => {
      const layout = createDefaultLayout()
      expect(validateLayout(layout)).toEqual({ valid: true })
    })

    it('returns valid for layout with non-overlapping buttons', () => {
      const layout: DeviceLayout = {
        gridSize: { rows: 4, cols: 3 },
        buttons: [
          { id: '1', commandId: 'cmd1', label: 'Power', color: '#e74c3c', position: { row: 0, col: 0 } },
          { id: '2', commandId: 'cmd2', label: 'Vol+', color: '#3498db', position: { row: 0, col: 1 } },
          { id: '3', commandId: 'cmd3', label: 'Vol-', color: '#3498db', position: { row: 1, col: 0 } },
        ],
      }
      expect(validateLayout(layout)).toEqual({ valid: true })
    })

    it('returns invalid when two buttons occupy the same cell', () => {
      const layout: DeviceLayout = {
        gridSize: { rows: 4, cols: 3 },
        buttons: [
          { id: '1', commandId: 'cmd1', label: 'Power', color: '#e74c3c', position: { row: 0, col: 0 } },
          { id: '2', commandId: 'cmd2', label: 'Dup', color: '#3498db', position: { row: 0, col: 0 } },
        ],
      }
      const result = validateLayout(layout)
      expect(result.valid).toBe(false)
      expect(result.error).toContain('same cell')
    })

    it('returns invalid when button is outside grid bounds', () => {
      const layout: DeviceLayout = {
        gridSize: { rows: 4, cols: 3 },
        buttons: [
          { id: '1', commandId: 'cmd1', label: 'Out', color: '#e74c3c', position: { row: 5, col: 0 } },
        ],
      }
      const result = validateLayout(layout)
      expect(result.valid).toBe(false)
      expect(result.error).toContain('outside grid bounds')
    })

    it('returns invalid when button has negative position', () => {
      const layout: DeviceLayout = {
        gridSize: { rows: 4, cols: 3 },
        buttons: [
          { id: '1', commandId: 'cmd1', label: 'Neg', color: '#e74c3c', position: { row: -1, col: 0 } },
        ],
      }
      const result = validateLayout(layout)
      expect(result.valid).toBe(false)
    })
  })

  describe('isWithinBounds', () => {
    const gridSize = { rows: 4, cols: 3 }

    it('returns true for valid position', () => {
      const button: LayoutButton = { id: '1', commandId: 'c1', label: 'A', color: '#fff', position: { row: 0, col: 0 } }
      expect(isWithinBounds(button, gridSize)).toBe(true)
    })

    it('returns true for last valid position', () => {
      const button: LayoutButton = { id: '1', commandId: 'c1', label: 'A', color: '#fff', position: { row: 3, col: 2 } }
      expect(isWithinBounds(button, gridSize)).toBe(true)
    })

    it('returns false for row out of bounds', () => {
      const button: LayoutButton = { id: '1', commandId: 'c1', label: 'A', color: '#fff', position: { row: 4, col: 0 } }
      expect(isWithinBounds(button, gridSize)).toBe(false)
    })

    it('returns false for col out of bounds', () => {
      const button: LayoutButton = { id: '1', commandId: 'c1', label: 'A', color: '#fff', position: { row: 0, col: 3 } }
      expect(isWithinBounds(button, gridSize)).toBe(false)
    })
  })

  describe('createDefaultLayout', () => {
    it('creates a 4x3 grid with no buttons', () => {
      const layout = createDefaultLayout()
      expect(layout.gridSize).toEqual({ rows: 4, cols: 3 })
      expect(layout.buttons).toEqual([])
    })
  })

  describe('resizeGrid', () => {
    it('preserves buttons within new bounds', () => {
      const layout: DeviceLayout = {
        gridSize: { rows: 4, cols: 3 },
        buttons: [
          { id: '1', commandId: 'c1', label: 'A', color: '#fff', position: { row: 0, col: 0 } },
          { id: '2', commandId: 'c2', label: 'B', color: '#fff', position: { row: 1, col: 1 } },
        ],
      }
      const resized = resizeGrid(layout, 4, 3)
      expect(resized.buttons).toHaveLength(2)
    })

    it('removes buttons outside new bounds when shrinking', () => {
      const layout: DeviceLayout = {
        gridSize: { rows: 4, cols: 3 },
        buttons: [
          { id: '1', commandId: 'c1', label: 'A', color: '#fff', position: { row: 0, col: 0 } },
          { id: '2', commandId: 'c2', label: 'B', color: '#fff', position: { row: 3, col: 2 } },
        ],
      }
      const resized = resizeGrid(layout, 2, 2)
      expect(resized.buttons).toHaveLength(1)
      expect(resized.buttons[0].id).toBe('1')
    })

    it('updates gridSize', () => {
      const layout = createDefaultLayout()
      const resized = resizeGrid(layout, 6, 5)
      expect(resized.gridSize).toEqual({ rows: 6, cols: 5 })
    })
  })
})
