
## Function to create Genesis Block

``` c++
void CChainParams::MineNewGenesisBlock()
{
    printf("Mining genesis block...\n");

    // deliberately empty for loop finds nonce value.
    for (genesis.nNonce = 0; genesis.GetHash() > bnProofOfWorkLimit; genesis.nNonce++) 
    {
        printf("Trying with this nNonce = %u \n", genesis.nNonce);
    }
    printf("genesis.nTime = %u \n", genesis.nTime);
    printf("genesis.nNonce = %u \n", genesis.nNonce);
    printf("genesis.nBirthdayA: %d\n", genesis.nBirthdayA);
    printf("genesis.nBirthdayB: %d\n", genesis.nBirthdayB);
    printf("genesis.nBits = %x \n", genesis.nBits);
    printf("genesis.GetHash = %s\n", genesis.GetHash().ToString().c_str());
    printf("genesis.hashMerkleRoot = %s\n", genesis.hashMerkleRoot.ToString().c_str());

    exit(1);
}

// This is how PTS mine genesis block
// This will figure out a valid hash and Nonce if you're
// creating a different genesis block:
uint256 hashTarget = CBigNum().SetCompact(block.nBits).getuint256();
uint256 thash;
block.nNonce = 0;

while(true)
{
    int collisions=0;
    thash = block.CalculateBestBirthdayHash(collisions);
    if (thash <= hashTarget)
        break;
    printf("nonce %08X: hash = %s (target = %s)\n", block.nNonce, thash.ToString().c_str(),
        hashTarget.ToString().c_str());
    ++block.nNonce;
    if (block.nNonce == 0)
    {
        printf("NONCE WRAPPED, incrementing time\n");
        ++block.nTime;
    }
}
printf("block.nTime = %u \n", block.nTime);
printf("block.nNonce = %u \n", block.nNonce);
printf("block.GetHash = %s\n", block.GetHash().ToString().c_str());
printf("block.nBits = %u \n", block.nBits);
printf("block.nBirthdayA = %u \n", block.nBirthdayA);
printf("block.nBirthdayB = %u \n", block.nBirthdayB);

void CChainParams::MineNewGenesisBlock_Legacy()
{
    fPrintToConsole = true;
    bool fNegative;
    bool fOverflow;
    genesis.nNonce = 0;
    LogPrintStr("Searching for genesis block...\n");

    //arith_uint256 hashTarget = UintToArith256(consensus.powLimit).GetCompact();
    LogPrintStr("Start to Find the Genesis \n");
    arith_uint256 hashTarget = arith_uint256().SetCompact(genesis.nBits, &fNegative, &fOverflow);

    if (fNegative || fOverflow) {
        LogPrintf("Please check nBits, negative or overflow value");
        LogPrintf("genesis.nBits = %x \n",  genesis.nBits);
        exit(1);
    }

    while(true) {
        LogPrintf("nNonce = %u \n",  genesis.nNonce);
        arith_uint256 thash = UintToArith256(genesis.CalculateBestBirthdayHash());
		LogPrintf("theHash %s\n", thash.ToString().c_str());
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
    LogPrintf("genesis.nTime          = %u\n", genesis.nTime);
    LogPrintf("genesis.nNonce         = %u\n", genesis.nNonce);
	LogPrintf("genesis.nBirthdayA     = %d\n", genesis.nBirthdayA);
	LogPrintf("genesis.nBirthdayB     = %d\n", genesis.nBirthdayB);
    LogPrintf("genesis.nBits          = %x\n", genesis.nBits);
    LogPrintf("genesis.GetHash        = %s\n", genesis.GetHash().ToString().c_str());
    LogPrintf("genesis.hashMerkleRoot = %s\n", genesis.hashMerkleRoot.ToString().c_str());

    exit(1);
}
```