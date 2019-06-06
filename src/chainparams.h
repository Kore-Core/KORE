// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015-2018 The KORE developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_CHAINPARAMS_H
#define BITCOIN_CHAINPARAMS_H

#include "chainparamsbase.h"
#include "checkpoints.h"
#include "primitives/block.h"
#include "primitives/transaction.h"
#include "protocol.h"
#include "uint256.h"

#include <vector>

typedef std::map<int, uint256> MapCheckpoints;

struct CCheckpointData {
    MapCheckpoints mapCheckpoints;
};

/**
 * Holds various statistics on transactions within a chain. Used to estimate
 * verification progress during chain sync.
 *
 * See also: CChainParams::TxData, GuessVerificationProgress.
 */
struct ChainTxData {
    int64_t nTime;    //!< UNIX timestamp of last known number of transactions
    int64_t nTxCount; //!< total number of transactions between genesis and that timestamp
    double dTxRate;   //!< estimated number of transactions per second after that timestamp
};

typedef unsigned char MessageStartChars[MESSAGE_START_SIZE];

struct CDNSSeedData {
    std::string name, host;
    CDNSSeedData(const std::string& strName, const std::string& strHost) : name(strName), host(strHost) {}
};

/**
 * CChainParams defines various tweakable parameters of a given instance of the
 * KORE system. There are three: the main network on which people trade goods
 * and services, the public test network which gets reset from time to time and
 * a regression test mode which is intended for private networks only. It has
 * minimal difficulty to ensure that blocks can be found instantly.
 */
class CChainParams
{
public:
    enum Base58Type {
        PUBKEY_ADDRESS,
        SCRIPT_ADDRESS,
        SECRET_KEY,     // BIP16
        EXT_PUBLIC_KEY, // BIP32
        EXT_SECRET_KEY, // BIP32
        EXT_COIN_TYPE,  // BIP44
        MAX_BASE58_TYPES
    };

    enum DeploymentPos {
        DEPLOYMENT_CSV, // Deployment of BIP68, BIP112, and BIP113.
        // NOTE: Also add new deployments to VersionBitsDeploymentInfo in versionbits.cpp
        MAX_VERSION_BITS_DEPLOYMENTS
    };

    struct BIP9Deployment {
        /** Bit position to select the particular bit in nVersion. */
        int bit;
        /** Start MedianTime for version bits miner confirmation. Can be a date in the past */
        int64_t nStartTime;
        /** Timeout/expiry MedianTime for the deployment attempt. */
        int64_t nTimeout;
    };

    typedef BIP9Deployment vDeployments_type[MAX_VERSION_BITS_DEPLOYMENTS];

    const std::vector<unsigned char>&      AlertKey() const                    { return vAlertPubKey; }
    const int32_t                          HeightToBanOldWallets() const       { return nBlocksToBanOldWallets; }
    const int32_t                          HeightToFork() const                { return nHeightToFork; };
    const uint256&                         HashGenesisBlock() const            { return nHashGenesisBlock; }
    const MessageStartChars&               MessageStart() const                { return pchMessageStart; }
    const uint256&                         ProofOfWorkLimit() const            { return bnProofOfWorkLimit; }
    const uint256&                         ProofOfStakeLimit() const           { return bnProofOfStakeLimit; }
    const CBlock&                          GenesisBlock() const                { return genesis; }
    const CChainParams::vDeployments_type& GetVDeployments() const             { return vDeployments; }
    const std::vector<CDNSSeedData>&       DNSSeeds() const                    { return vSeeds; }
    const std::vector<unsigned char>&      Base58Prefix(Base58Type type) const { return base58Prefixes[type]; }
    const std::vector<CAddress>&           FixedSeeds() const                  { return vFixedSeeds; }

    bool                      EnableBigRewards() const                 { return fEnableBigReward; }
    /** Make miner wait to have peers to avoid wasting work */
    bool                      DoesMiningRequiresPeers() const          { return fMiningRequiresPeers; }
    int64_t                   GetBudgetFeeConfirmations() const        { return nBudgetFeeConfirmations; }
    int64_t                   GetBudgetVoteUpdate() const              { return nBudgetVoteUpdate; }
    int32_t                   GetCoinMaturity() const                  { return nCoinMaturity; }
    /** Used if GenerateKores is called with a negative number of threads */
    int32_t                   GetDefaultMinerThreads() const           { return nMinerThreads; }
    int32_t                   GetDefaultPort() const                   { return nDefaultPort; }
    std::string               GetDevFundPubKey() const                 { return strDevFundPubKey; }
    int32_t                   GetLastPoWBlock() const                  { return nLastPOWBlock; }
    CAmount                   GetMaxMoneyOut() const                   { return nMaxMoneyOut; }
    int32_t                   GetMaxReorganizationDepth() const        { return nMaxReorganizationDepth; }
    int32_t                   GetMaxStakeModifierInterval() const      { return std::min(nCoinMaturity, 64U); }
    int64_t                   GetMaxTipAge() const                     { return nMaxTipAge; }
    uint32_t                  GetMinerConfirmationWindow() const       { return nMinerConfirmationWindow; }
    uint32_t                  GetModifierInterval() const              { return nTargetSpacing; }
    CBaseChainParams::Network GetNetworkID() const                     { return networkID; }
    /** Return the BIP70 network string (main, test or regtest) */
    std::string               GetNetworkIDString() const               { return strNetworkID; }
    int32_t                   GetPoolMaxTransactions() const           { return nPoolMaxTransactions; }
    uint32_t                  GetRuleChangeActivationThreshold() const { return nRuleChangeActivationThreshold; }
    int64_t                   GetPastBlocksMax() const                 { return nPastBlocksMax; }
    int64_t                   GetPastBlocksMin() const                 { return nPastBlocksMin; }
    // minimum spacing is maturity - 1
    int64_t                   GetStakeLockInterval() const             { return nStakeLockInterval; }
    int64_t                   GetStakeLockSequenceNumber() const       { return (nStakeLockInterval >> CTxIn::SEQUENCE_LOCKTIME_GRANULARITY) | CTxIn::SEQUENCE_LOCKTIME_TYPE_FLAG; }
    uint32_t                  GetStakeMinAge() const                   { return nStakeMinAge; }
    uint32_t                  GetTargetSpacing() const                 { return nTargetSpacing; }
    int64_t                   GetTargetSpacingForStake() const         { return nTargetSpacing * .75; }
    uint32_t                  GetTargetTimespan() const                { return nTargetSpacing; }
    /** Default value for -checkmempool and -checkblockindex argument */
    bool                      IsConsistencyChecksDefault() const       { return fDefaultConsistencyChecks; }
    uint64_t                  PruneAfterHeight() const                 { return nPruneAfterHeight; }
    /** Make standard checks */
    bool                      RequireStandard() const                  { return fRequireStandard; }
    /** Make miner stop after a block is found. In RPC, don't return until nGenProcLimit blocks are generated */
    bool                      ShouldMineBlocksOnDemand() const         { return fMineBlocksOnDemand; }
    /** Skip proof-of-work check: allow mining of any difficulty block */
    bool                      SkipProofOfWorkCheck() const             { return fSkipProofOfWorkCheck; }
    
    const CCheckpointData&    GetCheckpoints() const                   { return checkpointData; }
    const ChainTxData&        GetTxData() const                        { return chainTxData; }

protected:
    CChainParams() {}

    uint256                    bnProofOfWorkLimit;
    uint256                    bnProofOfStakeLimit;
    bool                       fMiningRequiresPeers;
    bool                       fDefaultConsistencyChecks;
    bool                       fRequireStandard;
    bool                       fMineBlocksOnDemand;
    bool                       fSkipProofOfWorkCheck;
    bool                       fEnableBigReward;
    CBlock                     genesis;
    int64_t                    nBudgetFeeConfirmations;
    int64_t                    nBudgetVoteUpdate;
    uint32_t                   nCoinMaturity;
    int32_t                    nDefaultPort;
    uint256                    nHashGenesisBlock;
    int32_t                    nBlocksToBanOldWallets;
    int32_t                    nHeightToFork;    
    MessageStartChars          pchMessageStart;
    int32_t                    nMaxReorganizationDepth;
    int64_t                    nMaxTipAge;
    uint64_t                   nPruneAfterHeight; // Legacy
    int32_t                    nLastPOWBlock;
    CAmount                    nMaxMoneyOut;
    uint32_t                   nMinerConfirmationWindow;
    int32_t                    nMinerThreads;
    CBaseChainParams::Network  networkID;
    std::string                strNetworkID;
    int64_t                    nPastBlocksMin; // used when calculating the NextWorkRequired
    int64_t                    nPastBlocksMax;
    int32_t                    nPoolMaxTransactions;
    uint32_t                   nRuleChangeActivationThreshold;
    int64_t                    nStakeLockInterval;
    uint32_t                   nStakeMinAge;
    uint32_t                   nTargetSpacing;
    int64_t                    nTargetSpacingForStake;
    std::string                strDevFundPubKey;
    //! Raw pub key bytes for the broadcast alert signing key.
    std::vector<unsigned char> vAlertPubKey;
    vDeployments_type          vDeployments;
    std::vector<CAddress>      vFixedSeeds;
    std::vector<CDNSSeedData>  vSeeds;
    std::vector<unsigned char> base58Prefixes[MAX_BASE58_TYPES];
    
    CCheckpointData            checkpointData;
    ChainTxData                chainTxData;

    void MineNewGenesisBlock_Legacy();
};

/**
 * Modifiable parameters interface is used by test cases to adapt the parameters in order
 * to test specific features more easily. Test cases should always restore the previous
 * values after finalization.
 */

class CModifiableParams
{
public:
    //! Published setters to allow changing values in unit test cases
    virtual void setCoinMaturity(int aCoinMaturity) = 0;
    virtual void setEnableBigRewards(bool bigRewards) = 0;
    virtual void setHeightToFork(int aHeightToFork) = 0;
    virtual void setLastPowBlock(int aLastPOWBlock) = 0;
    virtual void setStakeLockInterval(int aStakeLockInterval) = 0;
    virtual void setStakeMinAge(int aStakeMinAge) = 0;
    virtual void setTargetSpacing(uint32_t aTargetSpacing) = 0;
};

/**
 * Return the currently selected parameters. This won't change after app startup
 * outside of the unit tests.
 */
const CChainParams& Params();

/** Return parameters for the given network. */
CChainParams& Params(CBaseChainParams::Network network);

/** Get modifiable network parameters (UNITTEST only) */
CModifiableParams* ModifiableParams();

/** Sets the params returned by Params() to those for the given network. */
void SelectParams(CBaseChainParams::Network network);

/**
 * Looks for -regtest or -testnet and then calls SelectParams as appropriate.
 * Returns false if an invalid combination is given.
 */
bool SelectParamsFromCommandLine();

#endif // BITCOIN_CHAINPARAMS_H
