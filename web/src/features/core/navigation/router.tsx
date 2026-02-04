import { createBrowserRouter } from 'react-router-dom'
import { AppLayout } from './AppLayout'
import { DecoderPage } from '@/features/decoder/pages/DecoderPage'
import { DesignerPage } from '@/features/designer/pages/DesignerPage'
import { TransmissionPage } from '@/features/transmission/pages/TransmissionPage'

export const router = createBrowserRouter([
  {
    path: '/',
    element: <AppLayout />,
    children: [
      {
        index: true,
        element: <TransmissionPage />,
      },
      {
        path: 'remote',
        element: <TransmissionPage />,
      },
      {
        path: 'devices',
        element: <DecoderPage />,
      },
      {
        path: 'designer',
        element: <DesignerPage />,
      },
    ],
  },
])
