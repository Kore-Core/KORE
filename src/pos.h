// Copyright (c) 2015-2016 The Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef POS_H
#define POS_H

#include "main.h"
#include "stakeinput.h"

class CBlockIndex;
class CCoins;
class COutPoint;
class uint256;
class CTransaction;

bool CheckStakeKernelHash_Legacy(const CBlockIndex* pindexPrev, unsigned int nBits, const CCoins* txPrev, const COutPoint& prevout, unsigned int nTimeTx);

bool CheckKernel_Legacy(CBlockIndex* pindexPrev, unsigned int nBits, int64_t nTime, const COutPoint& prevout, int64_t* pBlockTime);

uint256 ComputeStakeModifier_Legacy(const CBlockIndex* pindexPrev, const uint256& kernel);
bool CheckProofOfStake_Legacy(CBlockIndex* pindexPrev, const CTransaction& tx, unsigned int nBits, CValidationState& state);

bool VerifySignature(const CTransaction& txFrom, const CTransaction& txTo, unsigned int nIn, unsigned int flags, int nHashType);
#endif // POS_H
