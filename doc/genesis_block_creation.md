
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

```