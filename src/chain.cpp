// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Copyright (c) 2016-2017 The KORE developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chain.h"

#include "chainparams.h"

using namespace std;

/**
 * CChain implementation
 */
void CChain::SetTip(CBlockIndex* pindex)
{
    if (pindex == NULL) {
        vChain.clear();
        return;
    }
    vChain.resize(pindex->nHeight + 1);
    while (pindex && vChain[pindex->nHeight] != pindex) {
        vChain[pindex->nHeight] = pindex;
        pindex = pindex->pprev;
    }
}

CBlockLocator CChain::GetLocator(const CBlockIndex* pindex) const
{
    int nStep = 1;
    std::vector<uint256> vHave;
    vHave.reserve(32);

    if (!pindex)
        pindex = Tip();
    while (pindex) {
        vHave.push_back(pindex->GetBlockHash());
        // Stop when we have added the genesis block.
        if (pindex->nHeight == 0)
            break;
        // Exponentially larger steps back, plus the genesis block.
        int nHeight = std::max(pindex->nHeight - nStep, 0);
        if (Contains(pindex)) {
            // Use O(1) CChain index if possible.
            pindex = (*this)[nHeight];
        } else {
            // Otherwise, use O(log n) skiplist.
            pindex = pindex->GetAncestor(nHeight);
        }
        if (vHave.size() > 10)
            nStep *= 2;
    }

    return CBlockLocator(vHave);
}

const CBlockIndex* CChain::FindFork(const CBlockIndex* pindex) const
{
    if (pindex == NULL) {
        return NULL;
    }    
    if (pindex->nHeight > Height())
        pindex = pindex->GetAncestor(Height());
    while (pindex && !Contains(pindex))
        pindex = pindex->pprev;
    return pindex;
}

uint256 CBlockIndex::GetBlockTrust() const
{
    uint256 bnTarget;
    bnTarget.SetCompact(nBits);
    if (bnTarget <= 0)
        return 0;

    if (IsProofOfStake()) {
        // Return trust score as usual
        return (uint256(1) << 256) / (bnTarget + 1);
    } else {
        // Calculate work amount for block
        uint256 bnPoWTrust = ((~uint256(0) >> 20) / (bnTarget + 1));
        return bnPoWTrust > 1 ? bnPoWTrust : 1;
    }
}

int64_t CBlockIndex::GetMedianTimeSpacing() const
{
    if (nHeight == 0)
        return 0;
        
    int64_t nActualTimespan = 0;
    const CBlockIndex* BlockReading = this;
    int64_t CountBlocks = 0;
    int64_t LastBlockTime = 0;
    int64_t PastBlocksMin = Params().GetPastBlocksMin();
    int64_t PastBlocksMax = Params().GetPastBlocksMax();

    for (unsigned int i = 1; BlockReading && BlockReading->nHeight > 0; i++) {
        if (PastBlocksMax > 0 && i > PastBlocksMax) {
            break;
        }
        CountBlocks++;

        if (LastBlockTime > 0) {
            int64_t Diff = (LastBlockTime - BlockReading->GetBlockTime());
            nActualTimespan += Diff;
        }
        LastBlockTime = BlockReading->GetBlockTime();

        if (BlockReading->pprev == NULL) {
            assert(BlockReading);
            break;
        }
        BlockReading = BlockReading->pprev;
    }

    return nActualTimespan/CountBlocks;
}

const CBlockIndex* GetLastBlockIndex_Legacy(const CBlockIndex* pindex, bool fProofOfStake)
{
    while (pindex && pindex->pprev && (pindex->IsProofOfStake() != fProofOfStake))
        pindex = pindex->pprev;
    return pindex;
}
