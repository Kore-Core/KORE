// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015-2018 The KORE developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "miner.h"

#include "amount.h"
#include "arith_uint256.h"
#include "hash.h"
#include "legacy/consensus/merkle.h"
#include "main.h"
#include "masternode-sync.h"
#include "net.h"
#include "pos.h"
#include "pow.h"
#include "primitives/block.h"
#include "primitives/transaction.h"
#include "support/csviterator.h"
#include "timedata.h"
#include "util.h"
#include "utilmoneystr.h"
#ifdef ENABLE_WALLET
#include "wallet.h"
#endif
#include "blocksignature.h"
#include "invalid.h"
#include "masternode-payments.h"
#include "spork.h"
#include "validationinterface.h"


#include <boost/thread.hpp>
#include <boost/tuple/tuple.hpp>
#include <fstream>
#include <queue> // Legacy

using namespace std;

//////////////////////////////////////////////////////////////////////////////
//
// KOREMiner
//

//
// Unconfirmed transactions in the memory pool often depend on other
// transactions in the memory pool. When we select transactions from the
// pool, we select by highest priority or fee rate, so we might consider
// transactions that depend on transactions that aren't yet in the block.
// The COrphan class keeps track of these 'temporary orphans' while
// CreateBlock is figuring out which transactions to include.
//
class COrphan
{
public:
    const CTransaction* ptx;
    set<uint256> setDependsOn;
    CFeeRate feeRate;
    double dPriority;

    COrphan(const CTransaction* ptxIn) : ptx(ptxIn), feeRate(0), dPriority(0)
    {
    }
};

uint64_t nLastBlockTx = 0;
uint64_t nLastBlockSize = 0;
int64_t nLastCoinStakeSearchInterval = 0;

// We want to sort transactions by priority and fee rate, so:
typedef boost::tuple<double, CFeeRate, const CTransaction*> TxPriority;
class TxPriorityCompare
{
    bool byFee;

public:
    TxPriorityCompare(bool _byFee) : byFee(_byFee) {}

    bool operator()(const TxPriority& a, const TxPriority& b)
    {
        if (byFee) {
            if (a.get<1>() == b.get<1>())
                return a.get<0>() < b.get<0>();
            return a.get<1>() < b.get<1>();
        } else {
            if (a.get<0>() == b.get<0>())
                return a.get<1>() < b.get<1>();
            return a.get<0>() < b.get<0>();
        }
    }
};

class ScoreCompare //Legacy class
{
public:
    ScoreCompare() {}

    bool operator()(const CTxMemPool::txiter a, const CTxMemPool::txiter b)
    {
        return CompareTxMemPoolEntryByScore()(*b, *a); // Convert to less than
    }
};

CAmount GetBlockReward(CBlockIndex* pindexPrev)
{
    if (pindexPrev->nHeight >= Params().GetLastPoWBlock()) {
        if (pindexPrev->nMoneySupply == MAX_MONEY)
            return 0;

        double moneySupplyFloat = (double)pindexPrev->nMoneySupply / COIN;        
        double rewardDouble = pow(1.44e14 - pow(moneySupplyFloat, 2), (double)1 / 2)/1.436e7;

        CAmount reward = ceil(rewardDouble * COIN);
        if (reward + pindexPrev->nMoneySupply < MAX_MONEY)
            return reward;
        else
            return (MAX_MONEY)-pindexPrev->nMoneySupply;
    } else
        return GetBlockValue(pindexPrev->nHeight);
}

void UpdateTime(CBlockHeader* pblock, const CBlockIndex* pindexPrev, bool fProofOfStake)
{
    int64_t nOldTime = pblock->nTime;
    int64_t nNewTime = std::max(pindexPrev->GetMedianTimePast() + 1, GetAdjustedTime());

    if (nOldTime < nNewTime)
        pblock->nTime = nNewTime;

    // Updating time can change work required:
    pblock->nBits = GetNextTarget(pindexPrev, pblock, fProofOfStake);
}

inline CBlockIndex* GetParentIndex(CBlockIndex* index)
{
    return index->pprev;
}

uint32_t GetNextTarget(const CBlockIndex* pindexLast, const CBlockHeader* pblock, bool fProofOfStake)
{
    // Lico
    if (UseLegacyCode(pindexLast->nHeight))
        return GetNextWorkRequired_Legacy(pindexLast, pblock, fProofOfStake);

    /* current difficulty formula, KoreCorrectionAlgorithm, written by The Kore Developers - 2019 */
    static int64_t nPastBlocksMin = Params().GetPastBlocksMin();
    static int64_t nPastBlocksMax = Params().GetPastBlocksMax();
    static int64_t nTargetSpacing = Params().GetTargetSpacing();
    static int64_t nTargetTimespan = Params().GetTargetTimespan();
    
    const CBlockIndex* BlockLastSolved = pindexLast;
    const CBlockIndex* BlockReading = pindexLast;
    int64_t nActualTimespan = 0;
    int64_t LastBlockTime = 0;

    int64_t CountBlocks = 0;
    uint256 PastDifficultyAverage;
    uint256 PastDifficultyAveragePrev;

    if (BlockLastSolved == NULL || BlockLastSolved->nHeight == 0 || BlockLastSolved->nHeight < nPastBlocksMin) {
        return fProofOfStake ?  Params().ProofOfStakeLimit().GetCompact() : Params().ProofOfWorkLimit().GetCompact();
    }

    if (pindexLast->nHeight > Params().GetLastPoWBlock() || fProofOfStake) {
        uint256 bnTargetLimit = fProofOfStake ? Params().ProofOfStakeLimit() : Params().ProofOfWorkLimit();
        int64_t nMedianTimeSpacing = pindexLast->GetMedianTimeSpacing();
        int64_t nMyBlockSpacing = pblock->GetBlockTime() - pindexLast->GetBlockTime();

        // ppcoin: target change every block
        // ppcoin: retarget with exponential moving toward target spacing
        uint256 bnNew;
        bnNew.SetCompact(pindexLast->nBits);

        int64_t nInterval = (nMyBlockSpacing + nMedianTimeSpacing - 2 * nTargetSpacing) / 2;
        int64_t pastDueSpacing = nInterval > 0 ? nInterval : 0;
        int64_t howManyDue = pastDueSpacing / nTargetSpacing;

        bnNew *= (nMedianTimeSpacing + pow(pastDueSpacing, howManyDue));
        bnNew /= (2 * nTargetSpacing);

        if (bnNew <= 0 || bnNew > bnTargetLimit)
            bnNew = bnTargetLimit;

        if (fDebug){
            static uint256 oldTarget = 0;
            if (bnNew != oldTarget) {
                LogPrintf("%s(): %s \n", __func__, bnNew.ToString().c_str());
                oldTarget = bnNew;
            }
        }
        
        return bnNew <= Params().ProofOfStakeLimit() ? bnNew.GetCompact() : Params().ProofOfStakeLimit().GetCompact();
    }

    for (unsigned int i = 1; BlockReading && BlockReading->nHeight > 0; i++) {
        if (nPastBlocksMax > 0 && i > nPastBlocksMax) {
            break;
        }
        CountBlocks++;

        if (CountBlocks <= nPastBlocksMin) {
            if (CountBlocks == 1) {
                PastDifficultyAverage.SetCompact(BlockReading->nBits);
            } else {
                PastDifficultyAverage = ((PastDifficultyAveragePrev * CountBlocks) + (uint256().SetCompact(BlockReading->nBits))) / (CountBlocks + 1);
            }
            PastDifficultyAveragePrev = PastDifficultyAverage;
        }

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

    int64_t Diff = (pblock->nTime - BlockLastSolved->GetBlockTime());
    nActualTimespan += Diff;

    uint256 bnNew(PastDifficultyAverage);

    int64_t _nTargetTimespan = CountBlocks * Params().GetTargetSpacing();
    if (fDebug) {
        LogPrintf("nActualTimespan: %d \n", nActualTimespan);
        LogPrintf("PastDifficultyAverage: %s \n", PastDifficultyAverage.ToString().c_str());
        LogPrintf("_nTargetTimespan : %d \n", _nTargetTimespan);
    }

    if (nActualTimespan < _nTargetTimespan / 3)
        nActualTimespan = _nTargetTimespan / 3;
    if (nActualTimespan > _nTargetTimespan * 3)
        nActualTimespan = _nTargetTimespan * 3;


    // Retarget
    bnNew *= nActualTimespan;
    bnNew /= _nTargetTimespan;

    if (bnNew > Params().ProofOfWorkLimit()) {
        bnNew = Params().ProofOfWorkLimit();
    }

    if (fDebug) {
        LogPrintf("nActualTimespan: %d \n", nActualTimespan);
        LogPrintf("GetNextTarget: %s \n", bnNew.ToString().c_str());
    }

    return bnNew.GetCompact();
}


inline CMutableTransaction CreateCoinbaseTransaction(const CScript& scriptPubKeyIn)
{
    // Create coinbase tx
    CMutableTransaction txNew;
    txNew.vin.resize(1);
    txNew.vin[0].prevout.SetNull();
    txNew.vout.resize(1);
    txNew.vout[0].scriptPubKey = scriptPubKeyIn;

    return txNew;
}

std::pair<int, std::pair<uint256, uint256> > pCheckpointCache;
CBlockTemplate* CreateNewBlock(const CScript& scriptPubKeyIn, CWallet* pwallet, bool fProofOfStake)
{
    CReserveKey reservekey(pwallet);

    // Create new block
    unique_ptr<CBlockTemplate> pblocktemplate(new CBlockTemplate(CBlockHeader::POS_FORK_VERSION));
    if (!pblocktemplate.get())
        return NULL;
    CBlock* pblock = &pblocktemplate->block; // pointer for convenience
    // Set if block as proof of stake or not
    pblock->fIsProofOfStake = fProofOfStake;

    // -regtest only: allow overriding block.nVersion with
    // -blockversion=N to test forking scenarios
    if (Params().ShouldMineBlocksOnDemand())
        pblock->nVersion = GetArg("-blockversion", pblock->nVersion);

    CMutableTransaction txNew;

    txNew = CreateCoinbaseTransaction(scriptPubKeyIn);
    pblock->vtx.push_back(txNew);

    pblocktemplate->vTxFees.push_back(-1);   // updated at end
    pblocktemplate->vTxSigOps.push_back(-1); // updated at end


    // ppcoin: if coinstake available add coinstake tx
    if (fProofOfStake) {
        boost::this_thread::interruption_point();
        static int64_t nLastCoinStakeSearchTime = GetAdjustedTime(); // only initialized at startup

        bool fStakeFound = false;
        unsigned int nTxNewTime = 0;
        CKey key;
        CMutableTransaction txCoinbase;
        CMutableTransaction txCoinStake;

        pblock->nTime = GetAdjustedTime();
        CBlockIndex* pindexPrev = chainActive.Tip();
        pblock->nBits = GetNextTarget(pindexPrev, pblock, fProofOfStake);
        int64_t nSearchTime = pblock->nTime; // search to current time

        if (nSearchTime >= nLastCoinStakeSearchTime) {
            if (pwallet->CreateCoinStake(*pwallet, pblock->nBits, nSearchTime - nLastCoinStakeSearchTime, txCoinbase, txCoinStake, nTxNewTime, fProofOfStake, key)) {
                pblock->nTime = nTxNewTime;
                pblock->vtx[0] = CTransaction(txCoinbase);
                pblock->vtx.push_back(CTransaction(txCoinStake));
                fStakeFound = true;
            }
            nLastCoinStakeSearchInterval = nSearchTime - nLastCoinStakeSearchTime;
            nLastCoinStakeSearchTime = nSearchTime;
        }
        if (fDebug && fStakeFound)
            LogPrintf("%s(): stake found\n", __func__);
        
        if (!fStakeFound)
            return NULL;
    }

    // Largest block you're willing to create:
    unsigned int nBlockMaxSize = GetArg("-blockmaxsize", DEFAULT_BLOCK_MAX_SIZE);
    // Limit to betweeen 1K and MAX_BLOCK_SIZE-1K for sanity:
    unsigned int nBlockMaxSizeNetwork = MAX_BLOCK_SIZE;
    nBlockMaxSize = std::max((unsigned int)1000, std::min((nBlockMaxSizeNetwork - 1000), nBlockMaxSize));

    // How much of the block should be dedicated to high-priority transactions,
    // included regardless of the fees they pay
    unsigned int nBlockPrioritySize = GetArg("-blockprioritysize", DEFAULT_BLOCK_PRIORITY_SIZE);
    nBlockPrioritySize = std::min(nBlockMaxSize, nBlockPrioritySize);

    // Minimum block size you want to create; block will be filled with free transactions
    // until there are no more or the block reaches this size:
    unsigned int nBlockMinSize = GetArg("-blockminsize", DEFAULT_BLOCK_MIN_SIZE);
    nBlockMinSize = std::min(nBlockMaxSize, nBlockMinSize);

    // Collect memory pool transactions into the block
    CAmount nFees = 0;

    {
        LOCK2(cs_main, mempool.cs);

        CBlockIndex* pindexPrev = chainActive.Tip();
        const int nHeight = pindexPrev->nHeight + 1;
        if (!fProofOfStake)
            pblock->nTime = GetAdjustedTime();
        // we will be creating blocks from after fork here always
        pblock->nVersion = CBlockHeader::POS_FORK_VERSION;
        CCoinsViewCache view(pcoinsTip);

        // Priority order to process transactions
        list<COrphan> vOrphan; // list memory doesn't move
        map<uint256, vector<COrphan*> > mapDependers;
        bool fPrintPriority = GetBoolArg("-printpriority", false);

        // This vector will be sorted into a priority queue:
        vector<TxPriority> vecPriority;
        vecPriority.reserve(mempool.mapTx.size());
        for (CTxMemPool::indexed_transaction_set::iterator mi = mempool.mapTx.begin();
             mi != mempool.mapTx.end(); ++mi) {
            const CTransaction& tx = mi->GetTx();
            if (tx.IsCoinBase() || tx.IsCoinStake() || !IsFinalTx(tx, nHeight)) {
                continue;
            }

            COrphan* porphan = NULL;
            double dPriority = 0;
            CAmount nTotalIn = 0;
            bool fMissingInputs = false;
            uint256 txid = tx.GetHash();
            for (const CTxIn& txin : tx.vin) {
                // Read prev transaction
                if (!view.HaveCoins(txin.prevout.hash)) {
                    // This should never happen; all transactions in the memory
                    // pool should connect to either transactions in the chain
                    // or other transactions in the memory pool.
                    if (!mempool.mapTx.count(txin.prevout.hash)) {
                        fMissingInputs = true;
                        if (porphan)
                            vOrphan.pop_back();
                        break;
                    }

                    // Has to wait for dependencies
                    if (!porphan) {
                        // Use list for automatic deletion
                        vOrphan.push_back(COrphan(&tx));
                        porphan = &vOrphan.back();
                    }
                    mapDependers[txin.prevout.hash].push_back(porphan);
                    porphan->setDependsOn.insert(txin.prevout.hash);
                    nTotalIn += mempool.mapTx.find(txin.prevout.hash)->GetTx().vout[txin.prevout.n].nValue;
                    continue;
                }

                //Check for invalid/fraudulent inputs. They shouldn't make it through mempool, but check anyways.
                if (invalid_out::ContainsOutPoint(txin.prevout)) {
                    if (fDebug)
                        LogPrintf("%s : found invalid input %s in tx %s", __func__, txin.prevout.ToString(), tx.GetHash().ToString());
                    
                    fMissingInputs = true;
                    break;
                }

                const CCoins* coins = view.AccessCoins(txin.prevout.hash);
                assert(coins);

                CAmount nValueIn = coins->vout[txin.prevout.n].nValue;
                nTotalIn += nValueIn;

                int nConf = nHeight - coins->nHeight;

                dPriority = double_safe_addition(dPriority, ((double)nValueIn * nConf));
            }
            if (fMissingInputs) continue;

            // Priority is sum(valuein * age) / modified_txsize
            unsigned int nTxSize = ::GetSerializeSize(tx, SER_NETWORK, PROTOCOL_VERSION);
            dPriority = tx.ComputePriority(dPriority, nTxSize);

            uint256 hash = tx.GetHash();
            mempool.ApplyDeltas(hash, dPriority, nTotalIn);

            CFeeRate feeRate(nTotalIn - tx.GetValueOut(), nTxSize);

            if (porphan) {
                porphan->dPriority = dPriority;
                porphan->feeRate = feeRate;
            } else
                vecPriority.push_back(TxPriority(dPriority, feeRate, &mi->GetTx()));
        }

        // Collect transactions into block
        uint64_t nBlockSize = 1000;
        uint64_t nBlockTx = 0;
        int nBlockSigOps = 100;
        bool fSortedByFee = (nBlockPrioritySize <= 0);

        TxPriorityCompare comparer(fSortedByFee);
        std::make_heap(vecPriority.begin(), vecPriority.end(), comparer);

        while (!vecPriority.empty()) {
            // Take highest priority transaction off the priority queue:
            double dPriority = vecPriority.front().get<0>();
            CFeeRate feeRate = vecPriority.front().get<1>();
            const CTransaction& tx = *(vecPriority.front().get<2>());

            std::pop_heap(vecPriority.begin(), vecPriority.end(), comparer);
            vecPriority.pop_back();

            // Size limits
            unsigned int nTxSize = ::GetSerializeSize(tx, SER_NETWORK, PROTOCOL_VERSION);
            if (nBlockSize + nTxSize >= nBlockMaxSize)
                continue;

            // Legacy limits on sigOps:
            unsigned int nMaxBlockSigOps = MAX_BLOCK_SIGOPS_CURRENT;
            unsigned int nTxSigOps = GetLegacySigOpCount(tx);
            if (nBlockSigOps + nTxSigOps >= nMaxBlockSigOps)
                continue;

            // Skip free transactions if we're past the minimum block size:
            const uint256& hash = tx.GetHash();
            double dPriorityDelta = 0;
            CAmount nFeeDelta = 0;
            mempool.ApplyDeltas(hash, dPriorityDelta, nFeeDelta);
            if (fSortedByFee && (dPriorityDelta <= 0) && (nFeeDelta <= 0) && (feeRate < ::minRelayTxFee) && (nBlockSize + nTxSize >= nBlockMinSize))
                continue;

            // Prioritise by fee once past the priority size or we run out of high-priority
            // transactions:
            if (!fSortedByFee && ((nBlockSize + nTxSize >= nBlockPrioritySize) || !AllowFree(dPriority))) {
                fSortedByFee = true;
                comparer = TxPriorityCompare(fSortedByFee);
                std::make_heap(vecPriority.begin(), vecPriority.end(), comparer);
            }

            if (!view.HaveInputs(tx))
                continue;

            CAmount nTxFees = view.GetValueIn(tx) - tx.GetValueOut();

            nTxSigOps += GetP2SHSigOpCount(tx, view);
            if (nBlockSigOps + nTxSigOps >= nMaxBlockSigOps)
                continue;

            // Note that flags: we don't want to set mempool/IsStandard()
            // policy here, but we still have to ensure that the block we
            // create only contains transactions that are valid in new blocks.
            CValidationState state;
            if (!CheckInputs(tx, state, view, true, MANDATORY_SCRIPT_VERIFY_FLAGS, true))
                continue;

            CTxUndo txundo;
            UpdateCoins(tx, state, view, txundo, nHeight);

            // Added
            pblock->vtx.push_back(tx);
            pblocktemplate->vTxFees.push_back(nTxFees);
            pblocktemplate->vTxSigOps.push_back(nTxSigOps);
            nBlockSize += nTxSize;
            ++nBlockTx;
            nBlockSigOps += nTxSigOps;
            nFees += nTxFees;

            if (fPrintPriority && fDebug) 
                LogPrintf("priority %.1f fee %s txid %s\n", dPriority, feeRate.ToString(), tx.GetHash().ToString());
            
            // Add transactions that depend on this one to the priority queue
            if (mapDependers.count(hash)) {
                BOOST_FOREACH (COrphan* porphan, mapDependers[hash]) {
                    if (!porphan->setDependsOn.empty()) {
                        porphan->setDependsOn.erase(hash);
                        if (porphan->setDependsOn.empty()) {
                            vecPriority.push_back(TxPriority(porphan->dPriority, porphan->feeRate, porphan->ptx));
                            std::push_heap(vecPriority.begin(), vecPriority.end(), comparer);
                        }
                    }
                }
            }
        }

        if (!fProofOfStake) {
            //Masternode and general budget payments
            FillBlockPayee(txNew, nFees, fProofOfStake, false);

            //Make payee
            if (txNew.vout.size() > 1) {
                pblock->payee = txNew.vout[1].scriptPubKey;
            }
        }

        nLastBlockTx = nBlockTx;
        nLastBlockSize = nBlockSize;
        // Compute final coinbase transaction.
        pblocktemplate->vTxFees[0] = -nFees;

        if (!fProofOfStake) {
            pblock->vtx[0] = txNew;
            pblock->vtx[0].vin[0].scriptSig = CScript() << nHeight << OP_0;
            pblock->vtx[0].vout[0].nValue += nFees;
        }
        else
            pblock->vtx[0].vout[1].nValue += nFees;

        // Fill in header
        pblock->hashPrevBlock = pindexPrev->GetBlockHash();
        if (!fProofOfStake) {
            UpdateTime(pblock, pindexPrev, fProofOfStake);
            pblock->nBits = GetNextTarget(pindexPrev, pblock, fProofOfStake);
        }
        pblock->nNonce = 0;

        pblocktemplate->vTxSigOps[0] = GetLegacySigOpCount(pblock->vtx[0]);

        CValidationState state;
        if (!TestBlockValidity(state, *pblock, pindexPrev, false, false)) {
            mempool.clear();
            return NULL;
        }
        if (fDebug)
            LogPrintf("CreateNewBlock() : Block is VALID !!! \n");
    }

    return pblocktemplate.release();
}

inline CMutableTransaction CreateCoinbaseTransaction_Legacy(const CScript& scriptPubKeyIn, CAmount nFees, const int nHeight, bool fProofOfStake)
{
    // Create and Compute final coinbase transaction.
    CAmount reward = nFees + GetBlockValue(chainActive.Tip()->nHeight + 1);
    CAmount devsubsidy = reward * 0.1;

    CMutableTransaction txNew;
    txNew.vin.resize(1);
    txNew.vin[0].prevout.SetNull();
    txNew.vin[0].scriptSig = CScript() << nHeight << OP_0;
    txNew.nTime = GetAdjustedTime();
    txNew.SetVersion(1);    

    if (fProofOfStake) {
        txNew.vout.resize(1);
        txNew.vout[0].SetEmpty();

        return txNew;
    } else {
        static string message = "Created on version 13 pre-fork";
        static vector<u_char> vecMessage(message.begin(), message.end());

        txNew.vout.resize(3);
        txNew.vout[0].nValue = reward - devsubsidy;
        txNew.vout[0].scriptPubKey = scriptPubKeyIn;
        txNew.vout[1].nValue = devsubsidy;
        txNew.vout[1].scriptPubKey = CScript() << ParseHex(Params().GetDevFundPubKey().c_str()) << OP_CHECKSIG;
        txNew.vout[2].SetEmpty();
        txNew.vout[2].scriptPubKey = CScript() << vecMessage << OP_RETURN;
    }

    //Masternode and general budget payments
    if (!fProofOfStake)
        FillBlockPayee_Legacy(txNew, 0, fProofOfStake);

    return txNew;
}

CBlockTemplate* CreateNewBlock_Legacy(const CChainParams& chainparams, const CScript& scriptPubKeyIn, CWallet* pwallet, bool fProofOfStake)
{
    // Create new block
    unique_ptr<CBlockTemplate> pblocktemplate(new CBlockTemplate(CBlockHeader::CURRENT_VERSION));
    if (!pblocktemplate.get())
        return NULL;
    CBlock* pblock = &pblocktemplate->block; // pointer for convenience

    // Add dummy coinbase tx as first transaction
    pblock->vtx.push_back(CTransaction());
    pblocktemplate->vTxFees.push_back(-1);   // updated at end
    pblocktemplate->vTxSigOps.push_back(-1); // updated at end

    // Largest block you're willing to create:
    unsigned int nBlockMaxSize = GetArg("-blockmaxsize", DEFAULT_BLOCK_MAX_SIZE);
    // Limit to between 1K and MAX_BLOCK_SIZE-1K for sanity:
    nBlockMaxSize = std::max((unsigned int)1000, std::min((unsigned int)(MAX_BLOCK_SIZE_LEGACY - 1000), nBlockMaxSize));

    // How much of the block should be dedicated to high-priority transactions,
    // included regardless of the fees they pay
    unsigned int nBlockPrioritySize = GetArg("-blockprioritysize", DEFAULT_BLOCK_PRIORITY_SIZE);
    nBlockPrioritySize = std::min(nBlockMaxSize, nBlockPrioritySize);

    // Minimum block size you want to create; block will be filled with free transactions
    // until there are no more or the block reaches this size:
    unsigned int nBlockMinSize = GetArg("-blockminsize", DEFAULT_BLOCK_MIN_SIZE);
    nBlockMinSize = std::min(nBlockMaxSize, nBlockMinSize);

    // Collect memory pool transactions into the block
    CTxMemPool::setEntries inBlock;
    CTxMemPool::setEntries waitSet;

    // This vector will be sorted into a priority queue:
    vector<TxCoinAgePriority> vecPriority;
    TxCoinAgePriorityCompare pricomparer;
    std::map<CTxMemPool::txiter, double, CTxMemPool::CompareIteratorByHash> waitPriMap;
    typedef std::map<CTxMemPool::txiter, double, CTxMemPool::CompareIteratorByHash>::iterator waitPriIter;
    double actualPriority = -1;

    std::priority_queue<CTxMemPool::txiter, std::vector<CTxMemPool::txiter>, ScoreCompare> clearedTxs;
    bool fPrintPriority = GetBoolArg("-printpriority", DEFAULT_PRINTPRIORITY_LEGACY);
    uint64_t nBlockSize = 1000;
    uint64_t nBlockTx = 0;
    unsigned int nBlockSigOps = 100;
    int lastFewTxs = 0;
    CAmount nFees = 0;

    {
        LOCK2(cs_main, mempool.cs);
        CBlockIndex* pindexPrev = chainActive.Tip();
        const int nHeight = pindexPrev->nHeight + 1;
        pblock->nTime = GetAdjustedTime();
        const int64_t nMedianTimePast = pindexPrev->GetMedianTimePast();

        // Setting the first bit, fork preparation and setting the version as 1
        pblock->nVersion = CBlockHeader::CURRENT_VERSION; //ComputeBlockVersion(pindexPrev, chainparams.GetConsensus());
        // -regtest only: allow overriding block.nVersion with
        // -blockversion=N to test forking scenarios
        if (chainparams.ShouldMineBlocksOnDemand())
            pblock->nVersion = GetArg("-blockversion", pblock->nVersion);

        int64_t nLockTimeCutoff = (STANDARD_LOCKTIME_VERIFY_FLAGS & LOCKTIME_MEDIAN_TIME_PAST) ? nMedianTimePast : pblock->GetBlockTime();

        bool fPriorityBlock = nBlockPrioritySize > 0;
        if (fPriorityBlock) {
            vecPriority.reserve(mempool.mapTx.size());
            for (CTxMemPool::indexed_transaction_set::iterator mi = mempool.mapTx.begin();
                 mi != mempool.mapTx.end(); ++mi) {
                double dPriority = mi->GetPriority(nHeight);
                CAmount dummy;
                mempool.ApplyDeltas(mi->GetTx().GetHash(), dPriority, dummy);
                vecPriority.push_back(TxCoinAgePriority(dPriority, mi));
            }
            std::make_heap(vecPriority.begin(), vecPriority.end(), pricomparer);
        }

        CTxMemPool::indexed_transaction_set::nth_index<3>::type::iterator mi = mempool.mapTx.get<3>().begin();
        CTxMemPool::txiter iter;

        while (mi != mempool.mapTx.get<3>().end() || !clearedTxs.empty()) {
            bool priorityTx = false;
            if (fPriorityBlock && !vecPriority.empty()) { // add a tx from priority queue to fill the blockprioritysize
                priorityTx = true;
                iter = vecPriority.front().second;
                actualPriority = vecPriority.front().first;
                std::pop_heap(vecPriority.begin(), vecPriority.end(), pricomparer);
                vecPriority.pop_back();
            } else if (clearedTxs.empty()) { // add tx with next highest score
                iter = mempool.mapTx.project<0>(mi);
                mi++;
            } else { // try to add a previously postponed child tx
                iter = clearedTxs.top();
                clearedTxs.pop();
            }

            if (inBlock.count(iter))
                continue; // could have been added to the priorityBlock

            const CTransaction& tx = iter->GetTx();

            bool fOrphan = false;
            BOOST_FOREACH (CTxMemPool::txiter parent, mempool.GetMemPoolParents(iter)) {
                if (!inBlock.count(parent)) {
                    fOrphan = true;
                    break;
                }
            }
            if (fOrphan) {
                if (priorityTx)
                    waitPriMap.insert(std::make_pair(iter, actualPriority));
                else
                    waitSet.insert(iter);
                continue;
            }

            unsigned int nTxSize = iter->GetTxSize();
            if (fPriorityBlock &&
                (nBlockSize + nTxSize >= nBlockPrioritySize || !AllowFree(actualPriority))) {
                fPriorityBlock = false;
                waitPriMap.clear();
            }
            if (!priorityTx &&
                (iter->GetModifiedFee() < ::minRelayTxFee.GetFee(nTxSize) && nBlockSize >= nBlockMinSize)) {
                break;
            }
            if (nBlockSize + nTxSize >= nBlockMaxSize) {
                if (nBlockSize > nBlockMaxSize - 100 || lastFewTxs > 50) {
                    break;
                }
                // Once we're within 1000 bytes of a full block, only look at 50 more txs
                // to try to fill the remaining space.
                if (nBlockSize > nBlockMaxSize - 1000) {
                    lastFewTxs++;
                }
                continue;
            }

            if (tx.IsCoinStake() || !IsFinalTx(tx, nHeight, nLockTimeCutoff) || pblock->GetBlockTime() < (int64_t)tx.nTime)
                continue;

            unsigned int nTxSigOps = iter->GetSigOpCount();
            if (nBlockSigOps + nTxSigOps >= MAX_BLOCK_SIGOPS_LEGACY) {
                if (nBlockSigOps > MAX_BLOCK_SIGOPS_LEGACY - 2) {
                    break;
                }
                continue;
            }

            CAmount nTxFees = iter->GetFee();
            // Added
            pblock->vtx.push_back(tx);
            pblocktemplate->vTxFees.push_back(nTxFees);
            pblocktemplate->vTxSigOps.push_back(nTxSigOps);
            nBlockSize += nTxSize;
            ++nBlockTx;
            nBlockSigOps += nTxSigOps;
            nFees += nTxFees;

            if (fPrintPriority) {
                double dPriority = iter->GetPriority(nHeight);
                CAmount dummy;
                mempool.ApplyDeltas(tx.GetHash(), dPriority, dummy);
                if (fDebug)
                    LogPrintf("priority %.1f fee %s txid %s\n", dPriority, CFeeRate(iter->GetModifiedFee(), nTxSize).ToString(), tx.GetHash().ToString());
            }

            inBlock.insert(iter);

            // Add transactions that depend on this one to the priority queue
            BOOST_FOREACH (CTxMemPool::txiter child, mempool.GetMemPoolChildren(iter)) {
                if (fPriorityBlock) {
                    waitPriIter wpiter = waitPriMap.find(child);
                    if (wpiter != waitPriMap.end()) {
                        vecPriority.push_back(TxCoinAgePriority(wpiter->second, child));
                        std::push_heap(vecPriority.begin(), vecPriority.end(), pricomparer);
                        waitPriMap.erase(wpiter);
                    }
                } else {
                    if (waitSet.count(child)) {
                        clearedTxs.push(child);
                        waitSet.erase(child);
                    }
                }
            }
        }
        nLastBlockTx = nBlockTx;
        nLastBlockSize = nBlockSize;

        if (fDebug)
            LogPrintf("CreateNewBlock_Legacy(): total size %u txs: %u fees: %ld sigops %d\n", nBlockSize, nBlockTx, nFees, nBlockSigOps);

        pblock->vtx[0] = CreateCoinbaseTransaction_Legacy(scriptPubKeyIn, nFees, nHeight, fProofOfStake);

        //Make payee
        if (!fProofOfStake && pblock->vtx[0].vout.size() > 1) {
            int size = pblock->vtx[0].vout.size();
            pblock->payee = pblock->vtx[0].vout[size - 1].scriptPubKey;
        }

        pblocktemplate->vTxFees[0] = -nFees;

        // Fill in header
        pblock->hashPrevBlock = pindexPrev->GetBlockHash();
        if (!fProofOfStake)
            UpdateTime(pblock, pindexPrev, fProofOfStake);

        pblock->nBits = GetNextWorkRequired_Legacy(pindexPrev, pblock, fProofOfStake);
        pblock->nNonce = 0;

        pblocktemplate->vTxSigOps[0] = GetLegacySigOpCount(pblock->vtx[0]);

        CValidationState state;
        if (!fProofOfStake && !TestBlockValidity_Legacy(state, chainparams, *pblock, pindexPrev, false, false)) {
            throw std::runtime_error(strprintf("%s: TestBlockValidity failed: %s", __func__, FormatStateMessage_Legacy(state)));
        }
    }
    return pblocktemplate.release();
}


void IncrementExtraNonce(CBlock* pblock, CBlockIndex* pindexPrev, unsigned int& nExtraNonce)
{
    // Update nExtraNonce
    static uint256 hashPrevBlock;
    if (hashPrevBlock != pblock->hashPrevBlock) {
        nExtraNonce = 0;
        hashPrevBlock = pblock->hashPrevBlock;
    }
    ++nExtraNonce;
    unsigned int nHeight = pindexPrev->nHeight + 1; // Height first in coinbase required for block.version=2
    CMutableTransaction txCoinbase(pblock->vtx[0]);
    txCoinbase.vin[0].scriptSig = (CScript() << nHeight << CScriptNum(nExtraNonce)) + COINBASE_FLAGS;
    assert(txCoinbase.vin[0].scriptSig.size() <= 100);

    pblock->vtx[0] = txCoinbase;
    pblock->hashMerkleRoot = pblock->BuildMerkleTree();
}

void IncrementExtraNonce_Legacy(CBlock* pblock, const CBlockIndex* pindexPrev, unsigned int& nExtraNonce)
{
    // Update nExtraNonce
    static uint256 hashPrevBlock;
    if (hashPrevBlock != pblock->hashPrevBlock) {
        nExtraNonce = 0;
        hashPrevBlock = pblock->hashPrevBlock;
    }
    ++nExtraNonce;
    unsigned int nHeight = pindexPrev->nHeight + 1; // Height first in coinbase required for block.version=2
    CMutableTransaction txCoinbase(pblock->vtx[0]);
    txCoinbase.vin[0].scriptSig = (CScript() << nHeight << CScriptNum(nExtraNonce)) + COINBASE_FLAGS;
    assert(txCoinbase.vin[0].scriptSig.size() <= 100);

    pblock->vtx[0] = txCoinbase;
    pblock->hashMerkleRoot = BlockMerkleRoot(*pblock);
}

#ifdef ENABLE_WALLET
//////////////////////////////////////////////////////////////////////////////
//
// Internal miner
//
double dHashesPerMin = 0.0;
int64_t nHPSTimerStart = 0;

bool ProcessBlockFound(CBlock* pblock, CWallet& wallet, CReserveKey& reservekey)
{
    // LogPrintf("%s\n", pblock->ToString());
    // LogPrintf("generated %s\n", FormatMoney(pblock->vtx[0].vout[0].nValue));

    // Found a solution
    {
        LOCK(cs_main);
        if (pblock->hashPrevBlock != chainActive.Tip()->GetBlockHash())
            return error("KOREMiner : generated block is stale");
    }

    // Remove key from key pool
    reservekey.KeepKey();

    // Track how many getdata requests this block gets
    {
        LOCK(wallet.cs_wallet);
        wallet.mapRequestCount[pblock->GetHash()] = 0;
    }

    // Inform about the new block
    GetMainSignals().BlockFound(pblock->GetHash());

    // Process this block the same as if we had receiveMinerd it from another node
    CValidationState state;
    if (!ProcessNewBlock(state, NULL, pblock)) {
        return error("KOREMiner : ProcessNewBlock, block not accepted");
    }

    for (CNode* node : vNodes) {
        node->PushInventory(CInv(MSG_BLOCK, pblock->GetHash()));
    }

    return true;
}

bool ProcessBlockFound_Legacy(const CBlock* pblock, const CChainParams& chainparams)
{
    CValidationState state;

    // Found a solution
    {
        LOCK(cs_main);
        if (pblock->hashPrevBlock != chainActive.Tip()->GetBlockHash())
            return error("ProcessBlockFound(): generated/staked block is stale");
    }

    if (fDebug)
        LogPrintf("%s \n ", pblock->ToString());

    // verify hash target and signature of coinstake tx
    if (pblock->IsProofOfStake() && !CheckProofOfStake_Legacy(mapBlockIndex[pblock->hashPrevBlock], pblock->vtx[1], pblock->nBits, state))
        return false;

    if (fDebug)
        LogPrintf("%s %s\n", pblock->IsProofOfStake() ? "Stake " : "Mined ", pblock->IsProofOfStake() ? FormatMoney(pblock->vtx[1].GetValueOut()) : FormatMoney(pblock->vtx[0].GetValueOut()));

    // Inform about the new block
    GetMainSignals().BlockFound(pblock->GetHash());

    // Process this block the same as if we had received it from another node
    if (!ProcessNewBlock_Legacy(state, chainparams, NULL, pblock, true, NULL))
        return error("KoreMiner: ProcessNewBlock, block not accepted");


    return true;
}

#include <iostream>

// attempt to generate suitable proof-of-stake
bool SignBlock_Legacy(CWallet* pwallet, CBlock* pblock)
{
    // if we are trying to sign
    //    something except proof-of-stake block template
    if (!pblock->vtx[0].vout[0].IsEmpty()) {
        if (fDebug)
            LogPrintf("something except proof-of-stake block\n");
        return false;
    }

    // if we are trying to sign
    //    a complete proof-of-stake block
    if (pblock->IsProofOfStake()) {
        if (fDebug)
            LogPrintf("trying to sign a complete proof-of-stake block\n");
        
        return true;
    }

    static int64_t nLastCoinStakeSearchTime = GetAdjustedTime(); // startup timestamp

    CKey key;
    CMutableTransaction txCoinStake;
    txCoinStake.nTime = GetAdjustedTime();
    txCoinStake.nTime &= ~15;
    CAmount nFees = 0;

    int64_t nSearchTime = txCoinStake.nTime; // search to current time

    //cout << "SearchTime               = " << nSearchTime << endl;
    //cout << "nLastCoinStakeSearchTime = " << nLastCoinStakeSearchTime << endl;

    if (nSearchTime >= nLastCoinStakeSearchTime) {
        int64_t nSearchInterval = 1;
        if (pwallet->CreateCoinStake_Legacy(*pwallet, pblock, nSearchInterval, nFees, txCoinStake, key)) {
            //if (txCoinStake.nTime >= pindexBestHeader->GetMedianTimePast()+1)
            //{
            // make sure coinstake would meet timestamp protocol
            //    as it would be the same as the block timestamp
            //pblock->nTime = txCoinStake.nTime = pblock->vtx[0].nTime;

            // we have to make sure that we have no future timestamps in
            //    our transactions set
            for (vector<CTransaction>::iterator it = pblock->vtx.begin(); it != pblock->vtx.end();)
                if (it->nTime > pblock->nTime) {
                    it = pblock->vtx.erase(it);
                } else {
                    ++it;
                }

            pblock->vtx.insert(pblock->vtx.begin() + 1, txCoinStake);
            pblock->hashMerkleRoot = BlockMerkleRoot(*pblock);

            // append a signature to our block
            return key.Sign(pblock->GetHash(), pblock->vchBlockSig);
            //}
        }
        nLastCoinStakeSearchInterval = nSearchTime - nLastCoinStakeSearchTime;
        nLastCoinStakeSearchTime = nSearchTime;
    }
    return false;
}

bool fGenerateBitcoins = false;
bool fMintableCoins = false;
int nMintableLastCheck = 0;

// ***TODO*** that part changed in bitcoin, we are using a mix with old one here for now

void BitcoinMiner(CWallet* pwallet, bool fProofOfStake)
{
    LogPrintf("KOREMiner started, fProofOfStake:%s \n", fProofOfStake ? "true" : "false");
    SetThreadPriority(THREAD_PRIORITY_LOWEST);
    RenameThread("kore-miner");

    // Each thread has its own key and counter
    CReserveKey reservekey(pwallet);
    unsigned int nExtraNonce = 0;

    while (!ShutdownRequested() && UseLegacyCode(GetnHeight(chainActive.Tip()) + 1)) {
        // while nobody requested to shutdown and we should use the legacy code
        // this thread should wait
        if (fDebug) {
            LogPrintf("This thread is waiting for the Fork to happen.\n");
            LogPrintf("Current nHeight: %d \n", GetnHeight(chainActive.Tip()));
            LogPrintf("Height to Fork : %d \n", Params().HeightToFork());
        }
        // check every 5 seconds
        MilliSleep(5000);
    }
    if (fDebug)
        LogPrintf("We are Free to create Block: %s \n", GetnHeight(chainActive.Tip()) + 1);
    
    while (!ShutdownRequested() && (fGenerateBitcoins || fProofOfStake)) {
        boost::this_thread::interruption_point();
        if (fProofOfStake) {
            //control the amount of times the client will check for mintable coins
            if ((GetTime() - nMintableLastCheck > Params().GetTargetSpacing())) {
                nMintableLastCheck = GetTime();
                fMintableCoins = pwallet->MintableCoins();
            }

            while (vNodes.empty() || pwallet->IsLocked() || !fMintableCoins || pwallet->GetBalance() == 0 || nReserveBalance > pwallet->GetBalance()
              || !(masternodeSync.IsSynced() && mnodeman.CountEnabled() >= 2))
            {
                if (fDebug) {
                    LogPrintf("%s(): still unable to stake.\n", __func__);
                    if (vNodes.empty())
                        LogPrintf("\tThere are no nodes connected;\n");
                    if (pwallet->IsLocked())
                        LogPrintf("\tThe wallet is locked;\n");
                    if (fMintableCoins)
                        LogPrintf("\tThere are no mintable coins;\n");
                    if (masternodeSync.IsSynced())
                        LogPrintf("\tMasternodes are not synced;\n");
                    if (mnodeman.CountEnabled() < 2)
                        LogPrintf("\tThere are not enough masternodes enabled;\n");
                    if (pwallet->GetBalance() == 0)
                        LogPrintf("\tZero balance;\n");
                    if (nReserveBalance >= pwallet->GetBalance())
                        LogPrintf("\tThe wallet balance is lower than the reserved balance;\n");
                }

                MilliSleep(5000);
                boost::this_thread::interruption_point();
                if (!fGenerateBitcoins && !fProofOfStake) {
                    if (fDebug)
                        LogPrintf("BitcoinMiner Going out of Loop !!! \n");

                    continue;
                }

                nLastCoinStakeSearchInterval = 0;
                // Do a separate 1 minute check here to ensure fMintableCoins is updated
                if (!fMintableCoins) {
                    if (GetTime() - nMintableLastCheck > Params().GetTargetSpacing()) // 1 minute check time
                    {
                        nMintableLastCheck = GetTime();
                        fMintableCoins = pwallet->MintableCoins();
                    }
                }
            }

            if (mapHashedBlocks.count(chainActive.Tip()->nHeight)) //search our map of hashed blocks, see if bestblock has been hashed yet
            {
                if (GetTime() - mapHashedBlocks[chainActive.Tip()->nHeight] < Params().GetTargetSpacing() * 0.75 / 2)  // wait half of the nHashDrift
                {
                    MilliSleep(5000);
                    boost::this_thread::interruption_point();
                    if (!fGenerateBitcoins && !fProofOfStake) {
                        if (fDebug)
                            LogPrintf("BitcoinMiner Going out of Loop !!! \n");
                        
                        continue;
                    }
                }
            }
        }

        if (vNodes.size() < 2 || nChainHeight < GetBestPeerHeight()) {
            MilliSleep(60000);
            continue;
        }

        if (!fProofOfStake && (chainActive.Tip()->nHeight > Params().GetLastPoWBlock())) {
             if (fDebug) 
               LogPrintf("Pow Period has ended, we need to exit this thread \n");
             
             break;
        }
        //
        // Create new block
        //
        unsigned int nTransactionsUpdatedLast = mempool.GetTransactionsUpdated();
        CBlockIndex* pindexPrev = chainActive.Tip();
        if (!pindexPrev) {
            MilliSleep(500);
            continue;
        }

        unique_ptr<CBlockTemplate> pblocktemplate(CreateNewBlockWithKey(reservekey, pwallet, fProofOfStake));
        if (!pblocktemplate.get())
            continue;

        CBlock* pblock = &pblocktemplate->block;
        IncrementExtraNonce(pblock, pindexPrev, nExtraNonce);

        //Stake miner main
        if (fProofOfStake) {
            LogPrintf("%s(): proof-of-stake block found %s \n", __func__, pblock->GetHash().ToString().c_str());
            
            if (!SignBlock(*pblock, *pwallet)) {
                LogPrintf("%s(): Signing new block with UTXO key failed \n", __func__);
                MilliSleep(500);
                continue;
            }

            if (fDebug)
                LogPrintf("BitcoinMiner : proof-of-stake block was signed %s \n", pblock->GetHash().ToString().c_str());
            
            SetThreadPriority(THREAD_PRIORITY_NORMAL);
            ProcessBlockFound(pblock, *pwallet, reservekey);
            SetThreadPriority(THREAD_PRIORITY_LOWEST);
            MilliSleep(Params().GetTargetSpacingForStake() * 1000);
            continue;
        }

        if (fDebug)
            LogPrintf("Running KOREMiner with %u transactions in block (%u bytes)\n", pblock->vtx.size(), ::GetSerializeSize(*pblock, SER_NETWORK, PROTOCOL_VERSION));

        //
        // Search
        //
        int64_t nStart = GetTime();
        uint256 hashTarget = uint256().SetCompact(pblock->nBits);
        if (fDebug)
            LogPrintf("target: %s\n", hashTarget.GetHex());
        
        while (true) {
            unsigned int nHashesDone = 0;

            uint256 hash;

            if (fDebug)
                LogPrintf("nbits : %08x \n", pblock->nBits);
            
            while (true) {
                hash = pblock->GetHash();
                if (fDebug) {
                    LogPrintf("hash      %s\n", hash.ToString().c_str());
                    LogPrintf("hashTarget %s\n", hashTarget.ToString().c_str());
                }

                if (hash <= hashTarget) {
                    // Found a solution
                    SetThreadPriority(THREAD_PRIORITY_NORMAL);
                    LogPrintf("%s(): proof-of-work found  \n  hash: %s  \ntarget: %s\n", __func__, hash.GetHex(), hashTarget.GetHex());
                    ProcessBlockFound(pblock, *pwallet, reservekey);
                    SetThreadPriority(THREAD_PRIORITY_LOWEST);
                    MilliSleep(Params().GetTargetSpacing() * 1000);

                    // In regression test mode, stop mining after a block is found. This
                    // allows developers to controllably generate a block on demand.
                    if (Params().ShouldMineBlocksOnDemand())
                        throw boost::thread_interrupted();

                    break;
                }
                pblock->nNonce += 1;
                nHashesDone += 1;
                if (fDebug)
                    LogPrintf("Looking for a solution with nounce: %d hashesDone : %d \n", pblock->nNonce, nHashesDone);
                
                if ((pblock->nNonce & 0xFF) == 0)
                    break;
            }

            // Meter hashes/sec
            static int64_t nHashCounter;
            if (nHPSTimerStart == 0) {
                nHPSTimerStart = GetTimeMillis();
                nHashCounter = 0;
            } else
                nHashCounter += nHashesDone;
            if (GetTimeMillis() - nHPSTimerStart > 4000) {
                static CCriticalSection cs;
                {
                    LOCK(cs);
                    if (GetTimeMillis() - nHPSTimerStart > 4000) {
                        dHashesPerMin = 1000.0 * nHashCounter / (GetTimeMillis() - nHPSTimerStart);
                        nHPSTimerStart = GetTimeMillis();
                        nHashCounter = 0;
                        static int64_t nLogTime;
                        if (GetTime() - nLogTime > 30 * 60) {
                            nLogTime = GetTime();
                            if (fDebug)
                                LogPrintf("hashmeter %6.0f khash/s\n", dHashesPerMin / 1000.0);
                        }
                    }
                }
            }

            // Check for stop or if block needs to be rebuilt
            boost::this_thread::interruption_point();
            // Regtest mode doesn't require peers
            if (vNodes.empty() && Params().DoesMiningRequiresPeers())
                break;
            if (pblock->nNonce >= 0xffff0000)
                break;
            if (mempool.GetTransactionsUpdated() != nTransactionsUpdatedLast && GetTime() - nStart > 60)
                break;
            if (pindexPrev != chainActive.Tip())
                break;

            // Update nTime every few seconds
            UpdateTime(pblock, pindexPrev, fProofOfStake);
            // Changing pblock->nTime can change work required:
            hashTarget.SetCompact(pblock->nBits);
        }
    }
    if (fDebug)
        LogPrintf("Exiting kore-miner \n");
}

void static ThreadBitcoinMiner(void* parg)
{
    boost::this_thread::interruption_point();
    CWallet* pwallet = (CWallet*)parg;
    try {
        BitcoinMiner(pwallet, false);
        boost::this_thread::interruption_point();
    } catch (std::exception& e) {
        LogPrintf("ThreadBitcoinMiner( %c) exception", e.what());
    } catch (...) {
        LogPrintf("ThreadBitcoinMiner() exception");
    }

    LogPrintf("ThreadBitcoinMiner exiting\n");
}

void ThreadStakeMinter_Legacy(CWallet* pwallet)
{
    if (fDebug)
        LogPrintf("StakeMiner Legacy started\n");
    
    SetThreadPriority(THREAD_PRIORITY_LOWEST);
    RenameThread("kore-pos-legacy");

    const CChainParams& chainparams = Params();
    boost::shared_ptr<CReserveScript> coinstakeScript;
    GetMainSignals().ScriptForMining(coinstakeScript);

    if (!coinstakeScript || coinstakeScript->reserveScript.empty())
        throw std::runtime_error("No coinstake script available (staking requires a wallet)");

    bool fTryToSync = true;

    // lets say the fork will happen at block 50, this thread can only be running until
    // block 48, because if we are in block 48 it means we are trying to create the last
    // legacy block which is the block 49.
    while (!ShutdownRequested() && UseLegacyCode(GetnHeight(chainActive.Tip()) + 1)) {
        boost::this_thread::interruption_point();

        while (pwallet->IsLocked()) {
            // nLastCoinStakeSearchInterval = 0;
            MilliSleep(2000);
            boost::this_thread::interruption_point();
        }

        while (vNodes.empty() || IsInitialBlockDownload()) {
            fTryToSync = true;
            // nLastCoinStakeSearchInterval = 0;
            MilliSleep(2000);
            boost::this_thread::interruption_point();
        }

		if (fTryToSync)
		{
			fTryToSync = false;
			if (vNodes.size() < 3 || nChainHeight < GetBestPeerHeight())
			{
				MilliSleep(60000);
				continue;
			}
		}

        if (nChainHeight < GetBestPeerHeight() - 1)
        {
            MilliSleep(2000);
            continue;
        }

        // Do we have balance ?
        if (pwallet->GetBalance() <= 0) {
            MilliSleep(60000);
            continue;
        }

        //
        // Create new block
        //
        unique_ptr<CBlockTemplate> pblocktemplate(CreateNewBlock_Legacy(chainparams, coinstakeScript->reserveScript, pwallet, true));
        if (!pblocktemplate.get())
            return;

        CBlock* pblock = &pblocktemplate->block;
        if (SignBlock_Legacy(pwallet, pblock)) {
            SetThreadPriority(THREAD_PRIORITY_NORMAL);
            ProcessBlockFound_Legacy(pblock, chainparams);
            SetThreadPriority(THREAD_PRIORITY_LOWEST);
            MilliSleep(Params().GetTargetSpacingForStake() * 1000);
        }

        MilliSleep(500);
    }

    if (fDebug)
        LogPrintf("Exiting stake-miner-legacy at block: %d", GetnHeight(chainActive.Tip()));
}

void KoreMiner_Legacy()
{
    if (fDebug)
        LogPrintf("KoreMiner_Legacy started\n");
    
    const CChainParams& chainparams = Params();
    SetThreadPriority(THREAD_PRIORITY_LOWEST);
    RenameThread("kore-pow-legacy");

    unsigned int nExtraNonce = 0;

    boost::shared_ptr<CReserveScript> coinbaseScript;
    GetMainSignals().ScriptForMining(coinbaseScript);

    try {
        // Throw an error if no script was provided.  This can happen
        // due to some internal error but also if the keypool is empty.
        // In the latter case, already the pointer is NULL.
        if (!coinbaseScript || coinbaseScript->reserveScript.empty()) {
            if (fDebug)
                LogPrintf("No coinbase script available (mining requires a wallet)\n");
            
            throw std::runtime_error("No coinbase script available (mining requires a wallet)");
        }

        // This thread should exit, if it has reached last
        while (!ShutdownRequested() && UseLegacyCode(GetnHeight(chainActive.Tip()) + 1)) {
            if (chainActive.Tip()->nHeight > Params().GetLastPoWBlock() ) {
                if (fDebug)
                    LogPrintf("Pow Period has ended, we need to exit this thread \n");
                
                break;
            }
            if (chainparams.DoesMiningRequiresPeers()) {
                // Busy-wait for the network to come online so we don't waste time mining
                // on an obsolete chain. In regtest mode we expect to fly solo.
                do {
                    if (fDebug)
                        LogPrintf("KoreMiner_Legacy is waiting for a Peer!!! \n");
                    
                    bool fvNodesEmpty;
                    {
                        LOCK(cs_vNodes);
                        fvNodesEmpty = vNodes.empty();
                    }
                    if (!fvNodesEmpty && !IsInitialBlockDownload())
                        break;
                    MilliSleep(1000);
                } while (true);
            }

            //
            // Create new block
            //
            unsigned int nTransactionsUpdatedLast = mempool.GetTransactionsUpdated();
            CBlockIndex* pindexPrev = chainActive.Tip();

            unique_ptr<CBlockTemplate> pblocktemplate(CreateNewBlock_Legacy(chainparams, coinbaseScript->reserveScript, NULL, false));

            if (!pblocktemplate.get()) {
                LogPrintf("Error in KoreMiner: Keypool ran out, please call keypoolrefill before restarting the mining thread\n");
                
                return;
            }
            CBlock* pblock = &pblocktemplate->block;
            IncrementExtraNonce_Legacy(pblock, pindexPrev, nExtraNonce);

            if (fDebug)
                LogPrintf("KoreMiner_Legacy Running with %u transactions in block (%u bytes)\n", pblock->vtx.size(), ::GetSerializeSize(*pblock, SER_NETWORK, PROTOCOL_VERSION));

            //
            // Search
            //
            int64_t nStart = GetTime();
            arith_uint256 hashTarget = arith_uint256().SetCompact(pblock->nBits);
            uint256 testHash;

            for (;;) {
                unsigned int nHashesDone = 0;
                unsigned int nNonceFound = (unsigned int)-1;
                if (fDebug)
                    LogPrintf("KoreMiner_Legacy Looking for a Hash Solution \n");
                
                for (int i = 0; i < 1; i++) {
                    pblock->nNonce = pblock->nNonce + 1;
                    testHash = pblock->CalculateBestBirthdayHash();
                    nHashesDone++;
                    if (fDebug) {
                        LogPrintf("KoreMiner_Legacy testHash %s\n", testHash.ToString().c_str());
                        LogPrintf("KoreMiner_Legacy Hash Target %s\n", hashTarget.ToString().c_str());
                    }

                    if (UintToArith256(testHash) < hashTarget) {
                        // Found a solution
                        nNonceFound = pblock->nNonce;
                        if (fDebug) {
                            LogPrintf("KoreMiner_Legacy Found Hash %s\n", testHash.ToString().c_str());
                            LogPrintf("KoreMiner_Legacy hash2 %s\n", pblock->GetHash().ToString().c_str());
                        }
                        // Found a solution
                        assert(testHash == pblock->GetHash());

                        SetThreadPriority(THREAD_PRIORITY_NORMAL);
                        ProcessBlockFound_Legacy(pblock, chainparams);
                        SetThreadPriority(THREAD_PRIORITY_LOWEST);
                        MilliSleep(Params().GetTargetSpacing() * 1000);
                        break;
                    }
                }

                // Meter hashes/sec
                static int64_t nHashCounter;
                if (nHPSTimerStart == 0) {
                    nHPSTimerStart = GetTimeMillis();
                    nHashCounter = 0;
                } else
                    nHashCounter += nHashesDone;

                if (GetTimeMillis() - nHPSTimerStart > 4000 * 60) {
                    static CCriticalSection cs;
                    {
                        LOCK(cs);
                        if (GetTimeMillis() - nHPSTimerStart > 4000 * 60) {
                            dHashesPerMin = 1000.0 * nHashCounter * 60 / (GetTimeMillis() - nHPSTimerStart);
                            nHPSTimerStart = GetTimeMillis();
                            nHashCounter = 0;
                            static int64_t nLogTime;
                            if (GetTime() - nLogTime > 30 * 60) {
                                nLogTime = GetTime();
                                LogPrintf("hashmeter %6.0f khash/s\n", dHashesPerMin / 1000.0);
                            }
                        }
                    }
                }

                // Check for stop or if block needs to be rebuilt
                boost::this_thread::interruption_point();
                // Regtest mode doesn't require peers
                if (vNodes.empty() && Params().DoesMiningRequiresPeers())
                    break;
                if (nNonceFound >= 0xffff0000)
                    break;
                if (mempool.GetTransactionsUpdated() != nTransactionsUpdatedLast && GetTime() - nStart > 60)
                    break;
                if (pindexPrev != chainActive.Tip())
                    break;

                // Update nTime every few seconds
                UpdateTime(pblock, pindexPrev);
            }
        }

    } catch (const boost::thread_interrupted&) {
        LogPrintf("KoreMiner_Legacy Exiting at block: %d", GetnHeight(chainActive.Tip()));
        
        throw;
    } catch (const std::runtime_error& e) {
        LogPrintf("KoreMiner runtime error: %s\n", e.what());
        LogPrintf("KoreMiner_Legacy Runtime Error : %s Exiting at block: %d", e.what(), GetnHeight(chainActive.Tip()));
        
        return;
    }
    if (fDebug)
        LogPrintf("Exiting stake-miner-legacy at block: %d", GetnHeight(chainActive.Tip()));
}

// ppcoin: stake minter thread
void ThreadStakeMinter()
{
    boost::this_thread::interruption_point();
    if (fDebug)
        LogPrintf("ThreadStakeMinter started\n");
    
    CWallet* pwallet = pwalletMain;
    try {
        BitcoinMiner(pwallet, true);

        boost::this_thread::interruption_point();
    } catch (std::exception& e) {
        LogPrintf("ThreadStakeMinter() exception \n");
    } catch (...) {
        LogPrintf("ThreadStakeMinter() error \n");
    }
    LogPrintf("ThreadStakeMinter exiting,\n");
}

void StakingCoins(bool fStaking)
{
    static boost::thread_group* stakingThreads = NULL;
    CWallet* pwallet = pwalletMain;

    if (stakingThreads != NULL) {
        stakingThreads->interrupt_all();
        delete stakingThreads;
        stakingThreads = NULL;
    }

    if (!fStaking){
        return;
    }

    stakingThreads = new boost::thread_group();
    stakingThreads->create_thread(boost::bind(&ThreadStakeMinter_Legacy, pwallet));
    stakingThreads->create_thread(boost::bind(&TraceThread<void (*)()>, "stakemint", &ThreadStakeMinter));
}

void GenerateBitcoins(bool fGenerate, CWallet* pwallet, int nThreads)
{
    LogPrintf("GenerateBitcoins with %d threads\n", nThreads);
    
    static boost::thread_group* minerThreads = NULL;
    fGenerateBitcoins = fGenerate;

    if (nThreads < 0) {
        // In regtest threads defaults to 1
        if (Params().GetDefaultMinerThreads())
            nThreads = Params().GetDefaultMinerThreads();
        else
            nThreads = boost::thread::hardware_concurrency();
    }

    if (minerThreads != NULL) {
        minerThreads->interrupt_all();
        delete minerThreads;
        minerThreads = NULL;
    }

    if (nThreads == 0 || !fGenerate)
        return;

    minerThreads = new boost::thread_group();
    for (int i = 0; i < nThreads; i++) {
        minerThreads->create_thread(boost::bind(&KoreMiner_Legacy));
        minerThreads->create_thread(boost::bind(&ThreadBitcoinMiner, pwallet));
    }
}

CBlockTemplate* CreateNewBlockWithKey(CReserveKey& reservekey, CWallet* pwallet, bool fProofOfStake)
{
    CPubKey pubkey;
    if (!reservekey.GetReservedKey(pubkey))
        return NULL;

    CScript scriptPubKey = CScript() << ToByteVector(pubkey) << OP_CHECKSIG;
    return CreateNewBlock(scriptPubKey, pwallet, fProofOfStake);
}

#endif // ENABLE_WALLET
