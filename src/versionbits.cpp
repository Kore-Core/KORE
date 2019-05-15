// Copyright (c) 2016 The Kore Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "versionbits.h"

const struct BIP9DeploymentInfo VersionBitsDeploymentInfo[CChainParams::MAX_VERSION_BITS_DEPLOYMENTS] = {
    {
        /*.name =*/"csv",
        /*.gbt_force =*/true,
    }};

ThresholdState AbstractThresholdConditionChecker::GetStateFor(const CBlockIndex* pindexPrev, ThresholdConditionCache& cache) const
{
    int nPeriod = Period();
    int nThreshold = Threshold();
    int64_t nTimeStart = BeginTime();
    int64_t nTimeTimeout = EndTime();

    // A block's state is always the same as that of the first of its period, so it is computed based on a pindexPrev whose height equals a multiple of nPeriod - 1.
    if (pindexPrev != NULL) {
        pindexPrev = pindexPrev->GetAncestor(pindexPrev->nHeight - ((pindexPrev->nHeight + 1) % nPeriod));
    }

    // Walk backwards in steps of nPeriod to find a pindexPrev whose information is known
    std::vector<const CBlockIndex*> vToCompute;
    while (cache.count(pindexPrev) == 0) {
        if (pindexPrev == NULL) {
            // The genesis block is by definition defined.
            cache[pindexPrev] = THRESHOLD_DEFINED;
            break;
        }
        if (pindexPrev->GetMedianTimePast() < nTimeStart) {
            // Optimizaton: don't recompute down further, as we know every earlier block will be before the start time
            cache[pindexPrev] = THRESHOLD_DEFINED;
            break;
        }
        vToCompute.push_back(pindexPrev);
        pindexPrev = pindexPrev->GetAncestor(pindexPrev->nHeight - nPeriod);
    }

    // At this point, cache[pindexPrev] is known
    assert(cache.count(pindexPrev));
    ThresholdState state = cache[pindexPrev];

    // Now walk forward and compute the state of descendants of pindexPrev
    while (!vToCompute.empty()) {
        ThresholdState stateNext = state;
        pindexPrev = vToCompute.back();
        vToCompute.pop_back();

        switch (state) {
        case THRESHOLD_DEFINED: {
            if (pindexPrev->GetMedianTimePast() >= nTimeTimeout) {
                stateNext = THRESHOLD_FAILED;
            } else if (pindexPrev->GetMedianTimePast() >= nTimeStart) {
                stateNext = THRESHOLD_STARTED;
            }
            break;
        }
        case THRESHOLD_STARTED: {
            if (pindexPrev->GetMedianTimePast() >= nTimeTimeout) {
                stateNext = THRESHOLD_FAILED;
                break;
            }
            // We need to count
            const CBlockIndex* pindexCount = pindexPrev;
            int count = 0;
            for (int i = 0; i < nPeriod; i++) {
                if (Condition(pindexCount)) {
                    count++;
                }
                pindexCount = pindexCount->pprev;
            }
            if (count >= nThreshold) {
                stateNext = THRESHOLD_LOCKED_IN;
            }
            break;
        }
        case THRESHOLD_LOCKED_IN: {
            // Always progresses into ACTIVE.
            stateNext = THRESHOLD_ACTIVE;
            break;
        }
        case THRESHOLD_FAILED:
        case THRESHOLD_ACTIVE: {
            // Nothing happens, these are terminal states.
            break;
        }
        }
        cache[pindexPrev] = state = stateNext;
    }

    return state;
}

namespace
{
/**
 * Class to implement versionbits logic.
 */
class VersionBitsConditionChecker : public AbstractThresholdConditionChecker
{
private:
    const CChainParams::DeploymentPos id;

protected:
    int64_t BeginTime() const { return Params().GetVDeployments()[id].nStartTime; }
    int64_t EndTime() const { return Params().GetVDeployments()[id].nTimeout; }
    int Period() const { return Params().GetMinerConfirmationWindow(); }
    int Threshold() const { return Params().GetRuleChangeActivationThreshold(); }

    bool Condition(const CBlockIndex* pindex) const
    {
        return (((pindex->nVersion & VERSIONBITS_TOP_MASK) == VERSIONBITS_TOP_BITS) && (pindex->nVersion & Mask()) != 0);
    }

public:
    VersionBitsConditionChecker(CChainParams::DeploymentPos id_) : id(id_) {}
    uint32_t Mask() const { return ((uint32_t)1) << Params().GetVDeployments()[id].bit; }
};

} // namespace

ThresholdState VersionBitsState(const CBlockIndex* pindexPrev, CChainParams::DeploymentPos pos, VersionBitsCache& cache)
{
    return VersionBitsConditionChecker(pos).GetStateFor(pindexPrev, cache.caches[pos]);
}

uint32_t VersionBitsMask(CChainParams::DeploymentPos pos)
{
    return VersionBitsConditionChecker(pos).Mask();
}

void VersionBitsCache::Clear()
{
    for (unsigned int d = 0; d < CChainParams::MAX_VERSION_BITS_DEPLOYMENTS; d++) {
        caches[d].clear();
    }
}
