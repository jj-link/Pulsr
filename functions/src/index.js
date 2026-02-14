const { onCall } = require('firebase-functions/v2/https');
const { createClaim, redeemClaim } = require('./claims');

exports.createClaim = onCall(
  {
    region: 'us-central1'
  },
  async (request) => {
    if (!request.auth) {
      throw new Error('Unauthorized: User must be authenticated');
    }

    const ownerId = request.auth.uid;

    const result = await createClaim(ownerId);

    return result;
  }
);

exports.redeemClaim = onCall(
  {
    region: 'us-central1'
  },
  async (request) => {
    const { claimCode, hardwareId, deviceName } = request.data;

    if (!claimCode || !hardwareId) {
      throw new Error('Missing required fields: claimCode and hardwareId');
    }

    const result = await redeemClaim(claimCode, hardwareId, deviceName);

    return result;
  }
);
