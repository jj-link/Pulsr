import { useState, useRef, useEffect } from 'react'
import { Outlet, NavLink, useNavigate, useMatch, useLocation } from 'react-router-dom'
import { useRepositories } from '@/features/core/context/RepositoryContext'
import { useDevices } from '@/features/learning/hooks/useDevices'
import './AppLayout.css'

export function AppLayout() {
  const { deviceRepository } = useRepositories()
  const { devices } = useDevices(deviceRepository)
  const navigate = useNavigate()
  const remoteMatch = useMatch('/remote/:deviceId')
  const deviceId = remoteMatch?.params.deviceId
  const location = useLocation()
  const [dropdownOpen, setDropdownOpen] = useState(false)
  const dropdownRef = useRef<HTMLDivElement>(null)

  const isRemoteActive = location.pathname === '/' || location.pathname.startsWith('/remote')
  const selectedDevice = devices.find((d) => d.id === deviceId)

  useEffect(() => {
    function handleClickOutside(e: MouseEvent) {
      if (dropdownRef.current && !dropdownRef.current.contains(e.target as Node)) {
        setDropdownOpen(false)
      }
    }
    document.addEventListener('mousedown', handleClickOutside)
    return () => document.removeEventListener('mousedown', handleClickOutside)
  }, [])

  const handleDeviceSelect = (id: string) => {
    navigate(`/remote/${id}`)
    setDropdownOpen(false)
  }

  return (
    <div className="app-layout">
      <nav className="app-nav">
        <div className="app-nav-brand">
          <h1>Pulsr</h1>
        </div>
        <div className="app-nav-links">
          <div className="nav-dropdown" ref={dropdownRef}>
            <button
              className={`nav-dropdown-trigger ${isRemoteActive ? 'active' : ''}`}
              onClick={() => setDropdownOpen(!dropdownOpen)}
            >
              {selectedDevice ? `Remote: ${selectedDevice.name}` : 'Remote'} ▼
            </button>
            {dropdownOpen && (
              <div className="nav-dropdown-menu">
                {devices.length === 0 ? (
                  <div className="nav-dropdown-empty">No devices yet</div>
                ) : (
                  devices.map((device) => (
                    <button
                      key={device.id}
                      className={`nav-dropdown-item ${device.id === deviceId ? 'selected' : ''}`}
                      onClick={() => handleDeviceSelect(device.id)}
                    >
                      {device.name}
                    </button>
                  ))
                )}
              </div>
            )}
          </div>
          <NavLink to="/learn" className={({ isActive }) => isActive ? 'active' : ''}>
            Learn
          </NavLink>
          <NavLink to="/designer" className={({ isActive }) => isActive ? 'active' : ''}>
            Designer
          </NavLink>
        </div>
      </nav>
      <main className="app-content">
        <Outlet />
      </main>
    </div>
  )
}
