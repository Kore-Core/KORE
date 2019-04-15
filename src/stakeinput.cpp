// Copyright (c) 2017-2018 The KORE developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#include "chain.h"
#include "main.h"
#include "stakeinput.h"
#include "wallet.h"

//!KORE Stake
CScript CKoreStake::GetScriptPubKey(CWallet* pwallet, CScript& scriptPubKey, bool fisStake)
{
    vector<valtype> vSolutions;
    CScript scriptPubKeyKernel = txFrom.vout[nPosition].scriptPubKey;
    if (!Solver(scriptPubKeyKernel, ptxType, vSolutions)) {
        LogPrintf("CreateCoinStake : failed to parse kernel\n");
        return false;
    }

    if (ptxType != TX_PUBKEY && ptxType != TX_PUBKEYHASH && ptxType != TX_LOCKSTAKE)
        return false; // only support pay to public key and pay to address

    if (ptxType == TX_PUBKEYHASH || (fisStake && ptxType == TX_PUBKEY) || (!fisStake && ptxType == TX_LOCKSTAKE)) // pay to address type
    {
        //convert to pay to public key type
        if (ptxType == TX_PUBKEYHASH) {
            CKey key;
            if (!pwallet->GetKey(uint160(vSolutions[0]), key))
                return false;

            if (fisStake)
                scriptPubKey << Params().GetStakeLockSequenceNumber() << OP_CHECKSEQUENCEVERIFY << OP_DROP << key.GetPubKey() << OP_CHECKSIG;
            else
                scriptPubKey << key.GetPubKey() << OP_CHECKSIG;
        } else {
            CPubKey pubKey(vSolutions[0]);
            if (!pubKey.IsValid())
                return false;

            if (fisStake)
                scriptPubKey << Params().GetStakeLockSequenceNumber() << OP_CHECKSEQUENCEVERIFY << OP_DROP << pubKey << OP_CHECKSIG;
            else
                scriptPubKey << pubKey << OP_CHECKSIG;
        }
    } else
        scriptPubKey = scriptPubKeyKernel;

    return scriptPubKey;
}

bool CKoreStake::SetInput(CTransaction txPrev, unsigned int n)
{
    this->txFrom = txPrev;
    this->nPosition = n;
    return true;
}

bool CKoreStake::GetTxFrom(CTransaction& tx)
{
    tx = txFrom;
    return true;
}

bool CKoreStake::CreateTxIn(CWallet* pwallet, CTxIn& txIn, uint256 hashTxOut)
{
    txIn = CTxIn(txFrom.GetHash(), nPosition);
    txIn.prevPubKey = txFrom.vout[nPosition].scriptPubKey;
    return true;
}

CAmount CKoreStake::GetValue()
{
    return txFrom.vout[nPosition].nValue;
}

bool CKoreStake::CreateLockingTxOuts(CWallet* pwallet, vector<CTxOut>& vout, CAmount value)
{
    CScript scriptPubKey;
    GetScriptPubKey(pwallet, scriptPubKey);
    CAmount splitValue = 0;
    CAmount thresholdValue = 0;

    // Calculate if we need to split the output
    CAmount newValue = MAXIMUM_STAKE_VALUE;
    if (value > newValue + (0.1 * COIN)) {
        splitValue = value - newValue;
        value = newValue;
    }

    if (value >= STAKE_SPLIT_TRESHOLD) {
        thresholdValue = value / 2;
        vout.emplace_back(CTxOut(thresholdValue, scriptPubKey));
    }

    vout.emplace_back(CTxOut(value - thresholdValue, scriptPubKey));

    if (splitValue) {
        scriptPubKey.clear();
        GetScriptPubKey(pwallet, scriptPubKey, false);
        vout.emplace_back(CTxOut(splitValue, scriptPubKey));
    }

    return true;
}

bool CKoreStake::CreateTxOut(CWallet* pwallet, CTxOut& txOut)
{
    CScript scriptPubKey;
    GetScriptPubKey(pwallet, scriptPubKey, false);
    txOut = CTxOut(0, scriptPubKey);

    return true;
}

bool CKoreStake::GetModifier(uint64_t& nStakeModifier)
{
    GetIndexFrom();
    if (!pindexFrom)
        return error("%s: failed to get index from", __func__);

    if (!GetKernelStakeModifier(pindexFrom->GetBlockHash(), nStakeModifier, true))
        return error("GetModifier(): failed to get kernel stake modifier \n");

    return true;
}

uint256 CKoreStake::GetOldModifier(bool isProofOfStake)
{
    // Lico, we could also, return the old modifier it the current one was not able
    // to find it.
    return isProofOfStake ? pindexFrom->nStakeModifierOld : chainActive.Tip()->nStakeModifierOld;
}

CDataStream CKoreStake::GetUniqueness()
{
    //The unique identifier for a KORE stake is the outpoint
    CDataStream ss(SER_NETWORK, 0);
    ss << txFrom.GetHash() << nPosition;
    return ss;
}

//The block that the UTXO was added to the chain
CBlockIndex* CKoreStake::GetIndexFrom()
{
    uint256 hashBlock = 0;
    CTransaction tx;
    if (GetTransaction(txFrom.GetHash(), tx, hashBlock, true)) {
        // If the index is in the chain, then set it as the "index from"
        if (mapBlockIndex.count(hashBlock)) {
            CBlockIndex* pindex = mapBlockIndex.at(hashBlock);
            if (chainActive.Contains(pindex))
                pindexFrom = pindex;
        }
    } else {
        LogPrintf("%s : failed to find tx %s\n", __func__, txFrom.GetHash().GetHex());
    }

    return pindexFrom;
}

int CKoreStake::GetPosition()
{
    return nPosition;
}