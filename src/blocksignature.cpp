// Copyright (c) 2017-2018 The KORE developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "blocksignature.h"
#include "main.h"

bool SignBlockWithKey(CBlock& block, const CKey& key)
{
    if (!key.Sign(block.GetHash(), block.vchBlockSig))
        return error("%s: failed to sign block hash with key", __func__);

    return true;
}

bool GetKeyIDFromUTXO(const CTxOut& txout, CKeyID& keyID)
{
    std::vector<valtype> vSolutions;
    txnouttype whichType;
    if (!Solver(txout.scriptPubKey, whichType, vSolutions))
        return false;
    if (whichType == TX_PUBKEY || whichType == TX_LOCKSTAKE) {
        keyID = CPubKey(vSolutions[0]).GetID();
    } else if (whichType == TX_PUBKEYHASH) {
        keyID = CKeyID(uint160(vSolutions[0]));
    }

    return true;
}

bool SignBlock(CBlock& block, const CKeyStore& keystore)
{
    CKeyID keyID;
    if (block.IsProofOfWork()) {
        bool fFoundID = false;
        for (const CTxOut& txout :block.vtx[0].vout) {
            if (!GetKeyIDFromUTXO(txout, keyID))
                continue;
            fFoundID = true;
            break;
        }
        if (!fFoundID)
            return error("%s: failed to find key for PoW", __func__);
    } else {
        if (!GetKeyIDFromUTXO(block.vtx[0].vout[1], keyID))
            return error("%s: failed to find key for PoS", __func__);
    }

    CKey key;
    if (!keystore.GetKey(keyID, key))
        return error("%s: failed to get key from keystore", __func__);

    return SignBlockWithKey(block, key);
}

bool CheckBlockSignature(const CBlock& block)
{
    if (block.IsProofOfWork())
        return block.vchBlockSig.empty();

    if (block.vchBlockSig.empty())
        return error("%s: vchBlockSig is empty!", __func__);

    /** Each block is signed by the private key of the input that is staked. This is normal UTXO
     *  The public key that signs must match the public key associated with the first utxo of the coinstake tx.
     */
    CPubKey pubkey;
    txnouttype whichType;
    std::vector<valtype> vSolutions;
    const CTxOut& txout = block.vtx[1].vout[0];
    if (!Solver(txout.scriptPubKey, whichType, vSolutions))
        return false;
    if (whichType == TX_PUBKEY || whichType == TX_PUBKEYHASH || whichType == TX_LOCKSTAKE) {
        valtype& vchPubKey = vSolutions[0];
        pubkey = CPubKey(vchPubKey);
    }

    if (!pubkey.IsValid())
        return error("%s: invalid pubkey %s", __func__, pubkey.GetHex());

    return pubkey.Verify(block.GetHash(), block.vchBlockSig);
}

bool CheckBlockSignature_Legacy(const CBlock& block, const uint256& hash)
{
    if (block.IsProofOfWork())
        return block.vchBlockSig.empty();

    if (block.vchBlockSig.empty())
        return false;

    vector<vector<unsigned char> > vSolutions;
    txnouttype whichType;

    const CTxOut& txout = block.vtx[1].vout[1];

    if (!Solver(txout.scriptPubKey, whichType, vSolutions))
        return false;

    if (whichType == TX_PUBKEY)
    {
        vector<unsigned char>& vchPubKey = vSolutions[0];
        return CPubKey(vchPubKey).Verify(hash, block.vchBlockSig);
    }
    else
    {
        // Block signing key also can be encoded in the nonspendable output
        // This allows to not pollute UTXO set with useless outputs e.g. in case of multisig staking

        const CScript& script = txout.scriptPubKey;
        CScript::const_iterator pc = script.begin();
        opcodetype opcode;
        vector<unsigned char> vchPushValue;

        if (!script.GetOp(pc, opcode, vchPushValue))
            return false;
        if (opcode != OP_RETURN)
            return false;
        if (!script.GetOp(pc, opcode, vchPushValue))
            return false;
        if (!IsCompressedOrUncompressedPubKey(vchPushValue))
            return false;
        return CPubKey(vchPushValue).Verify(hash, block.vchBlockSig);
    }

    return false;
}