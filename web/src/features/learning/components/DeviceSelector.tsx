import { Device } from '@/features/core/types'

interface DeviceSelectorProps {
  devices: Device[]
  selectedDeviceId: string | null
  onSelectDevice: (deviceId: string) => void
  onCreateDevice?: () => void
}

export function DeviceSelector({
  devices,
  selectedDeviceId,
  onSelectDevice,
  onCreateDevice,
}: DeviceSelectorProps) {
  return (
    <div>
      <select
        value={selectedDeviceId || ''}
        onChange={(e) => onSelectDevice(e.target.value)}
      >
        <option value="">Select a device...</option>
        {devices.map((device) => (
          <option key={device.id} value={device.id}>
            {device.name}
          </option>
        ))}
      </select>
      {onCreateDevice && <button onClick={onCreateDevice}>+ New Device</button>}
    </div>
  )
}
