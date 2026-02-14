import { useState, useCallback } from 'react'
import { httpsCallable } from 'firebase/functions'
import { useFunctions, useAuth } from '@/features/core/firebase/FirebaseProvider'

export interface ClaimResult {
  claimId: string
  claimCode: string
  expiresAt: string
}

export type ClaimStatus = 'idle' | 'creating' | 'waiting' | 'claimed' | 'expired' | 'failed'

export function useClaim() {
  const functions = useFunctions()
  const auth = useAuth()
  
  const [status, setStatus] = useState<ClaimStatus>('idle')
  const [claimResult, setClaimResult] = useState<ClaimResult | null>(null)
  const [error, setError] = useState<string | null>(null)

  const createClaim = useCallback(async () => {
    if (!auth.currentUser) {
      setError('Must be authenticated to create a claim')
      setStatus('failed')
      return null
    }

    setStatus('creating')
    setError(null)

    try {
      const createClaimFn = httpsCallable(functions, 'createClaim')
      const result = await createClaimFn()
      
      const data = result.data as ClaimResult
      setClaimResult(data)
      setStatus('waiting')
      return data
    } catch (err) {
      const message = err instanceof Error ? err.message : 'Failed to create claim'
      setError(message)
      setStatus('failed')
      return null
    }
  }, [functions, auth.currentUser])

  const reset = useCallback(() => {
    setStatus('idle')
    setClaimResult(null)
    setError(null)
  }, [])

  return {
    status,
    claimResult,
    error,
    createClaim,
    reset,
  }
}
