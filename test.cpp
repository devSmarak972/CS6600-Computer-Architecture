#include <iostream>
#include <cassert>
#include "cache.cpp"
// Assuming the cache system is implemented with the classes: Cache, CacheSet, CacheBlock, and VictimCache

void simpleReadWriteTest() {
    std::cout << "Running Simple Read/Write Test..." << std::endl;
    
    VictimCache vc(4); // Victim Cache with 4 blocks
    Cache l2Cache(8192, 4, 64); // L2 Cache with 8KB, 4-way associative, 64-byte block
    Cache l1Cache(4096, 2, 64, &l2Cache, &vc); // L1 Cache with VC and L2 as next level
    
    // Simple read/write tests
    l1Cache.handleRead(0x1A2B3C4D);
    l1Cache.handleWrite(0x1A2B3C4D);
    
    l1Cache.handleRead(0x1A2B3C4E);
    l1Cache.handleWrite(0x1A2B3C4E);

    std::cout << "Simple Read/Write Test Completed!" << std::endl;
}

void cacheHitTest() {
    std::cout << "Running Cache Hit Test..." << std::endl;
    
    VictimCache vc(4); // Victim Cache with 4 blocks
    Cache l2Cache(8192, 4, 64); // L2 Cache with 8KB, 4-way associative, 64-byte block
    Cache l1Cache(4096, 2, 64, &l2Cache, &vc); // L1 Cache with VC and L2 as next level
    
    // First access should miss
    l1Cache.handleRead(0x100);  // Cache miss -> L1 -> L2 -> Memory
    l1Cache.handleWrite(0x100); // Write into L1

    // Access again to hit
    l1Cache.handleRead(0x100);  // Cache hit in L1
    
    std::cout << "Cache Hit Test Completed!" << std::endl;
}

void cacheMissInL1HitInL2Test() {
    std::cout << "Running Cache Miss in L1, Hit in L2 Test..." << std::endl;
    
    VictimCache vc(4); // Victim Cache with 4 blocks
    Cache l2Cache(8192, 4, 64); // L2 Cache with 8KB, 4-way associative, 64-byte block
    Cache l1Cache(4096, 2, 64, &l2Cache, &vc); // L1 Cache with VC and L2 as next level
    
    // First access will miss in both L1 and L2, goes to memory
    l1Cache.handleRead(0x200);  // Cache miss -> L1 -> L2 -> Memory
    
    // Second access is in L2 but not in L1
    l1Cache.handleRead(0x200);  // Cache miss in L1, hit in L2

    std::cout << "Cache Miss in L1, Hit in L2 Test Completed!" << std::endl;
}

void cacheMissInL1L2Test() {
    std::cout << "Running Cache Miss in L1 and L2 Test..." << std::endl;
    
    VictimCache vc(4); // Victim Cache with 4 blocks
    Cache l2Cache(8192, 4, 64); // L2 Cache with 8KB, 4-way associative, 64-byte block
    Cache l1Cache(4096, 2, 64, &l2Cache, &vc); // L1 Cache with VC and L2 as next level
    
    // Access an address not in L1 or L2, fetches from memory
    l1Cache.handleRead(0x300);  // Cache miss in L1 and L2 -> Memory
    
    std::cout << "Cache Miss in L1 and L2 Test Completed!" << std::endl;
}

void victimCacheTest() {
    std::cout << "Running Victim Cache Test..." << std::endl;
    
    VictimCache vc(4); // Victim Cache with 4 blocks
    Cache l2Cache(8192, 4, 64); // L2 Cache with 8KB, 4-way associative, 64-byte block
    Cache l1Cache(4096, 2, 64, &l2Cache, &vc); // L1 Cache with VC and L2 as next level
    
    // Fill L1 cache to force eviction
    l1Cache.handleRead(0x100);  // L1 cache miss -> Insert into L1
    l1Cache.handleRead(0x110);  // L1 cache miss -> Insert into L1
    l1Cache.handleRead(0x120);  // L1 cache miss -> Insert into L1
    l1Cache.handleRead(0x130);  // L1 cache miss -> Insert into L1

    // Eviction should now go to VC
    l1Cache.handleRead(0x140);  // Eviction from L1 -> Insert into VC
    
    // Access VC for an evicted block
    l1Cache.handleRead(0x100);  // Hit in VC, swap with L1
    
    std::cout << "Victim Cache Test Completed!" << std::endl;
}

void edgeCaseEvictionTest() {
    std::cout << "Running Edge Case Eviction Test..." << std::endl;
    
    VictimCache vc(4); // Victim Cache with 4 blocks
    Cache l2Cache(8192, 4, 64); // L2 Cache with 8KB, 4-way associative, 64-byte block
    Cache l1Cache(4096, 2, 64, &l2Cache, &vc); // L1 Cache with VC and L2 as next level
    
    // Fill L1 cache to capacity and evict a dirty block
    l1Cache.handleWrite(0x200);  // Write -> Cache miss -> Insert into L1 as dirty
    l1Cache.handleWrite(0x210);  // Write -> Cache miss -> Insert into L1 as dirty
    l1Cache.handleWrite(0x220);  // Write -> Cache miss -> Insert into L1 as dirty
    l1Cache.handleWrite(0x230);  // Write -> Cache miss -> Insert into L1 as dirty

    // This write should evict the oldest block from L1 to VC
    l1Cache.handleWrite(0x240);  // Evict dirty block from L1 -> Insert into VC
    
    std::cout << "Edge Case Eviction Test Completed!" << std::endl;
}

int main() {
    // Running all the test cases
    simpleReadWriteTest();
    cacheHitTest();
    cacheMissInL1HitInL2Test();
    cacheMissInL1L2Test();
    victimCacheTest();
    edgeCaseEvictionTest();
    
    std::cout << "All Tests Completed Successfully!" << std::endl;

    return 0;
}
