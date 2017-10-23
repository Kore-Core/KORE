// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The KoreCore developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "primitives/block.h"

#include "hash.h"
#include "tinyformat.h"
#include "momentum.h"
#include "utilstrencodings.h"
#include "crypto/common.h"

uint256 CBlockHeader::GetHash() const
{
    return Hash(BEGIN(nVersion), END(nBirthdayB));
}

uint256 CBlock::GetMidHash() const
{
    return Hash(BEGIN(nVersion), END(nNonce));
}

uint256 CBlock::GetVerifiedHash() const
{
 
 	uint256 midHash = GetMidHash();
 		    	
	uint256 r = Hash(BEGIN(nVersion), END(nBirthdayB));

 	if(!bts::momentum_verify( midHash, nBirthdayA, nBirthdayB)){
 		return uint256("0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffeeee");
 	}
   
     return r;
}
 
uint256 CBlock::CalculateBestBirthdayHash() {
 				
	uint256 midHash = GetMidHash();		
	std::vector< std::pair<uint32_t,uint32_t> > results =bts::momentum_search( midHash );
	uint32_t candidateBirthdayA=0;
	uint32_t candidateBirthdayB=0;
	uint256 smallestHashSoFar("0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffdddd");
	for (unsigned i=0; i < results.size(); i++) {
	nBirthdayA = results[i].first;
	nBirthdayB = results[i].second;
	uint256 fullHash = Hash(BEGIN(nVersion), END(nBirthdayB));
		if(fullHash<smallestHashSoFar){
	
				smallestHashSoFar=fullHash;
				candidateBirthdayA=results[i].first;
				candidateBirthdayB=results[i].second;
			}
			nBirthdayA = candidateBirthdayA;
 			nBirthdayB = candidateBirthdayB;
 		}
 		
 		return GetHash();
}

std::string CBlockHeader::ToString() const
{
    std::stringstream s;
    s << strprintf("CBlockHeader(hash=%s, ver=%d, hashPrevBlock=%s, hashMerkleRoot=%s, nTime=%u, nBits=%08x, nNonce=%u)\n",
        GetHash().ToString(), nVersion, hashPrevBlock.ToString(), hashMerkleRoot.ToString(), nTime, nBits, nNonce);
    return s.str();
}

std::string CBlock::ToString() const
{
    std::stringstream s;
    s << strprintf("CBlock(hash=%s, ver=%d, hashPrevBlock=%s, hashMerkleRoot=%s, nTime=%u, nBits=%08x, nNonce=%u, vtx=%u, vchBlockSig=%s)\n",
        GetHash().ToString(),
        nVersion,
        hashPrevBlock.ToString(),
        hashMerkleRoot.ToString(),
        nTime, nBits, nNonce,
        vtx.size(),
        HexStr(vchBlockSig.begin(), vchBlockSig.end()));
    for (unsigned int i = 0; i < vtx.size(); i++)
    {
        s << "  " << vtx[i].ToString() << "\n";
    }
    return s.str();
}
