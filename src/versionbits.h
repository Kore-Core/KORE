// Copyright (c) 2016 The Kore Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_CONSENSUS_VERSIONBITS
#define BITCOIN_CONSENSUS_VERSIONBITS

#include "chain.h"
#include "chainparams.h"
#include <map>

/** What block version to use for new blocks (pre versionbits) */
static const int32_t VERSIONBITS_LAST_OLD_BLOCK_VERSION = 4;
/** What bits to set in version for versionbits blocks */
static const int32_t VERSIONBITS_TOP_BITS = 0x20000000UL;
/** What bitmask determines whether versionbits is in use */
static const int32_t VERSIONBITS_TOP_MASK = 0xE0000000UL;
/** Total bits available for versionbits */
static const int32_t VERSIONBITS_NUM_BITS = 29;

enum ThresholdState {
    THRESHOLD_DEFINED,
    THRESHOLD_STARTED,
    THRESHOLD_LOCKED_IN,
    THRESHOLD_ACTIVE,
    THRESHOLD_FAILED,
};

// A map that gives the state for blocks whose height is a multiple of Period().
// The map is indexed by the block's parent, however, so all keys in the map
// will either be NULL or a block with (height + 1) % Period() == 0.
typedef std::map<const CBlockIndex*, ThresholdState> ThresholdConditionCache;

struct BIP9DeploymentInfo {
    /** Deployment name */
    const char *name;
    /** Whether GBT clients can safely ignore this rule in simplified usage */
    bool gbt_force;
};

extern const struct BIP9DeploymentInfo VersionBitsDeploymentInfo[];

/**
 * Abstract class that implements BIP9-style threshold logic, and caches results.
 */
class AbstractThresholdConditionChecker {
protected:
    virtual bool Condition(const CBlockIndex* pindex) const =0;
    virtual int64_t BeginTime() const =0;
    virtual int64_t EndTime() const =0;
    virtual int Period() const =0;
    virtual int Threshold() const =0;

public:
    // Note that the function below takes a pindexPrev as input: they compute information for block B based on its parent.
    ThresholdState GetStateFor(const CBlockIndex* pindexPrev, ThresholdConditionCache& cache) const;
};

struct VersionBitsCache
{
    ThresholdConditionCache caches[CChainParams::MAX_VERSION_BITS_DEPLOYMENTS];

    void Clear();
};

ThresholdState VersionBitsState(const CBlockIndex* pindexPrev, CChainParams::DeploymentPos pos, VersionBitsCache& cache);
uint32_t VersionBitsMask(CChainParams::DeploymentPos pos);

#endif
