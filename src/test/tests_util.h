#ifndef KORE_TESTS_UTIL_H
#define KORE_TESTS_UTIL_H

#include "script/script.h"
#include "amount.h"

#include <boost/filesystem.hpp>


class CWallet;
class CBlock;
class CBlockIndex;
class CCoinsViewDB;

typedef struct {
    bool fProofOfStake;
    uint32_t nTime;
    uint32_t transactionTime;
    uint32_t nBits;
    unsigned int nonce;
    unsigned int extranonce;
    uint32_t nBirthdayA;
    uint32_t nBirthdayB;
    uint256 hash;
    uint256 hashMerkleRoot;
    CAmount balance;
} blockinfo_t;

extern blockinfo_t blockinfo[];


void InitializeDBTest(const boost::filesystem::path & path);


void FinalizeDBTest(bool shutdown);
void CheckDatabaseState(CWallet* pwalletMain);

void LogBlockFound(CWallet* pwallet, int blockNumber, CBlock* pblock, unsigned int nExtraNonce, bool fProofOfStake, bool logToStdout=false);

/*
  Before Creating a PoS Block it is necessary to call this function to make sure
  we will initialize the static variable: nLastCoinStakeSearchTime, otherwise the first
  block will fail to create
*/
void InitializeLastCoinStakeSearchTime(CWallet* pwallet, CScript& scriptPubKey);


CScript GenerateSamePubKeyScript4Wallet(const string & secret, CWallet* pwallet);

/*
  Method for making sure the correct balance will come
*/
void ScanForWalletTransactions(CWallet* pwallet);

/*
  This function will generate Pow or Pos Blocks for express chain
*/
void GenerateBlocks(int startBlock, int endBlock, CWallet* pwallet, CScript& scriptPubKey, bool fProofOfStake, bool logToStdout=false);

void GeneratePOWLegacyBlocks(int startBlock, int endBlock, CWallet* pwallet, CScript& scriptPubKey, bool logToStdout=false);

void GeneratePOSLegacyBlocks(int startBlock, int endBlock, CWallet* pwallet, CScript& scriptPubKey, bool logToStdout=false);

void Create_Transaction(CBlock* pblock, const CBlockIndex* pindexPrev, const blockinfo_t blockinfo[], int i);

void Create_NewTransaction(CBlock* pblock, const CBlockIndex* pindexPrev, const blockinfo_t blockinfo[], int i);

/*
  This funciton will create Blocks based in a previous block generation found at blockinfo
  note that if you want to recreate the blockInfo, you can uncomment the testcase generate_old_pow
  and get the results from debug.log
*/
void CreateOldBlocksFromBlockInfo(int startBlock, int endBlock, blockinfo_t& blockInfo, CWallet* pwallet, CScript& scriptPubKey, bool fProofOfStake, bool logToStdout=false);

void createNewBlocksFromBlockInfo(int startBlock, int endBlock, blockinfo_t& blockInfo, CWallet* pwallet, CScript& scriptPubKey, bool fProofOfStake, bool logToStdout=false);

#endif