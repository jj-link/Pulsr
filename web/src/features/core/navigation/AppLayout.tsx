import { Outlet, NavLink } from 'react-router-dom'
import './AppLayout.css'

export function AppLayout() {
  return (
    <div className="app-layout">
      <nav className="app-nav">
        <div className="app-nav-brand">
          <h1>Pulsr</h1>
        </div>
        <div className="app-nav-links">
          <NavLink to="/remote" className={({ isActive }) => isActive ? 'active' : ''}>
            Remote
          </NavLink>
          <NavLink to="/devices" className={({ isActive }) => isActive ? 'active' : ''}>
            Devices
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
