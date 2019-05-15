// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015-2018 The KORE developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chainparams.h"
#include "amount.h"
#include "arith_uint256.h"
#include "kernel.h"
#include "random.h"
#include "util.h"
#include "utilstrencodings.h"

#include <assert.h>

#include <boost/assign/list_of.hpp>

using namespace std;
using namespace boost::assign;

struct SeedSpec6 {
    uint8_t addr[16];
    uint16_t port;
};

#include "chainparamsseeds.h"

static CBlock CreateGenesisBlock(const char* pszTimestamp, const CScript& genesisOutputScript, uint32_t nTime, uint32_t nNonce, uint32_t nBirthdayA, uint32_t nBirthdayB, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    CMutableTransaction txNew;
    txNew.vin.resize(1);
    txNew.vout.resize(1);

    txNew.nTime = nTime;
    txNew.nVersion = 1;

    if (pszTimestamp != NULL)
        txNew.vin[0].scriptSig = CScript() << 486604799 << CScriptNum(4) << vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
    else
        txNew.vin[0].scriptSig = CScript() << 0 << OP_0;
    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].scriptPubKey = genesisOutputScript;

    CBlock genesis;
    genesis.nTime       = nTime;
    genesis.nBits       = nBits;
    genesis.nNonce      = nNonce;
    genesis.nBirthdayA  = nBirthdayA;
    genesis.nBirthdayB  = nBirthdayB;
    genesis.nVersion    = nVersion;
    genesis.vtx.push_back(txNew);
    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = genesis.BuildMerkleTree();

    return genesis;
}

/**
 * Main network
 */

//! Convert the pnSeeds6 array into usable address objects.
static void convertSeed6(std::vector<CAddress>& vSeedsOut, const SeedSpec6* data, unsigned int count)
{
    // It'll only connect to one or two seed nodes because once it connects,
    // it'll get a pile of addresses with newer timestamps.
    // Seed nodes are given a random 'last seen time' of between one and two
    // weeks ago.
    const int64_t nOneWeek = 7 * 24 * 60 * 60;
    for (unsigned int i = 0; i < count; i++) {
        struct in6_addr ip;
        memcpy(&ip, data[i].addr, sizeof(ip));
        CAddress addr(CService(ip, data[i].port));
        addr.nTime = GetTime() - GetRand(nOneWeek) - nOneWeek;
        vSeedsOut.push_back(addr);
    }
}

/**
 * 
 * Follow this rules in order to get the correct stake modifier
 * nCoinMaturity    : 25
 * remember that the miminum spacing is 10 !!!
 * nCoinbaseMaturity = nStakeMinConfirmations = nCoinMaturity
 * nTargetSpacing    : max(nCoinMaturity-1, value)
 * pow blocks        : max(nCoinMaturity+1, value), this way we will have 2 modifiers
 */
class CMainParams : public CChainParams
{
public:
    CMainParams()
    {
        networkID = CBaseChainParams::MAIN;
        strNetworkID = "main";
        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 4-byte int at any alignment.
         */
        pchMessageStart[0]                            = 0xe4;
        pchMessageStart[1]                            = 0x7b;
        pchMessageStart[2]                            = 0xb3;
        pchMessageStart[3]                            = 0x4a;

        // Start to set main chain consensus data
        // base58Prefixes[EXT_COIN_TYPE]                 = boost::assign::list_of(0x80)(0x00)(0x00)(0x77).convert_to_container<std::vector<unsigned char> >();
        base58Prefixes[EXT_PUBLIC_KEY]                = boost::assign::list_of(0x04)(0x88)(0xB2)(0x1E).convert_to_container<std::vector<unsigned char> >();
        base58Prefixes[EXT_SECRET_KEY]                = boost::assign::list_of(0x04)(0x88)(0xAD)(0xE4).convert_to_container<std::vector<unsigned char> >();
        base58Prefixes[PUBKEY_ADDRESS]                = std::vector<unsigned char>(1, 45);
        base58Prefixes[SCRIPT_ADDRESS]                = std::vector<unsigned char>(1, 85);
        base58Prefixes[SECRET_KEY]                    = std::vector<unsigned char>(1, 128);
        bnProofOfStakeLimit                           = ~uint256(0) >> 16;   // 0000 FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF 
        bnProofOfWorkLimit                            = ~uint256(0) >> 3;    // 1FFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF
        fDefaultConsistencyChecks                     = false;
        fEnableBigReward 							  = false;
        fMineBlocksOnDemand                           = false;
        fMiningRequiresPeers                          = true;
        fRequireStandard                              = true;
        fSkipProofOfWorkCheck                         = false;
        nDefaultPort                                  = 10743;
        nCoinMaturity                                 = 25;
        nMaxMoneyOut                                  = MAX_MONEY;
        nMaxReorganizationDepth                       = 25;
        nMaxTipAge                                    = 24 * 60 * 60;
        nMinerConfirmationWindow                      = 50;                  // nPowTargetTimespan / nPowTargetSpacing
        nMinerThreads                                 = 0;
        nPastBlocksMax                                = 128;
        nPastBlocksMin                                = 24;
        nPoolMaxTransactions                          = 3;
        nPruneAfterHeight                             = 100000;              // Legacy
        nRuleChangeActivationThreshold                = 1916;                // 95% of 2016
        nStakeLockInterval                            = 2 * 60 * 60;         // Stake remains locked for 4 hours
        nStakeMinAge                                  = 2 * 60 * 60;
        nTargetSpacing                                = 1 * 60;              // [nStakeMinConfirmations-1, max(nStakeMinConfirmations-1, any bigger value)]
        nBlocksToBanOldWallets                        = 1440;                // Ban old nodes one day before fork
        nHeightToFork                                 = 483063;              // Height to perform the fork
        nLastPOWBlock                                 = 1000;
        strDevFundPubKey 				 			  = "04D410C4A7FEC6DBF6FEDC9721104ADA1571D5E3E4791085EFC083A9F3F4C007D240A6A647DDA0CA1466641B0739A86A67B97AC48484FC7CA88257804B7CE52ED2";
        vAlertPubKey                                  = ParseHex("042b0fb78026380244cc458a914dae461899b121f53bc42105d134158b9773e3fdadca67ca3015dc9c4ef9b9df91f2ef05b890a15cd2d2b85930d37376b2196002");
        
        // Deployment of BIP68, BIP112, and BIP113.
        vDeployments[DEPLOYMENT_CSV].bit 			  = 0;
        vDeployments[DEPLOYMENT_CSV].nStartTime       = 1462060800; 		 // May 1st, 2016
        vDeployments[DEPLOYMENT_CSV].nTimeout 	      = 1493596800;   		 // May 1st, 2017

        CScript genesisOutputScript = CScript() << ParseHex("04678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef38c4f35504e51ec112de5c384df7ba0b8d578a4c702b6bf11d5f") << OP_CHECKSIG;
        genesis = CreateGenesisBlock(NULL, genesisOutputScript, 1508884606, 22, 12624920, 58284520, 0x201fffff, 1, pow (7,2) * COIN);

        nHashGenesisBlock = genesis.GetHash();
        assert(nHashGenesisBlock == uint256("0x0aab10677b4fe0371a67f99e78a69e7d9fa03a1c7d48747978da405dc5abeb99"));
        assert(genesis.hashMerkleRoot == uint256S("0x53e2105c87e985ab3a3a3b3c6921f660f18535f935e447760758d4ed7c4c748c"));

        // Primary DNS Seeder
        vSeeds.push_back(CDNSSeedData("kore-dnsseed-1", "dnsseed.kore.life"));
        vSeeds.push_back(CDNSSeedData("kore-dnsseed-2", "dnsseed2.kore.life"));
        vSeeds.push_back(CDNSSeedData("kore-dnsseed-3", "dnsseed3.kore.life"));
        vSeeds.push_back(CDNSSeedData("kore-dnsseed-4", "dnsseed4.kore.life"));				
		
        convertSeed6(vFixedSeeds, pnSeed6_main, ARRAYLEN(pnSeed6_main));

        //   What makes a good checkpoint block?
        // + Is surrounded by blocks with reasonable timestamps
        //   (no blocks before with a timestamp after, none after with timestamp before)
        // + Contains no strange transactions
        checkpointData = {
            {
                {     0, nHashGenesisBlock},
                {     5, uint256S("0x00eaaa465402e6bcf745c00c38c0033a26e4dea19448d9109e4555943d677a31")},
                {  1000, uint256S("0x2073f0a245cedde8344c2d0b48243a58908ffa50b02e2378189f2bb80037abd9")}, // last PoW block, begin PoS
                { 40000, uint256S("0x572b31cc34f842aecbbc89083f7e40fff6a07e73e6002be75cb95468f4e3b4ca")},
                { 80000, uint256S("0x070aa76a8a879f3946322086a542dd9e4afca81efafd7642192ed9fe56ba74f1")},
                {120000, uint256S("0x70edc85193638b8adadb71ea766786d207f78a173dd13f965952eb76932f5729")},
                {209536, uint256S("0x8a718dbb44b57a5693ac70c951f2f81a01b39933e3e19e841637f757598f571a")},
                {300000, uint256S("0xb0d6c4c7240b03e70587bb52ebdc63a694a90f22b30fb73856b5cc3d192a231f")},
                {400000, uint256S("0x59aee83d1f027d2107a8a9c4951767a27eb2224b24022b89f6b9247d2ebb4fdd")},
                {450000, uint256S("0xa03c16b67f4c8303e0df024b81de65619ebe5f30cc7cd02fe2049b384f2a3a84")}
            }
        };

        chainTxData = ChainTxData{
        // Data from rpc: getchaintxstats 
        /* nTime    */ 1554323680,
        /* nTxCount */ 937630,
        /* dTxRate  */ 0.01864469112830689
        };
    }
};
static CMainParams mainParams;

class CTestNetParams : public CMainParams
{
public:
    CTestNetParams()
    {
        networkID    = CBaseChainParams::TESTNET;
        strNetworkID = "test";

        pchMessageStart[0] = 0x18;
        pchMessageStart[1] = 0x15;
        pchMessageStart[2] = 0x14;
        pchMessageStart[3] = 0x88;

        // Start to set test chain consensus data
        // base58Prefixes[EXT_COIN_TYPE]                 = boost::assign::list_of(0x80)(0x00)(0x00)(0x01).convert_to_container<std::vector<unsigned char> >();  // Kore BIP44
        base58Prefixes[EXT_PUBLIC_KEY]                = boost::assign::list_of(0x04)(0x35)(0x87)(0xCF).convert_to_container<std::vector<unsigned char> >(); // Kore BIP32 pubkeys
        base58Prefixes[EXT_SECRET_KEY]                = boost::assign::list_of(0x04)(0x35)(0x83)(0x94).convert_to_container<std::vector<unsigned char> >(); // Kore BIP32 prvkeys
        base58Prefixes[PUBKEY_ADDRESS]                = std::vector<unsigned char>(1, 105);
        base58Prefixes[SCRIPT_ADDRESS]                = std::vector<unsigned char>(1, 190);
        base58Prefixes[SECRET_KEY]                    = std::vector<unsigned char>(1, 233);
        fEnableBigReward                              = true;
        nDefaultPort                                  = 11743;
        nBlocksToBanOldWallets                        = 60;           // Ban old nodes one hour before fork 
        nHeightToFork                                 = 51;
        nLastPOWBlock                                 = 50;
        vAlertPubKey                                  = ParseHex("04cd7ce93858b4257079f4ed9150699bd9f66437ff76617690d1cc180321e94ea391bbccf3bccdcf2edaf0429e32c07b53354e9cecf458cca3fe71dc277f11d9c5");
        strDevFundPubKey                              = "04fb16faf70501f5292a630bced3ec5ff4df277d637e855d129896066854e1d2c9d7cab8dbd5b98107594e74a005e127c66c13a918be477fd3827b872b33d25e03";
        // Deployment of BIP68, BIP112, and BIP113.
        vDeployments[DEPLOYMENT_CSV].bit              = 0;
        vDeployments[DEPLOYMENT_CSV].nStartTime       = 0;
        vDeployments[DEPLOYMENT_CSV].nTimeout         = 0;
        
        // sending rewards to this public key
        CScript genesisOutputScript                   = CScript() << ParseHex("04678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef38c4f35504e51ec112de5c384df7ba0b8d578a4c702b6bf11d5f") << OP_CHECKSIG;
        const char* pszTimestamp                      = "https://bitcoinmagazine.com/articles/altcoins-steal-spotlight-bitcoin-reaches-new-highs/";
        
        genesis = CreateGenesisBlock(NULL, genesisOutputScript, 1557340145, 0, 2500634, 64441706, 0x201fffff, 1, 49 * COIN);
    
        nHashGenesisBlock = genesis.GetHash();
        assert(nHashGenesisBlock == uint256S("0x108f21fa13f48ef6f4edfdbe4d18b09475aa9b5488d9f4ef8a1b02d2c081e7cb"));
        assert(genesis.hashMerkleRoot == uint256S("0x1b889cb87b7ef1b6cef609072307f7714b355d849975955e6ee6d60f9f0c7a2c"));
        
        vFixedSeeds.clear();
        vSeeds.clear();
        vSeeds.push_back(CDNSSeedData("fuzzbawls.pw", "kore-testnet.seed.fuzzbawls.pw"));
        convertSeed6(vFixedSeeds, pnSeed6_test, ARRAYLEN(pnSeed6_test));

        checkpointData = {
            {
                {     0, nHashGenesisBlock}
            }
        };

        chainTxData = ChainTxData{
        // Data from rpc: getchaintxstats
        /* nTime    */ genesis.GetBlockTime(),
        /* nTxCount */ 0,
        /* dTxRate  */ 0
        };
    }
};
static CTestNetParams testNetParams;

/**
 * Regression test
 */
class CRegTestParams : public CTestNetParams
{
public:
    CRegTestParams()
    {
        networkID    = CBaseChainParams::REGTEST;
        strNetworkID = "regtest";

        pchMessageStart[0] = 0xcf;
        pchMessageStart[1] = 0x05;
        pchMessageStart[2] = 0x6a;
        pchMessageStart[3] = 0xe1;

        genesis.nTime                  = 1453993470;
        genesis.nBits                  = 0x207fffff;
        genesis.nNonce                 = 12345;
        
        // Start to set test chain consensus data
        // base58Prefixes[EXT_COIN_TYPE] = boost::assign::list_of(0x80)(0x00)(0x00)(0x01).convert_to_container<std::vector<unsigned char> >();  // Kore BIP44
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x35)(0x87)(0xCF).convert_to_container<std::vector<unsigned char> >(); // Kore BIP32 pubkeys
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x35)(0x83)(0x94).convert_to_container<std::vector<unsigned char> >(); // Kore BIP32 prvkeys
        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1, 105);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1, 190);
        base58Prefixes[SECRET_KEY]     = std::vector<unsigned char>(1, 233);
        bnProofOfWorkLimit             = ~uint256(0) >> 1; // this make easier to find a block !
        fDefaultConsistencyChecks      = true;
        fMineBlocksOnDemand            = true;
        fMiningRequiresPeers           = true;
        fRequireStandard               = false;
        nDefaultPort                   = 18444;
        nHashGenesisBlock              = genesis.GetHash();
        nHeightToFork                  = 900000;           // Height to perform the fork
        nMinerThreads                  = 1;
        nTargetSpacing                 = 1 * 60;           // consensus.nTargetSpacing 1 minutes

        vFixedSeeds.clear(); //! Testnet mode doesn't have any fixed seeds.
        vSeeds.clear();      //! Testnet mode doesn't have any DNS seeds.

        checkpointData = {
            {
                {0, uint256S("0x001")}
            }
        };

        chainTxData = ChainTxData{
        // Data from rpc: getchaintxstats 20160 e5b7d252d6b2ab66702ddd457d90cc13db34b17e01201db056d91736aa505865
        /* nTime    */ 1454124731,
        /* nTxCount */ 0,
        /* dTxRate  */ 100
        };
    }
};
static CRegTestParams regTestParams;


class CUnitTestParams : public CMainParams, public CModifiableParams
{
public:
    CUnitTestParams()
    {
        networkID    = CBaseChainParams::UNITTEST;
        strNetworkID = "unittest";

        // Start to set test chain consensus data
        fDefaultConsistencyChecks                  = true;
        fMineBlocksOnDemand                        = true;
        fMiningRequiresPeers                       = false;
        fSkipProofOfWorkCheck                      = true;
        nDefaultPort                               = 51478;
        nTargetSpacing                             = 10;
        nCoinMaturity                              = nTargetSpacing + 1;
        nPastBlocksMax                             = 128;
        nPastBlocksMin                             = 32;
        nStakeLockInterval                         = 32; // minimum value
        nStakeMinAge                               = 5;

        vFixedSeeds.clear(); //! Unit test mode doesn't have any fixed seeds.
        vSeeds.clear();      //! Unit test mode doesn't have any DNS seeds.
    }

    virtual void setCoinMaturity(int aCoinMaturity) { nCoinMaturity = aCoinMaturity; }
    virtual void setEnableBigRewards(bool afBigRewards) { fEnableBigReward = afBigRewards; };
    virtual void setHeightToFork(int aHeightToFork) { nHeightToFork = aHeightToFork; };
    virtual void setLastPowBlock(int aLastPOWBlock) { nLastPOWBlock = aLastPOWBlock; };
    virtual void setStakeLockInterval(int aStakeLockInterval) { nStakeLockInterval = aStakeLockInterval; };
    virtual void setStakeMinAge(int aStakeMinAge) { nStakeMinAge = aStakeMinAge; };
    // PoS may fail to create new Blocks, if we try to set this to less than 10
    virtual void setTargetSpacing(uint32_t aTargetSpacing) { nTargetSpacing = aTargetSpacing; };
};
static CUnitTestParams unitTestParams;


static CChainParams* pCurrentParams = 0;

CModifiableParams* ModifiableParams()
{
    assert(pCurrentParams);
    assert(pCurrentParams == &unitTestParams);
    return (CModifiableParams*)&unitTestParams;
}

const CChainParams& Params()
{
    assert(pCurrentParams);
    return *pCurrentParams;
}

CChainParams& Params(CBaseChainParams::Network network)
{
    switch (network) {
    case CBaseChainParams::MAIN:
        return mainParams;
    case CBaseChainParams::TESTNET:
        return testNetParams;
    case CBaseChainParams::REGTEST:
        return regTestParams;
    case CBaseChainParams::UNITTEST:
        return unitTestParams;
    default:
        assert(false && "Unimplemented network");
        return mainParams;
    }
}

void SelectParams(CBaseChainParams::Network network)
{
    SelectBaseParams(network);
    pCurrentParams = &Params(network);
}

bool SelectParamsFromCommandLine()
{
    CBaseChainParams::Network network = NetworkIdFromCommandLine();
    if (network == CBaseChainParams::MAX_NETWORK_TYPES)
        return false;

    SelectParams(network);
    return true;
}
