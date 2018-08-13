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

uint256 CBlockHeader::GetMidHash() const
{
    return Hash(BEGIN(nVersion), END(nNonce));
}

uint256 CBlockHeader::GetVerifiedHash() const
{
 
 	uint256 midHash = GetMidHash();
 		    	
	uint256 r = Hash(BEGIN(nVersion), END(nBirthdayB));

 	if(!bts::momentum_verify( midHash, nBirthdayA, nBirthdayB)){
 		return uint256S("0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffeeee");
 	}
   
     return r;
}
 
uint256 CBlockHeader::CalculateBestBirthdayHash() {
 				
	uint256 midHash = GetMidHash();		
	std::vector< std::pair<uint32_t,uint32_t> > results =bts::momentum_search( midHash );
	uint32_t candidateBirthdayA=0;
	uint32_t candidateBirthdayB=0;
	uint256 smallestHashSoFar = uint256S("0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffdddd");
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
    s << "CBlock ============================>>>>\n";
    s << strprintf("    hash=%s \n", GetHash().ToString());
    s << strprintf("    ver=%d \n", nVersion);
    s << strprintf("    hashPrevBlock=%s, \n", hashPrevBlock.ToString());
    s << strprintf("    hashMerkleRoot=%s, \n", hashMerkleRoot.ToString());
    s << strprintf("    nTime=%u, \n", nTime);
    s << strprintf("    nBits=%x, \n", nBits);
    s << strprintf("    nNonce=%u, \n", nNonce);
    s << strprintf("    nBirthdayA=%u, \n", nBirthdayA);
    s << strprintf("    nBirthdayB=%u, \n", nBirthdayB);
	s << strprintf("    vchBlockSig=%s, \n", HexStr(vchBlockSig.begin(), vchBlockSig.end()));
	
    s << strprintf("    Vtx : size %u \n",vtx.size());
    for (unsigned int i = 0; i < vtx.size(); i++)
    {
        s << "    " << vtx[i].ToString() << "\n";
    }

    s << "CBlock <<<<============================\n";
    return s.str();
}
