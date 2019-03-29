// Copyright (c) 2009-2014 The Bitcoin developers
// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015-2017 The KORE developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "checkpoints.h"

#include "chainparams.h"
#include "main.h"
#include "uint256.h"

#include <stdint.h>

#include <boost/foreach.hpp>

namespace Checkpoints
{
/**
     * How many times we expect transactions after the last checkpoint to
     * be slower. This number is a compromise, as it can't be accurate for
     * every system. When reindexing from a fast disk with a slow CPU, it
     * can be up to 20, while when downloading from a slow network with a
     * fast multicore CPU, it won't be much higher than 1.
     */
static const double SIGCHECK_VERIFICATION_FACTOR = 5.0;

bool fEnabled = DEFAULT_CHECKPOINTS_ENABLED;

bool CheckBlock(int nHeight, const uint256& hash, bool fMatchesCheckpoint)
{
    if (!fEnabled)
        return true;

    const MapCheckpoints& checkpoints = Params().GetCheckpoints().mapCheckpoints;

    MapCheckpoints::const_iterator i = checkpoints.find(nHeight);
    // If looking for an exact match, then return false
    if (i == checkpoints.end()) return !fMatchesCheckpoint;
    return hash == i->second;
}

//! Guess how far we are in the verification process at the given block index
//! require cs_main if pindex has not been validated yet (because nChainTx might be unset)
double GuessVerificationProgress(const ChainTxData& data, const CBlockIndex *pindex) {
    if (pindex == nullptr)
        return 0.0;

    int64_t nNow = time(nullptr);

    double fTxTotal;

    if (pindex->nChainTx <= data.nTxCount) {
        fTxTotal = data.nTxCount + (nNow - data.nTime) * data.dTxRate;
    } else {
        fTxTotal = pindex->nChainTx + (nNow - pindex->GetBlockTime()) * data.dTxRate;
    }

    return pindex->nChainTx / fTxTotal;
}

int GetTotalBlocksEstimate()
{
    if (!fEnabled)
        return 0;

    const MapCheckpoints& checkpoints = Params().GetCheckpoints().mapCheckpoints;
    if (checkpoints.empty())
        return 0;

    return checkpoints.rbegin()->first;
}

CBlockIndex* GetLastCheckpoint()
{
    if (!fEnabled)
        return NULL;

    const MapCheckpoints& checkpoints = Params().GetCheckpoints().mapCheckpoints;

    BOOST_REVERSE_FOREACH (const MapCheckpoints::value_type& i, checkpoints) {
        const uint256& hash = i.second;
        BlockMap::const_iterator t = mapBlockIndex.find(hash);
        if (t != mapBlockIndex.end())
            return t->second;
    }
    return NULL;
}

} // namespace Checkpoints
