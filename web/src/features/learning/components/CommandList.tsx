import { IRCommand } from '@/features/core/types'

interface CommandListProps {
  commands: IRCommand[]
  onDelete: (commandId: string) => void
}

export function CommandList({ commands, onDelete }: CommandListProps) {
  if (commands.length === 0) {
    return (
      <div>
        <p>No commands learned yet</p>
      </div>
    )
  }

  return (
    <div>
      <h3>Learned Commands ({commands.length})</h3>
      <div>
        {commands.map((command) => (
          <div key={command.id}>
            <div>
              <div>{command.name}</div>
              <div>
                {command.protocol} | Address: {command.address} | Command: {command.command}
              </div>
            </div>
            <button onClick={() => onDelete(command.id)}>Delete</button>
          </div>
        ))}
      </div>
    </div>
  )
}
