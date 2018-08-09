// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The KoreCore developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chainparams.h"
#include "consensus/merkle.h"
#include "main.h"
#include "tinyformat.h"
#include "util.h"
#include "utilstrencodings.h"
#include "pow.h"
#include "arith_uint256.h"
#include "miner.h"
#include "base58.h"

#include <assert.h>

#include <boost/assign/list_of.hpp>

#include "chainparamsseeds.h"

static CBlock CreateGenesisBlock(const char* pszTimestamp, const CScript& genesisOutputScript, uint32_t nTime, uint32_t nNonce, uint32_t nBirthdayA, uint32_t nBirthdayB, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    CMutableTransaction txNew;
    txNew.nTime = nTime;
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig = CScript() << 0 << OP_0;
    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].scriptPubKey = genesisOutputScript;

    CBlock genesis;
    genesis.nTime    = nTime;
    genesis.nBits    = nBits;
    genesis.nNonce   = nNonce;
    genesis.nBirthdayA   = nBirthdayA;
    genesis.nBirthdayB   = nBirthdayB;
    genesis.nVersion = nVersion;
    genesis.vtx.push_back(txNew);
    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
    return genesis;
}

/**
 * Build the genesis block. Note that the output of its generation
 * transaction cannot be spent since it did not originally exist in the
 * database.
 *
 * CBlock(hash=000000000019d6, ver=1, hashPrevBlock=00000000000000, hashMerkleRoot=4a5e1e, nTime=1231006505, nBits=1d00ffff, nNonce=2083236893, vtx=1)
 *   CTransaction(hash=4a5e1e, ver=1, vin.size=1, vout.size=1, nLockTime=0)
 *     CTxIn(COutPoint(000000, -1), coinbase 04ffff001d0104455468652054696d65732030332f4a616e2f32303039204368616e63656c6c6f72206f6e206272696e6b206f66207365636f6e64206261696c6f757420666f722062616e6b73)
 *     CTxOut(nValue=50.00000000, scriptPubKey=0x5F1DF16B2B704C8A578D0B)
 *   vMerkleTree: 4a5e1e
 */
static CBlock CreateGenesisBlock(uint32_t nTime, uint32_t nNonce, uint32_t nBirthdayA, uint32_t nBirthdayB, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    const char* pszTimestamp = "https://bitcoinmagazine.com/articles/altcoins-steal-spotlight-bitcoin-reaches-new-highs/";
    const CScript genesisOutputScript = CScript() << ParseHex("04678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef38c4f35504e51ec112de5c384df7ba0b8d578a4c702b6bf11d5f") << OP_CHECKSIG;
    return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTime, nNonce, nBirthdayA, nBirthdayB, nBits, nVersion, genesisReward);
}

void  CChainParams::MineNewGenesisBlock()
{
    fPrintToConsole = true;
    LogPrintf("Searching for genesis block...\n");

    arith_uint256 hashTarget = UintToArith256(consensus.powLimit);//.GetCompact();
    //arith_uint256 hashTarget = arith_uint256().SetCompact(genesis.nBits);
    while(true) {
        arith_uint256 thash = UintToArith256(genesis.CalculateBestBirthdayHash());
		LogPrintf("teHash %s\n", thash.ToString().c_str());
		LogPrintf("Hash Target %s\n", hashTarget.ToString().c_str());  
      if (thash <= hashTarget)
            break;
        if ((genesis.nNonce & 0xFFF) == 0)
            LogPrintf("nonce %08X: hash = %s (target = %s)\n", genesis.nNonce, thash.ToString().c_str(), hashTarget.ToString().c_str());

        ++genesis.nNonce;
        if (genesis.nNonce == 0) {
            LogPrintf("NONCE WRAPPED, incrementing time\n");
            ++genesis.nTime;
        }
    }
    LogPrintf("genesis.nTime = %u \n",  genesis.nTime);
    LogPrintf("genesis.nNonce = %u \n",  genesis.nNonce);
	LogPrintf("genesis.nBirthdayA: %d\n", genesis.nBirthdayA);
	LogPrintf("genesis.nBirthdayB: %d\n", genesis.nBirthdayB);
    LogPrintf("genesis.nBits = %u \n",  genesis.nBits);
    LogPrintf("genesis.GetHash = %s\n",  genesis.GetHash().ToString().c_str());
    LogPrintf("genesis.hashMerkleRoot = %s\n",  genesis.hashMerkleRoot.ToString().c_str());

    exit(1);
}

/**
 * Main network
 */
/**
 * What makes a good checkpoint block?
 * + Is surrounded by blocks with reasonable timestamps
 *   (no blocks before with a timestamp after, none after with
 *    timestamp before)
 * + Contains no strange transactions
 */

class CMainParams : public CChainParams {
public:
    CMainParams() {
        strNetworkID = "main";
        consensus.nSubsidyHalvingInterval = 4000;
        consensus.nMaxReorganizationDepth = 25;
        consensus.nMajorityEnforceBlockUpgrade = 750;
        consensus.nMajorityRejectBlockOutdated = 950;
        consensus.nMajorityWindow = 1000;
        consensus.BIP34Height = 227931;
        consensus.BIP34Hash = uint256S("0x000000000000024b89b42a942fe0d9fea3bb44ab7bd1b19115dd6a759c0808b8");
        consensus.powLimit = uint256S("1fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.posLimit = uint256S("0000ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        
        consensus.nTargetTimespan = 60;
        consensus.nTargetSpacing = 60;
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 1916; // 95% of 2016
        consensus.nMinerConfirmationWindow = 50; // nPowTargetTimespan / nPowTargetSpacing
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008
        consensus.nStakeTimestampMask = 0xf;
        consensus.nLastPOWBlock = 1000;
        consensus.nStakeMinAge = 4 * 60 * 60;
        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1462060800; // May 1st, 2016
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1493596800; // May 1st, 2017

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 32-bit integer with any alignment.
         */
        pchMessageStart[0] = 0xe4;
        pchMessageStart[1] = 0x7b;
        pchMessageStart[2] = 0xb3;
        pchMessageStart[3] = 0x4a;
        vAlertPubKey = ParseHex("042b0fb78026380244cc458a914dae461899b121f53bc42105d134158b9773e3fdadca67ca3015dc9c4ef9b9df91f2ef05b890a15cd2d2b85930d37376b2196002");
        nDefaultPort = 20743; // 8333;
        nMaxTipAge = 24 * 60 * 60;
        nPruneAfterHeight = 100000;

        genesis = CreateGenesisBlock(1508884606, 22, 12624920, 58284520, 0x201fffff, 1, pow (7,2) * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        //MineNewGenesisBlock();
        assert(consensus.hashGenesisBlock == uint256S("0x0aab10677b4fe0371a67f99e78a69e7d9fa03a1c7d48747978da405dc5abeb99"));
        assert(genesis.hashMerkleRoot == uint256S("0x53e2105c87e985ab3a3a3b3c6921f660f18535f935e447760758d4ed7c4c748c"));

        vSeeds.push_back(CDNSSeedData("kore.life", "dnsseed.kore.life"));

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,45);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,85);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,128);
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x88)(0xB2)(0x1E).convert_to_container<std::vector<unsigned char> >();
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x88)(0xAD)(0xE4).convert_to_container<std::vector<unsigned char> >();

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_main, pnSeed6_main + ARRAYLEN(pnSeed6_main));

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;
        fTestnetToBeDeprecatedFieldRPC = false;

        nPoolMaxTransactions = 3;
        strSporkKey = "0427E31B51989DB4DFEAB8C3901FB1862A621E6B0D4CF556E5C9AAD7283A46C915EC4508FB4F248534C3A03FC0475ED3785086B9C217E0F42ED4C8BF80ED2296C8";
        strObfuscationPoolDummyAddress = "KWFvN4Gb55dzG95cq3k5jXFmNVkJLftyjZ";
        nStartMasternodePayments = 1508884606; //Genesis time

        static MapCheckpoints mapCheckpoints =
            boost::assign::map_list_of
            (0, uint256S("0x0aab10677b4fe0371a67f99e78a69e7d9fa03a1c7d48747978da405dc5abeb99"));
        static const CCheckpointData data = {
            mapCheckpoints,
            1529501760, // * UNIX timestamp of last checkpoint block
            422518,    // * total number of transactions between genesis and last checkpoint
                        //   (the tx=... number in the SetBestChain debug.log lines)
            1440        // * estimated number of transactions per day after checkpoint
        };
    }
};
static CMainParams mainParams;

/**
 * Testnet (v3)
 */
class CTestNetParams : public CChainParams {
public:
    CTestNetParams() {
        strNetworkID = "test";
        consensus.nSubsidyHalvingInterval = 131250;
        consensus.nMajorityEnforceBlockUpgrade = 51;
        consensus.nMajorityRejectBlockOutdated = 75;
        consensus.nMajorityWindow = 100;
        consensus.BIP34Height = 21111;
        consensus.BIP34Hash = uint256S("0x0000000023b3a96d3484e5abb3755c413e7d41500f8e2a5c3f0dd01299cd8ef8");
        consensus.powLimit = uint256S("00ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nTargetTimespan = 12 * 60; // two weeks
        consensus.nTargetSpacing = 3 * 60;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 1512; // 75% for testchains
        consensus.nMinerConfirmationWindow = 2016; // nPowTargetTimespan / nPowTargetSpacing
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1456790400; // March 1st, 2016
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1493596800; // May 1st, 2017

        pchMessageStart[0] = 0x18;
        pchMessageStart[1] = 0x15;
        pchMessageStart[2] = 0x14;
        pchMessageStart[3] = 0x88;
        vAlertPubKey = ParseHex("04cd7ce93858b4257079f4ed9150699bd9f66437ff76617690d1cc180321e94ea391bbccf3bccdcf2edaf0429e32c07b53354e9cecf458cca3fe71dc277f11d9c5");
        nDefaultPort = 11743; // 18333;
        nMaxTipAge = 0x7fffffff;
        nPruneAfterHeight = 1000;

        genesis = CreateGenesisBlock(1453993470, 414098458, 0, 0, 0x1d00ffff, 1, 48 * COIN); // 50 * COIN);
        // MineBlock(&genesis); // REMOVE
        consensus.hashGenesisBlock = genesis.GetHash();

        //assert(consensus.hashGenesisBlock == uint256S("0x9cc9b3b19831e0bfde990f1c2dd9c655fa4414a47c9c7d4a6843b0994febb419"));
        //assert(genesis.hashMerkleRoot == uint256S("0xaab825e79a76eff348a54e8dcfadcc6a1e93b00fad9ff5d4971367d367e60b2d"));

        vFixedSeeds.clear();
        vSeeds.clear();
/*
        vSeeds.push_back(CDNSSeedData("kore.petertodd.org", "testnet-seed.kore.petertodd.org"));
        vSeeds.push_back(CDNSSeedData("bluematt.me", "testnet-seed.bluematt.me"));
        vSeeds.push_back(CDNSSeedData("kore.schildbach.de", "testnet-seed.kore.schildbach.de"));
*/
        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,105);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,190);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,233);
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x35)(0x87)(0xCF).convert_to_container<std::vector<unsigned char> >();
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x35)(0x83)(0x94).convert_to_container<std::vector<unsigned char> >();

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_test, pnSeed6_test + ARRAYLEN(pnSeed6_test));

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        fMineBlocksOnDemand = false;
        fTestnetToBeDeprecatedFieldRPC = true;

        checkpointData = (CCheckpointData){
            boost::assign::map_list_of
            ( 0, uint256S("0x")),
            0,
            0,
            0
        };

    }
};
static CTestNetParams testNetParams;

/**
 * Regression test
 */
class CRegTestParams : public CChainParams {
public:
    CRegTestParams() {
        strNetworkID = "regtest";
        consensus.nSubsidyHalvingInterval = 150;
        consensus.nMajorityEnforceBlockUpgrade = 750;
        consensus.nMajorityRejectBlockOutdated = 950;
        consensus.nMajorityWindow = 1000;
        consensus.BIP34Height = -1; // BIP34 has not necessarily activated on regtest
        consensus.BIP34Hash = uint256();
        consensus.powLimit = uint256S("00ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nTargetTimespan = 60 * 60; // one hour
        consensus.nTargetSpacing = 1 * 60;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = true;
        consensus.nRuleChangeActivationThreshold = 108; // 75% for testchains
        consensus.nMinerConfirmationWindow = 144; // Faster than normal for regtest (144 instead of 2016)
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 999999999999ULL;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 999999999999ULL;

        pchMessageStart[0] = 0xcf;
        pchMessageStart[1] = 0x05;
        pchMessageStart[2] = 0x6a;
        pchMessageStart[3] = 0xe1;
        nMaxTipAge = 24 * 60 * 60;
        nDefaultPort = 18444;
        nPruneAfterHeight = 1000;

        genesis = CreateGenesisBlock(1453993470, 2,0,0, 0x207fffff, 1, 48 * COIN); // 50 * COIN);
//        MineBlock(&genesis); // REMOVE
        consensus.hashGenesisBlock = genesis.GetHash();

        //3779f8aeb763c0b383376497dd7fb83686cae0d78b7630096baaec404b0d6754
        //assert(consensus.hashGenesisBlock == uint256S("0xa2a8e906aec004c6be800f3769c184721de7ce926e4918ecdbe611f13b047f7b"));
        //0fef9c33f77531f903de86949cbc89740da74c88e28a1386f6b70bdb58a35429
        //assert(genesis.hashMerkleRoot == uint256S("0xaab825e79a76eff348a54e8dcfadcc6a1e93b00fad9ff5d4971367d367e60b2d"));

        vFixedSeeds.clear(); //! Regtest mode doesn't have any fixed seeds.
        vSeeds.clear();  //! Regtest mode doesn't have any DNS seeds.

        fMiningRequiresPeers = false;
        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        fMineBlocksOnDemand = true;
        fTestnetToBeDeprecatedFieldRPC = false;

        checkpointData = (CCheckpointData){
            boost::assign::map_list_of
            ( 0, uint256S("0x")),
            0,
            0,
            0
        };
        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,105);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,190);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,233);
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x35)(0x87)(0xCF).convert_to_container<std::vector<unsigned char> >();
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x35)(0x83)(0x94).convert_to_container<std::vector<unsigned char> >();
    }
};
static CRegTestParams regTestParams;

static CChainParams *pCurrentParams = 0;

const CChainParams &Params() {
    assert(pCurrentParams);
    return *pCurrentParams;
}

CChainParams& Params(const std::string& chain)
{
    if (chain == CBaseChainParams::MAIN)
            return mainParams;
    else if (chain == CBaseChainParams::TESTNET)
            return testNetParams;
    else if (chain == CBaseChainParams::REGTEST)
            return regTestParams;
    else
        throw std::runtime_error(strprintf("%s: Unknown chain %s.", __func__, chain));
}

void SelectParams(const std::string& network)
{
    SelectBaseParams(network);
    pCurrentParams = &Params(network);
}
