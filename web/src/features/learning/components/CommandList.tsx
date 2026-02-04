import { useState } from 'react'
import { IRCommand } from '@/features/core/types'

interface CommandListProps {
  commands: IRCommand[]
  onDelete: (commandId: string) => void
  onEdit?: (commandId: string, updates: Partial<IRCommand>) => void
}

export function CommandList({ commands, onDelete, onEdit }: CommandListProps) {
  const [editingId, setEditingId] = useState<string | null>(null)
  const [editName, setEditName] = useState('')

  if (commands.length === 0) {
    return (
      <div>
        <p>No commands learned yet</p>
      </div>
    )
  }

  const handleEdit = (command: IRCommand) => {
    setEditingId(command.id)
    setEditName(command.name)
  }

  const handleSave = (commandId: string) => {
    if (onEdit && editName.trim()) {
      onEdit(commandId, { name: editName.trim() })
    }
    setEditingId(null)
    setEditName('')
  }

  const handleCancel = () => {
    setEditingId(null)
    setEditName('')
  }

  return (
    <div>
      <h3>Learned Commands ({commands.length})</h3>
      <div>
        {commands.map((command) => (
          <div key={command.id}>
            <div>
              {editingId === command.id ? (
                <input
                  type="text"
                  value={editName}
                  onChange={(e) => setEditName(e.target.value)}
                />
              ) : (
                <div>{command.name}</div>
              )}
              <div>
                {command.protocol} | Address: {command.address} | Command: {command.command}
              </div>
            </div>
            <div>
              {editingId === command.id ? (
                <>
                  <button onClick={() => handleSave(command.id)}>Save</button>
                  <button onClick={handleCancel}>Cancel</button>
                </>
              ) : (
                <>
                  {onEdit && <button onClick={() => handleEdit(command)}>Edit</button>}
                  <button onClick={() => onDelete(command.id)}>Delete</button>
                </>
              )}
            </div>
          </div>
        ))}
      </div>
    </div>
  )
}
