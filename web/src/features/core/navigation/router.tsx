import { createBrowserRouter } from 'react-router-dom'
import { AppLayout } from './AppLayout'
import { LearningPage } from '@/features/learning/pages/LearningPage'
import { DesignerPage } from '@/features/designer/pages/DesignerPage'
import { RemotePage } from '@/features/remote/pages/RemotePage'

export const router = createBrowserRouter([
  {
    path: '/',
    element: <AppLayout />,
    children: [
      {
        index: true,
        element: <RemotePage />,
      },
      {
        path: 'remote',
        element: <RemotePage />,
      },
      {
        path: 'remote/:deviceId',
        element: <RemotePage />,
      },
      {
        path: 'learn',
        element: <LearningPage />,
      },
      {
        path: 'designer',
        element: <DesignerPage />,
      },
    ],
  },
])
