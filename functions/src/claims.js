const { db } = require('./firebase');

const CLAIM_TTL_HOURS = 24;
const CLAIM_CODE_PREFIX = 'PULSR-';

function generateClaimCode() {
  const chars = 'ABCDEFGHJKLMNPQRSTUVWXYZ23456789';
  let code = '';
  for (let i = 0; i < 4; i++) {
    code += chars.charAt(Math.floor(Math.random() * chars.length));
  }
  return `${CLAIM_CODE_PREFIX}${code}`;
}

async function createClaim(ownerId) {
  const claimCode = generateClaimCode();
  const expiresAt = new Date();
  expiresAt.setHours(expiresAt.getHours() + CLAIM_TTL_HOURS);

  const claimDoc = {
    code: claimCode,
    ownerId: ownerId,
    status: 'pending',
    expiresAt: expiresAt,
    consumedAt: null,
    hardwareId: null,
    deviceId: null,
    createdAt: new Date()
  };

  const docRef = await db.collection('deviceClaims').add(claimDoc);

  return {
    claimId: docRef.id,
    claimCode: claimCode,
    expiresAt: expiresAt.toISOString()
  };
}

async function redeemClaim(claimCode, hardwareId, deviceName) {
  const claimsSnapshot = await db.collection('deviceClaims')
    .where('code', '==', claimCode.toUpperCase())
    .where('status', '==', 'pending')
    .limit(1)
    .get();

  if (claimsSnapshot.empty) {
    throw new Error('Invalid or already-used claim code');
  }

  const claimDoc = claimsSnapshot.docs[0];
  const claimData = claimDoc.data();

  if (claimData.expiresAt.toDate() < new Date()) {
    await claimDoc.ref.update({ status: 'expired' });
    throw new Error('Claim code has expired');
  }

  const deviceId = db.collection('devices').doc().id;
  const defaultName = deviceName || `Pulsr-${hardwareId}`;

  const deviceData = {
    ownerId: claimData.ownerId,
    name: defaultName,
    isLearning: false,
    lastSeenAt: new Date(),
    hardwareId: hardwareId,
    createdAt: new Date()
  };

  await db.collection('devices').doc(deviceId).set(deviceData);

  const consumedAt = new Date();
  await claimDoc.ref.update({
    status: 'consumed',
    consumedAt: consumedAt,
    hardwareId: hardwareId,
    deviceId: deviceId
  });

  return {
    deviceId: deviceId,
    ownerId: claimData.ownerId,
    status: 'consumed'
  };
}

module.exports = {
  createClaim,
  redeemClaim,
  generateClaimCode
};
