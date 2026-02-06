import { useState, useEffect, useCallback } from 'react'
import { DeviceLayout, LayoutButton } from '@/features/core/types'
import { useRepositories } from '@/features/core/context/RepositoryContext'
import { createDefaultLayout, validateLayout, resizeGrid } from '../utils/layoutValidation'

export function useLayout(deviceId: string | undefined) {
  const { layoutRepository } = useRepositories()
  const [layout, setLayout] = useState<DeviceLayout | null>(null)
  const [isDirty, setIsDirty] = useState(false)
  const [loading, setLoading] = useState(true)
  const [error, setError] = useState<string | null>(null)

  useEffect(() => {
    if (!deviceId) {
      setLayout(null)
      setLoading(false)
      return
    }

    setLoading(true)
    const unsubscribe = layoutRepository.subscribe(deviceId, (savedLayout) => {
      if (!isDirty) {
        setLayout(savedLayout || createDefaultLayout())
      }
      setLoading(false)
    })

    return unsubscribe
  }, [deviceId, layoutRepository])

  const addButton = useCallback((button: LayoutButton) => {
    setLayout((prev) => {
      if (!prev) return prev
      const updated = { ...prev, buttons: [...prev.buttons, button] }
      const validation = validateLayout(updated)
      if (!validation.valid) {
        setError(validation.error || 'Invalid layout')
        return prev
      }
      setError(null)
      setIsDirty(true)
      return updated
    })
  }, [])

  const updateButton = useCallback((buttonId: string, updates: Partial<LayoutButton>) => {
    setLayout((prev) => {
      if (!prev) return prev
      const updated = {
        ...prev,
        buttons: prev.buttons.map((btn) =>
          btn.id === buttonId ? { ...btn, ...updates } : btn
        ),
      }
      const validation = validateLayout(updated)
      if (!validation.valid) {
        setError(validation.error || 'Invalid layout')
        return prev
      }
      setError(null)
      setIsDirty(true)
      return updated
    })
  }, [])

  const removeButton = useCallback((buttonId: string) => {
    setLayout((prev) => {
      if (!prev) return prev
      setIsDirty(true)
      setError(null)
      return {
        ...prev,
        buttons: prev.buttons.filter((btn) => btn.id !== buttonId),
      }
    })
  }, [])

  const changeGridSize = useCallback((rows: number, cols: number) => {
    setLayout((prev) => {
      if (!prev) return prev
      setIsDirty(true)
      setError(null)
      return resizeGrid(prev, rows, cols)
    })
  }, [])

  const save = useCallback(async () => {
    if (!deviceId || !layout) return
    const validation = validateLayout(layout)
    if (!validation.valid) {
      setError(validation.error || 'Invalid layout')
      return
    }
    try {
      await layoutRepository.save(deviceId, layout)
      setIsDirty(false)
      setError(null)
    } catch (e) {
      setError(e instanceof Error ? e.message : 'Failed to save layout')
    }
  }, [deviceId, layout, layoutRepository])

  const reset = useCallback(() => {
    if (!deviceId) return
    setIsDirty(false)
    setError(null)
    layoutRepository.getByDevice(deviceId).then((savedLayout) => {
      setLayout(savedLayout || createDefaultLayout())
    })
  }, [deviceId, layoutRepository])

  return {
    layout,
    loading,
    isDirty,
    error,
    addButton,
    updateButton,
    removeButton,
    changeGridSize,
    save,
    reset,
  }
}
