// Copyright (c) 2012-2013 The PPCoin developers
// Copyright (c) 2015-2018 The KORE developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/assign/list_of.hpp>
#include <boost/lexical_cast.hpp>

#include "db.h"
#include "kernel.h"
#include "script/interpreter.h"
#include "stakeinput.h"
#include "timedata.h"
#include "util.h"
#include "wallet.h"

using namespace std;

// Hard checkpoints of stake modifiers to ensure they are deterministic
static std::map<int, unsigned int> mapStakeModifierCheckpoints =
    boost::assign::map_list_of(0, 0xfd11f4e7u);

// Get time weight
int64_t GetWeight(int64_t nIntervalBeginning, int64_t nIntervalEnd)
{
    return nIntervalEnd - nIntervalBeginning - Params().GetStakeMinAge();
}

// Get the last stake modifier and its generation time from a given block
static bool GetLastStakeModifier(const CBlockIndex* pindex, uint64_t& nStakeModifier, int64_t& nModifierTime)
{
    if (!pindex)
        return error("GetLastStakeModifier: null pindex");
    while (pindex && pindex->pprev && !pindex->GeneratedStakeModifier())
        pindex = pindex->pprev;
    if (!pindex->GeneratedStakeModifier())
        return error("GetLastStakeModifier: no generation at genesis block");
    nStakeModifier = pindex->nStakeModifier;
    nModifierTime = pindex->GetBlockTime();
    return true;
}

// Get selection interval section (in seconds)
static int64_t GetStakeModifierSelectionIntervalSection(int nSection)
{
    int nminimum = Params().GetMaxStakeModifierInterval() - 1;
    assert(nSection >= 0 && nSection < Params().GetMaxStakeModifierInterval());
    return Params().GetModifierInterval() * nminimum / (nminimum + ((nminimum - nSection) * (MODIFIER_INTERVAL_RATIO - 1)));
}

// Get stake modifier selection interval (in seconds)
static int64_t GetStakeModifierSelectionInterval(int nHeight)
{
    int64_t nSelectionInterval = 0;
    for (int nSection = 0; nSection < Params().GetMaxStakeModifierInterval(); nSection++) {
        nSelectionInterval += GetStakeModifierSelectionIntervalSection(nSection);
    }
    return nSelectionInterval;
}

// select a block from the candidate blocks in vSortedByTimestamp, excluding
// already selected blocks in vSelectedBlocks, and with timestamp up to
// nSelectionIntervalStop.
static bool SelectBlockFromCandidates(
    vector<pair<int64_t, uint256> >& vSortedByTimestamp,
    map<uint256, const CBlockIndex*>& mapSelectedBlocks,
    int64_t nSelectionIntervalStop,
    uint64_t nStakeModifierPrev,
    const CBlockIndex** pindexSelected)
{
    bool fModifierV2 = false;
    bool fSelected = false;
    uint256 hashBest = 0;
    *pindexSelected = (const CBlockIndex*)0;
    BOOST_FOREACH (const PAIRTYPE(int64_t, uint256) & item, vSortedByTimestamp) {
        if (!mapBlockIndex.count(item.second))
            return error("SelectBlockFromCandidates: failed to find block index for candidate block %s", item.second.ToString().c_str());

        const CBlockIndex* pindex = mapBlockIndex[item.second];
        if (fSelected && pindex->GetBlockTime() > nSelectionIntervalStop)
            break;

        if (mapSelectedBlocks.count(pindex->GetBlockHash()) > 0)
            continue;

        // compute the selection hash by hashing an input that is unique to that block
        uint256 hashProof;
        hashProof = pindex->GetBlockHash();

        CDataStream ss(SER_GETHASH, 0);
        ss << hashProof << nStakeModifierPrev;
        uint256 hashSelection = Hash(ss.begin(), ss.end());

        // the selection hash is divided by 2**32 so that proof-of-stake block
        // is always favored over proof-of-work block. this is to preserve
        // the energy efficiency property
        if (pindex->IsProofOfStake())
            hashSelection >>= 32;

        if (fSelected && hashSelection < hashBest) {
            hashBest = hashSelection;
            *pindexSelected = (const CBlockIndex*)pindex;
        } else if (!fSelected) {
            fSelected = true;
            hashBest = hashSelection;
            *pindexSelected = (const CBlockIndex*)pindex;
        }
    }
    if (GetBoolArg("-printstakemodifier", false))
        LogPrintf("SelectBlockFromCandidates: selection hash=%s\n", hashBest.ToString().c_str());
    return fSelected;
}

// This function will only be used before fork happens
void StartStakeModifier_Legacy(CBlockIndex* pindexNew)
{
    if (pindexNew->nHeight < Params().GetLastPoWBlock()) {
        //Give a stake modifier to the first block
        // Lets give a stake modifier to the last block
        uint64_t nStakeModifier = PREDEFINED_MODIFIER; //uint64_t("stakemodifier");
        pindexNew->SetStakeModifier(nStakeModifier, true);
        pindexNew->nStakeModifierChecksum = GetStakeModifierChecksum(pindexNew);

        if (pindexNew->nHeight)
            pindexNew->pprev->pnext = pindexNew;

        // mark as PoS seen
        if (pindexNew->IsProofOfStake())
            setStakeSeen.insert(make_pair(pindexNew->prevoutStake, pindexNew->nStakeTime));
    }
}

// Stake Modifier (hash modifier of proof-of-stake):
// The purpose of stake modifier is to prevent a txout (coin) owner from
// computing future proof-of-stake generated by this txout at the time
// of transaction confirmation. To meet kernel protocol, the txout
// must hash with a future stake modifier to generate the proof.
// Stake modifier consists of bits each of which is contributed from a
// selected block of a given block group in the past.
// The selection of a block is based on a hash of the block's proof-hash and
// the previous stake modifier.
// Stake modifier is recomputed at a fixed time interval instead of every
// block. This is to make it difficult for an attacker to gain control of
// additional bits in the stake modifier, even after generating a chain of
// blocks.
bool ComputeNextStakeModifier(const CBlockIndex* pindexPrev, uint64_t& nStakeModifier, bool& fGeneratedStakeModifier)
{
    nStakeModifier = 0;
    fGeneratedStakeModifier = false;
    if (!pindexPrev) {
        fGeneratedStakeModifier = true;
        return true; // genesis block's modifier is 0
    }
    if (pindexPrev->nHeight == 0 || Params().HeightToFork() == pindexPrev->nHeight + 1) {
        // Give a stake modifier to the first block
        // Lets give a stake modifier First Block After Fork
        fGeneratedStakeModifier = true;        
        nStakeModifier = PREDEFINED_MODIFIER; //uint64_t("stakemodifier");
        return true;
    }

    // First find current stake modifier and its generation block time
    // if it's not old enough, return the same stake modifier
    int64_t nModifierTime = 0;
    if (!GetLastStakeModifier(pindexPrev, nStakeModifier, nModifierTime))
        return error("ComputeNextStakeModifier: unable to get last modifier");

    if (GetBoolArg("-printstakemodifier", false))
        LogPrintf("ComputeNextStakeModifier: prev modifier= %s time=%s\n", boost::lexical_cast<std::string>(nStakeModifier).c_str(), DateTimeStrFormat("%Y-%m-%d %H:%M:%S", nModifierTime).c_str());

    // Checks if it is already time to generate a new Modifier
    if (Params().GetModifierInterval() > pindexPrev->GetBlockTime() - nModifierTime)
        return true;

    // Sort candidate blocks by timestamp
    vector<pair<int64_t, uint256> > vSortedByTimestamp;
    vSortedByTimestamp.reserve(Params().GetMaxStakeModifierInterval() * Params().GetModifierInterval() / Params().GetTargetSpacingForStake());
    int64_t nSelectionInterval = GetStakeModifierSelectionInterval(pindexPrev->nHeight);
    int64_t nSelectionIntervalStart = pindexPrev->GetBlockTime() - nSelectionInterval;
    const CBlockIndex* pindex = pindexPrev;

    while (pindex && pindex->GetBlockTime() >= nSelectionIntervalStart) {
        vSortedByTimestamp.push_back(make_pair(pindex->GetBlockTime(), pindex->GetBlockHash()));
        pindex = pindex->pprev;
    }

    int nHeightFirstCandidate = pindex ? (pindex->nHeight + 1) : 0;
    reverse(vSortedByTimestamp.begin(), vSortedByTimestamp.end());
    // TODO: Verify if really needed!
    sort(vSortedByTimestamp.begin(), vSortedByTimestamp.end());

    // Select 64 blocks from candidate blocks to generate stake modifier
    uint64_t nStakeModifierNew = 0;
    int64_t nSelectionIntervalStop = nSelectionIntervalStart;
    map<uint256, const CBlockIndex*> mapSelectedBlocks;
    for (int nRound = 0; nRound < min(Params().GetMaxStakeModifierInterval(), (int)vSortedByTimestamp.size()); nRound++) {
        // add an interval section to the current selection round
        nSelectionIntervalStop += GetStakeModifierSelectionIntervalSection(nRound);

        // select a block from the candidates of current round
        if (!SelectBlockFromCandidates(vSortedByTimestamp, mapSelectedBlocks, nSelectionIntervalStop, nStakeModifier, &pindex))
            return error("ComputeNextStakeModifier: unable to select block at round %d", nRound);

        // write the entropy bit of the selected block
        nStakeModifierNew |= (((uint64_t)pindex->GetStakeEntropyBit()) << nRound);

        // add the selected block from candidates to selected list
        mapSelectedBlocks.insert(make_pair(pindex->GetBlockHash(), pindex));
        if (GetBoolArg("-printstakemodifier", false))
            LogPrintf("ComputeNextStakeModifier: selected round %d stop=%s height=%d bit=%d\n",
                nRound, DateTimeStrFormat("%Y-%m-%d %H:%M:%S", nSelectionIntervalStop).c_str(), pindex->nHeight, pindex->GetStakeEntropyBit());
    }

    // Print selection map for visualization of the selected blocks
    if (GetBoolArg("-printstakemodifier", false)) {
        string strSelectionMap = "";
        // '-' indicates proof-of-work blocks not selected
        strSelectionMap.insert(0, pindexPrev->nHeight - nHeightFirstCandidate + 1, '-');
        pindex = pindexPrev;
        while (pindex && pindex->nHeight >= nHeightFirstCandidate) {
            // '=' indicates proof-of-stake blocks not selected
            if (pindex->IsProofOfStake())
                strSelectionMap.replace(pindex->nHeight - nHeightFirstCandidate, 1, "=");
            pindex = pindex->pprev;
        }
        BOOST_FOREACH (const PAIRTYPE(uint256, const CBlockIndex*) & item, mapSelectedBlocks) {
            // 'S' indicates selected proof-of-stake blocks
            // 'W' indicates selected proof-of-work blocks
            strSelectionMap.replace(item.second->nHeight - nHeightFirstCandidate, 1, item.second->IsProofOfStake() ? "S" : "W");
        }
        LogPrintf("ComputeNextStakeModifier: selection height [%d, %d] map %s\n", nHeightFirstCandidate, pindexPrev->nHeight, strSelectionMap.c_str());
    }
    if (GetBoolArg("-printstakemodifier", false)) {
        LogPrintf("ComputeNextStakeModifier: new modifier=%s time=%s\n", boost::lexical_cast<std::string>(nStakeModifierNew).c_str(), DateTimeStrFormat("%Y-%m-%d %H:%M:%S", pindexPrev->GetBlockTime()).c_str());
    }

    nStakeModifier = nStakeModifierNew;
    fGeneratedStakeModifier = true;
    return true;
}

// The stake modifier used to hash for a stake kernel is chosen as the stake
// modifier about a selection interval later than the coin generating the kernel
bool GetKernelStakeModifier(uint256 hashBlockFrom, uint64_t& nStakeModifier, bool fPrintProofOfStake)
{
    nStakeModifier = 0;
    if (!mapBlockIndex.count(hashBlockFrom))
        return error("GetKernelStakeModifier() : block not indexed");
    const CBlockIndex* pindexFrom = mapBlockIndex[hashBlockFrom];
    // Lets check if the block is from befor fork, if so we need to change it
    if (UseLegacyCode(pindexFrom->nHeight)) {
        // for all blocks before fork we will return an modifier as
        // it was found at the last block before fork, please note that
        // we will no be able to store the last kore block, once we don't have
        // some fields in the database yet, like nFlags, nStakeModifier
        nStakeModifier = PREDEFINED_MODIFIER; //uint64_t("stakemodifier");

        if(fDebug)
            LogPrintf("GetKernelStakeModifier(): PREDEFINED_MODIFIER, nStakeModifier=%u\n", nStakeModifier);

        return true;
    }

    int32_t nStakeModifierHeight = pindexFrom->nHeight;
    int64_t nStakeModifierTime = pindexFrom->GetBlockTime();
    int64_t nStakeModifierSelectionInterval = GetStakeModifierSelectionInterval(pindexFrom->nHeight);
    int64_t nTargetTime = pindexFrom->GetBlockTime() + nStakeModifierSelectionInterval;
    const CBlockIndex* pindex = pindexFrom;
    CBlockIndex* pindexNext = chainActive[pindexFrom->nHeight + 1];

    // loop to find the stake modifier later by a selection interval

    if (fDebug) {
        LogPrintf("GetKernelStakeModifier coin from             : %d \n", pindexFrom->nHeight);
        LogPrintf("GetKernelStakeModifier StakeModifierInterval : %d \n", nStakeModifierSelectionInterval);
        LogPrintf("GetKernelStakeModifier target Time           : %u \n", nTargetTime);
    }
    while (nStakeModifierTime < nTargetTime) {
        if (!pindexNext) {
            // there is no more modifier generated, this situation should
            // never happen! Check your configuration
            nStakeModifier = PREDEFINED_MODIFIER; //uint64_t("stakemodifier");
            return true;
        }

        pindex = pindexNext;
        pindexNext = chainActive[pindexNext->nHeight + 1];
        if (pindex->GeneratedStakeModifier()) {
            nStakeModifierHeight = pindex->nHeight;
            nStakeModifierTime = pindex->GetBlockTime();
        }
        if (fDebug)
            LogPrintf("GetKernelStakeModifier nStakeModifierTime           : %u  nStakeModifierHeight : %d block: %d \n", nStakeModifierTime, nStakeModifierHeight, pindex->nHeight);
    }
    nStakeModifier = pindex->nStakeModifier;
    return true;
}

bool CheckStake(const CDataStream& ssUniqueID, CAmount nValueIn, const uint64_t nStakeModifier, const uint256& bnTarget, unsigned int nTimeBlockFrom, unsigned int& nTimeTx)
{
    CHashWriter ss(SER_GETHASH, 0);
    ss << nStakeModifier << nTimeBlockFrom << ssUniqueID << nTimeTx;
    uint256 hashProofOfStake = ss.GetHash();

    // get the stake weight - weight is equal to coin amount
    uint256 bnCoinDayWeight = uint256(nValueIn) / MINIMUM_STAKE_VALUE;

    // Now check if proof-of-stake hash meets target protocol
    bool canStake = hashProofOfStake / bnCoinDayWeight < bnTarget;

    if (fDebug) {
        LogPrintf("CheckStake()\n");
        LogPrintf("hash:                %s\n", hashProofOfStake.ToString());
        LogPrintf("hashWeightened:      %s\n", (hashProofOfStake / bnCoinDayWeight).ToString());
        LogPrintf("target:              %s\n", bnTarget.ToString());
        LogPrintf("bnCoinDayWeight:     %s\n", bnCoinDayWeight.ToString());
        LogPrintf("nValueIn:            %s\n", nValueIn);
        LogPrintf("nStakeModifier:      %u\n", nStakeModifier);
        LogPrintf("nTimeBlockFrom:      %u\n", nTimeBlockFrom);
        LogPrintf("nTimeTx:             %u\n", nTimeTx);
        LogPrintf("%s\n", canStake ? "Can stake" : "Can't stake");
    }

    return canStake;
}

bool IsBelowMinAge(const COutput& output, const unsigned int nTimeBlockFrom, const unsigned int nTimeTx)
{
    return (output.nDepth < Params().GetCoinMaturity() && nTimeTx - nTimeBlockFrom < Params().GetStakeMinAge());
}

bool Stake(CStakeInput* stakeInput, unsigned int nBits, unsigned int nTimeBlockFrom, unsigned int& nTimeTx, CAmount stakeableBalance, const COutput& output)
{
    if (nTimeTx < nTimeBlockFrom)
        return error("Stake() : nTime violation => nTimeTx=%d nTimeBlockFrom=%d", nTimeTx, nTimeBlockFrom );

    if (IsBelowMinAge(output, nTimeBlockFrom, nTimeTx))
      return error("Stake() : min age violation - nTimeBlockFrom=%d nStakeMinAge=%d nTimeTx=%d",
            nTimeBlockFrom, Params().GetStakeMinAge(), nTimeTx);

    // grab difficulty
    uint256 bnTargetPerCoinDay;
    bnTargetPerCoinDay.SetCompact(nBits);

    // grab stake modifier
    uint64_t nStakeModifier = 0;
    if (!stakeInput->GetModifier(nStakeModifier))
        return error("failed to get kernel stake modifier");

    bool fSuccess = false;
    unsigned int nTryTime = 0;
    int nHeightStart = chainActive.Height();
    int nHashDrift = Params().GetTargetSpacing() * 0.75;
    CDataStream ssUniqueID = stakeInput->GetUniqueness();

    for (int i = 0; i < nHashDrift; i++) //iterate the hashing
    {
        // new block came in, move on
        if (chainActive.Height() != nHeightStart)
            break;

        // hash this iteration
        nTryTime = nTimeTx + nHashDrift - i;

        // if stake hash does not meet the target then continue to next iteration
        if (!CheckStake(ssUniqueID, stakeableBalance, nStakeModifier, bnTargetPerCoinDay, nTimeBlockFrom, nTryTime))
            continue;

        fSuccess = true; // if we make it this far then we have successfully created a stake hash

        nTimeTx = nTryTime;
        break;
    }

    mapHashedBlocks.clear();
    mapHashedBlocks[chainActive.Tip()->nHeight] = GetTime(); // store a time stamp of when we last hashed on this block
    return fSuccess;
}

// Check kernel hash target and coinstake signature
bool CheckProofOfStake(const CBlock block, uint256& hashProofOfStake, std::list<CKoreStake>& listStake, CAmount& stakedBalance)
{
    const CTransaction tx = block.vtx[1];
    if (!tx.IsCoinStake())
        return error("CheckProofOfStake(): called on non-coinstake %s", tx.GetHash().ToString().c_str());

    
    uint256 bnTargetPerCoinDay;
    bnTargetPerCoinDay.SetCompact(block.nBits);
    if (bnTargetPerCoinDay > Params().ProofOfStakeLimit())
        return error("%s(): Target is easier than limit %s", __func__, bnTargetPerCoinDay.ToString());
        
    CTransaction originTx;
    uint256 hashBlock = 0;
    if (!GetTransaction(block.vtx[1].vin[0].prevout.hash, originTx, hashBlock, true))
        return error("%s(): Origin tx (%s) not found for block %s. Possible reorg underway so we are skipping a few checks.", __func__, block.GetHash().ToString(), block.vtx[1].vin[0].prevout.hash.ToString());
    
    uint160 lockPubKeyID;
    if (!ExtractDestination(originTx.vout[block.vtx[1].vin[0].prevout.n].scriptPubKey, lockPubKeyID))
        return error("%s(): Couldn't get destination from script: %s", __func__, originTx.vout[block.vtx[1].vin[0].prevout.n].scriptPubKey.ToString());

    // Second transaction must lock coins from same pubkey as coinbase
    uint160 pubKeyID;
    if (!ExtractDestination(block.vtx[0].vout[1].scriptPubKey, pubKeyID))
        return error("%s(): Couldn't get destination from script: %s", __func__, block.vtx[0].vout[1].scriptPubKey.ToString());

    if (lockPubKeyID != pubKeyID)
        return error("%s(): locking pubkey different from coinbase pubkey", __func__);

    // There must be only one pubkey on the locking transaction
    for (unsigned int i = 1; i < block.vtx[1].vin.size(); i++) {
        CTransaction otherOriginTx;
        uint256 otherHashBlock;
        pubKeyID.SetNull();
        if (!GetTransaction(block.vtx[1].vin[i].prevout.hash, otherOriginTx, otherHashBlock, true))
            return error("%s(): Other origin tx (%s) not found for block %s. Possible reorg underway so we are skipping a few checks.", __func__, block.GetHash().ToString(), block.vtx[1].vin[i].prevout.hash.ToString());
        
        if (!ExtractDestination(otherOriginTx.vout[block.vtx[1].vin[i].prevout.n].scriptPubKey, pubKeyID))
            return error("%s(): Couldn't get destination from script: %s", __func__, otherOriginTx.vout[block.vtx[1].vin[i].prevout.n].scriptPubKey.ToString());

        if (lockPubKeyID != pubKeyID)
            return error("%s(): more than one pubkey on lock", __func__);
    }

    // All the outputs pubkeys must be the same as the locking pubkey
    for (unsigned int i = 0; i < block.vtx[1].vout.size(); i++) {
        pubKeyID.SetNull();
        if (!ExtractDestination(block.vtx[1].vout[i].scriptPubKey, pubKeyID))
            return error("%s(): Couldn't get destination from script: %s", __func__, block.vtx[1].vout[i].scriptPubKey.ToString());

        if (pubKeyID != lockPubKeyID)
            return error("%s(): more than one pubkey on lock tx", __func__);
    }

    CKoreStake kernel;
    stakedBalance = 0;
    for (int i = 0; i < tx.vin.size(); i++)
    {
        const CTxIn& txin = tx.vin[i];

        // First try finding the previous transaction in database
        uint256 hashBlock;
        CTransaction txPrev;
        if (!GetTransaction(txin.prevout.hash, txPrev, hashBlock, true))
            return error("CheckProofOfStake(): INFO: read txPrev failed");

        // verify signature and script
        if (!VerifyScript(txin.scriptSig, txPrev.vout[txin.prevout.n].scriptPubKey, STANDARD_SCRIPT_VERIFY_FLAGS, TransactionSignatureChecker(&tx, i)))
            return error("CheckProofOfStake(): VerifySignature failed on coinstake %s", tx.GetHash().ToString().c_str());

        // Construct the stakeinput object
        CKoreStake koreInput;
        koreInput.SetInput(txPrev, txin.prevout.n);
        CBlockIndex* pindex = koreInput.GetIndexFrom();
        if (!pindex)
            return error("%s: Failed to find the block index", __func__);

        stakedBalance += koreInput.GetValue();

        listStake.emplace_back(koreInput);

        if (i == 0)
            kernel = koreInput;
    }

    CBlockIndex* pindex = kernel.GetIndexFrom();

    CTransaction kernelTxFrom;
    kernel.GetTxFrom(kernelTxFrom);
    if (kernelTxFrom.IsNull())
        return error("%s failed to get parent tx for stake input\n", __func__);

    uint64_t nStakeModifier = 0;
    if (!kernel.GetModifier(nStakeModifier))
        return error("%s failed to get modifier for stake input\n", __func__);

    unsigned int nTxTime = block.nTime;
    if (!CheckStake(kernel.GetUniqueness(), stakedBalance, nStakeModifier, bnTargetPerCoinDay, kernelTxFrom.nTime, nTxTime)) {
        return error("CheckProofOfStake(): INFO: check kernel failed on coinstake %s \n", tx.GetHash().GetHex());
    }

    return true;
}

// Check whether the coinstake timestamp meets protocol
bool CheckCoinStakeTimestamp(int64_t nTimeBlock, int64_t nTimeTx)
{
    // v0.3 protocol
    return (nTimeBlock == nTimeTx);
}

// Get stake modifier checksum
unsigned int GetStakeModifierChecksum(const CBlockIndex* pindex)
{
    assert(pindex->pprev || pindex->GetBlockHash() == Params().HashGenesisBlock());
    // Hash previous checksum with flags, hashProofOfStake and nStakeModifier
    CDataStream ss(SER_GETHASH, 0);
    if (pindex->pprev)
        ss << pindex->pprev->nStakeModifierChecksum;
    ss << pindex->nFlags << pindex->hashProofOfStake << pindex->nStakeModifier;
    uint256 hashChecksum = Hash(ss.begin(), ss.end());
    hashChecksum >>= (256 - 32);
    return hashChecksum.Get64();
}

// Check stake modifier hard checkpoints
bool CheckStakeModifierCheckpoints(int nHeight, unsigned int nStakeModifierChecksum)
{
    if (Params().GetNetworkID() == CBaseChainParams::TESTNET) return true; // Testnet has no checkpoints
    if (mapStakeModifierCheckpoints.count(nHeight)) {
        return nStakeModifierChecksum == mapStakeModifierCheckpoints[nHeight];
    }
    return true;
}
