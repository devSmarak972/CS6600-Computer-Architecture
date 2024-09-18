#include <vector>
#include <list>
#include <iostream>
#include <iomanip>

class CacheBlock {
public:
    unsigned long tag; // Tag for the block
    bool valid;        // Is the block valid?
    bool dirty;        // Is the block dirty (for write-back)?

    CacheBlock() : tag(0), valid(false), dirty(false) {}
    CacheBlock(int tag_) : tag(tag_), valid(false), dirty(false) {}

    void invalidate() {
        valid = false;
        dirty = false;
    }
};


class VictimCache {
private:
    int numBlocks;
    std::list<CacheBlock> blocks;
    int blockSize;
public:
    VictimCache(int nblocks,int blockSize) : numBlocks(nblocks),blockSize(blockSize) {
        for (int i = 0; i < numBlocks; ++i) {
            blocks.push_back(CacheBlock());
        }
        // std::cout<<blocks.size();
    }

    bool findBlock(unsigned long tag) {
        for (auto it = blocks.begin(); it != blocks.end(); ++it) {
            if (it->valid && it->tag == tag) {
                blocks.splice(blocks.begin(), blocks, it); // Move to front (MRU)
                return true;
            }
        }
        return false;
    }

    void swapBlock(unsigned long tag, CacheBlock evictedBlock) {
        // Insert the evicted block into VC (LRU)
        insert(evictedBlock);
    }

    void insert(CacheBlock block) {
        if (blocks.size() == numBlocks) {
            blocks.pop_back(); // Evict LRU block
        }
        blocks.push_front(block); // Insert new block as MRU
    }
    bool evictBlock(unsigned long &evictedAddress, bool &evictedDirty) {
        if (blocks.empty()) return false;
        auto& block = blocks.front();
        evictedAddress = block.tag;
        evictedDirty = block.dirty;
        blocks.erase(blocks.begin());
        return true;
    }
      void printContent() const {
        // std::cout<<"===== VC contents =====\n";
        std::cout<<"set 0: ";
        std::cout<<blocks.size();
        for (const auto& block : blocks) {
                std::cout << " " << std::setw(6) << block.tag;
                if (block.dirty) std::cout << " D";
            }
        std::cout<<"\n";
        // std::cout << "Victim Cache: " << blocks.size() << " blocks\n";
    }

};

class CacheSet {
private:
    int assoc; // Associativity
    std::list<CacheBlock> blocks; // List for maintaining LRU order

public:
    CacheSet(int associativity) : assoc(associativity) {
        // Initialize cache blocks for the set
        for (int i = 0; i < assoc; ++i) {
            blocks.push_back(CacheBlock());
        }
    }

    // Check if a block with a given tag is present in the set
    bool findBlock(unsigned long tag, CacheBlock& block) {
        for (auto it = blocks.begin(); it != blocks.end(); ++it) {
            if (it->valid && it->tag == tag) {
                block = *it;
                blocks.erase(it);
                blocks.push_front(block); // Move to MRU
                return true;
            }
        }
        return false;
    }

    // Insert a block into the set (with LRU replacement)
    CacheBlock evictAndInsert(unsigned long tag, bool dirty) {
        CacheBlock evicted = blocks.back();
        blocks.pop_back(); // Evict the LRU block
        CacheBlock newBlock;
        newBlock.tag = tag;
        newBlock.valid = true;
        newBlock.dirty = dirty;
        blocks.push_front(newBlock); // Insert the new block as MRU
        return evicted;
    }

    bool hasSpace() const {
        for (const auto& block : blocks) {
            if (!block.valid) return true;
        }
        return false;
    }

    void insertBlock(unsigned long tag, bool dirty) {
        for (auto it=blocks.begin();it!=blocks.end();it++) {
            if (!(*it).valid) {
                CacheBlock block=*it;
                blocks.erase(it);
                block.tag = tag;
                block.valid = true;
                block.dirty = dirty;
                blocks.push_front(block); // Move to MRU
                return;
            }
        }
    }
    void displayBlocks()
    {
    for (const auto& block : blocks) {
                std::cout << " " << std::setw(6) <<block.tag;
                if (block.dirty) std::cout << " D";
            }
    }
};


class Cache {
private:
    int size;
    int assoc;
    int blockSize;
    int numSets;
    bool writeBack;
    bool writeAllocate;

    std::vector<CacheSet> sets;
    Cache* nextLevelCache; // Pointer to the next level cache (e.g., L2 or memory)
    VictimCache* victimCache; // Optional victim cache

public:
    int numReads;
    int numReadMisses;
    int numWrites;
    int numWriteMisses;
    int numSwaps;
    int numSwapsFromVC;
    int numWritebacks;
    Cache(int cacheSize, int associativity, int blockSize, Cache* nextLevel = nullptr, int victimCacheSize = 0) 
        : size(cacheSize), assoc(associativity), blockSize(blockSize), nextLevelCache(nextLevel), victimCache(victimCacheSize > 0 ? new VictimCache(victimCacheSize, blockSize) : nullptr),
         numReads(0), numReadMisses(0), numWrites(0), numWriteMisses(0), numSwaps(0), numSwapsFromVC(0), numWritebacks(0) {
        
        numSets = size / (blockSize * assoc);
        sets.resize(numSets, CacheSet(assoc));
       
    }
  ~Cache() {
        delete victimCache;
    }

    unsigned long getTag(unsigned long address) const {
        return address / blockSize;
    }

    int getIndex(unsigned long address) const {
        return (address / blockSize) % numSets;
    }

    void handleRead(unsigned long address) {
        unsigned long tag = getTag(address);
        int index = getIndex(address);

        numReads++;
        CacheBlock block;
        if (sets[index].findBlock(tag, block)) {
            // Cache hit, update LRU
            //Latest block moved to front inside the function
            return;
        }
        numReadMisses++;
        // Cache miss, handle VC if present
        if (victimCache && !sets[index].hasSpace()) {
            // Search VC for the block
            if (victimCache->findBlock(tag)) {
                numSwaps++;
                // Swap block from VC to L1
                numSwapsFromVC++;
                victimCache->swapBlock(tag, sets[index].evictAndInsert(tag, false));
                return;
            }

        }

        // Miss in both cache and VC
        if (nextLevelCache) {
            nextLevelCache->handleRead(address);
            numReadMisses++;
        }

        if (sets[index].hasSpace()) {
            
            sets[index].insertBlock(tag, false);
        } else {
            CacheBlock evicted = sets[index].evictAndInsert(tag, false);
            
            if (victimCache) {
                victimCache->insert(evicted);
            }
        }
    }

    void handleWrite(unsigned long address) {
        unsigned long tag = getTag(address);
        int index = getIndex(address);

        numWrites++;
        CacheBlock block;
        if (sets[index].findBlock(tag, block)) {
            // Cache hit, mark dirty if write-back policy
            block.dirty = true;
            return;
        }
        numWriteMisses++;
        // Cache miss, handle VC
        if (victimCache && !sets[index].hasSpace()) {
            if (victimCache->findBlock(tag)) {
                numSwaps++;
                numSwapsFromVC++;
                victimCache->swapBlock(tag, sets[index].evictAndInsert(tag, true));
                return;
            }
        }

        // Miss in both cache and VC
        if (nextLevelCache) {
            
            nextLevelCache->handleRead(address);
        }

        if (sets[index].hasSpace()) {
            sets[index].insertBlock(tag, true);
        } else {
            CacheBlock evicted = sets[index].evictAndInsert(tag, true);
            if (victimCache) {
                victimCache->insert(evicted);
            }
        }
    }
    void printContents() {
        for (int i = 0; i < numSets; ++i) {
            std::cout << "  set " << i << ":";
            sets[i].displayBlocks();
            std::cout << "\n";
        }
    }
    void printStatistics()  {
        std::cout << "L1 Cache Statistics:\n";
        printContents();
         std::cout << "L1 reads: " << numReads << "\n";
        std::cout << "L1 read misses: " << numReadMisses << "\n";
        std::cout << "L1 writes: " << numWrites << "\n";
        std::cout << "L1 write misses: " << numWriteMisses << "\n";
        std::cout << "Swap requests from L1 to VC: " << numSwaps << "\n";
        std::cout << "Swap request rate: " << (numReads + numWrites > 0 ? static_cast<double>(numSwaps) / (numReads + numWrites) : 0) << "\n";
        std::cout << "Swaps between L1 and VC: " << numSwaps << "\n";
        std::cout << "Combined L1+VC miss rate: " << (numReads + numWrites > 0 ? static_cast<double>(numReadMisses + numWriteMisses - numSwaps) / (numReads + numWrites) : 0) << "\n";
        std::cout << "Writebacks from L1 or VC to next level: " << numWritebacks << "\n";
 
    }
    void printVictimContents() const {
        // std::cout << "Victim Cache Statistics:\n";
        victimCache->printContent();
    }
};



// int main() {
//     // Example setup for CPU -> L1 + VC -> L2 -> Memory hierarchy
//     VictimCache vc(4); // Victim Cache with 4 blocks
//     Cache l2Cache(8192, 4, 64); // L2 Cache with 8KB, 4-way associative, 64-byte block
//     Cache l1Cache(4096, 2, 64, &l2Cache, &vc); // L1 Cache with VC and L2 as next level
//     std::cout<<"here\n";
//     // Example Read/Write
//     l1Cache.handleRead(0x1A2B3C4D);
//     l1Cache.handleWrite(0x1A2B3C4D);
    
//     return 0;
// }

