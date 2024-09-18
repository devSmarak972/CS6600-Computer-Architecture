#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <unordered_map>
#include <queue>
#include <cassert>
#include <assert.h>
#include <cstring>
// Assuming Cache and VictimCache classes are defined elsewhere
#include "cache_sim.cc"
#include "parse.h"

// Function to parse command line arguments
void parseArguments(int argc, char *argv[], int &L1_SIZE, int &L1_ASSOC, int &L1_BLOCKSIZE,
                    int &VC_NUM_BLOCKS, int &L2_SIZE, int &L2_ASSOC, std::string &trace_file) {
    if (argc != 7 && argc != 8) {
        std::cerr << "Usage: " << argv[0] << " <L1_SIZE> <L1_ASSOC> <L1_BLOCKSIZE> <VC_NUM_BLOCKS> <L2_SIZE> <L2_ASSOC> <trace_file>\n";
        exit(EXIT_FAILURE);
    }
    
    std::istringstream(argv[1]) >> L1_SIZE;
    std::istringstream(argv[2]) >> L1_ASSOC;
    std::istringstream(argv[3]) >> L1_BLOCKSIZE;
    std::istringstream(argv[4]) >> VC_NUM_BLOCKS;
    std::istringstream(argv[5]) >> L2_SIZE;
    std::istringstream(argv[6]) >> L2_ASSOC;
    trace_file = argv[7];
     std::cout<<"===== Simulator configuration =====\n"; 
        std::cout<<"L1_SIZE:\t\t"<<L1_SIZE<<"\n"; 
        std::cout<<"L1_ASSOC:\t\t"<<L1_ASSOC<<"\n"; 
        std::cout<<"L1_BLOCKSIZE:\t\t"<<L1_BLOCKSIZE<<"\n"; 
        std::cout<<"VC_NUM_BLOCKS:\t\t"<<VC_NUM_BLOCKS<<"\n"; 
        std::cout<<"L2_SIZE:\t\t"<<L2_SIZE<<"\n"; 
        std::cout<<"L2_ASSOC:\t\t"<<L2_ASSOC<<"\n"; 
        std::cout<<"trace_file:\t\t"<<trace_file<<"\n"; 
        
}

int main(int argc, char *argv[]) {
    // Parse command-line arguments
    int L1_SIZE, L1_ASSOC, L1_BLOCKSIZE, VC_NUM_BLOCKS, L2_SIZE, L2_ASSOC;
    std::string trace_file;
    parseArguments(argc, argv, L1_SIZE, L1_ASSOC, L1_BLOCKSIZE, VC_NUM_BLOCKS, L2_SIZE, L2_ASSOC, trace_file);
    
    // Initialize L1 Cache
    Cache *l2Cache = nullptr;
    if (L2_SIZE > 0) {
        l2Cache = new Cache(L2_SIZE, L2_ASSOC, L1_BLOCKSIZE);
    }
    Cache l1Cache(L1_SIZE, L1_ASSOC, L1_BLOCKSIZE,l2Cache,VC_NUM_BLOCKS);
    // Initialize Victim Cache if VC_NUM_BLOCKS > 0
    // VictimCache *l1VictimCache = nullptr;
    // if (VC_NUM_BLOCKS > 0) {
    //     l1VictimCache = new VictimCache(VC_NUM_BLOCKS, L1_BLOCKSIZE);
    // }

    // Initialize L2 Cache if L2_SIZE > 0
    
    // Read and process the trace file
    std::ifstream trace(trace_file);
    if (!trace) {
        std::cerr << "Error opening trace file: " << trace_file << "\n";
        return EXIT_FAILURE;
    }
    
    float L1accessTime,L1Energy,L1Area;
    std::string line;
    while (std::getline(trace, line)) {
        char type;
        std::string address_str;
        std::istringstream iss(line);
        iss >> type >> address_str;
        unsigned long address = std::stoull(address_str, nullptr, 16);
        
        if (type == 'r') {
            // Read operation
            l1Cache.handleRead(address);
           
        } else if (type == 'w') {
            // Write operation
            l1Cache.handleWrite(address);
            
        } else {
            std::cerr << "Invalid operation type: " << type << "\n";
            return EXIT_FAILURE;
        }
    }
    // get_cacti_results(L1_SIZE,L1_BLOCKSIZE,L1_ASSOC,&L1accessTime,&L1Energy,&L1Area);

    trace.close();
    std::cout<<"===== L1 contents =====\n";
    l1Cache.printContents();
    if(L2_SIZE)
    {
    std::cout<<"===== L2 contents =====\n";
    l2Cache->printContents();
    }
    if(VC_NUM_BLOCKS)
    {
    std::cout<<"===== VC contents =====\n";
    l1Cache.printVictimContents();
    }
    std::cout << "===== Simulation results (raw) =====\n";
        std::cout << "  a. number of L1 reads:\t\t\t" << l1Cache.numReads << "\n";
        std::cout << "  b. number of L1 read misses:\t\t\t" << l1Cache.numReadMisses << "\n";
        std::cout << "  c. number of L1 writes:\t\t\t" << l1Cache.numWrites << "\n";
        std::cout << "  d. number of L1 write misses:\t\t\t" << l1Cache.numWriteMisses << "\n";
        std::cout << "  e. number of swap requests:\t\t\t" << l1Cache.numSwaps << "\n";
        std::cout << "  f. swap request rate:\t\t\t\t" << std::fixed << std::setprecision(4) << (l1Cache.numReads + l1Cache.numWrites > 0 ? static_cast<double>(l1Cache.numSwaps) / (l1Cache.numReads + l1Cache.numWrites) : 0) << "\n";
        std::cout << "  g. number of swaps:\t\t\t\t" << l1Cache.numSwaps << "\n";
        std::cout << "  h. combined L1+VC miss rate:\t\t\t" << std::fixed << std::setprecision(4) << (l1Cache.numReads + l1Cache.numWrites > 0 ? static_cast<double>(l1Cache.numReadMisses + l1Cache.numWriteMisses - l1Cache.numSwaps) / (l1Cache.numReads + l1Cache.numWrites) : 0) << "\n";
        std::cout << "  i. number writebacks from L1/VC:\t\t" << l1Cache.numWritebacks << "\n";
        std::cout << "  j. number of L2 reads:\t\t\t" << (L2_SIZE?l2Cache->numReads:0) << "\n";
        std::cout << "  k. number of L2 read misses:\t\t\t" << (L2_SIZE?l2Cache->numReadMisses:0) << "\n";
        std::cout << "  l. number of L2 writes:\t\t\t" << (L2_SIZE?l2Cache->numWrites:0) << "\n";
        std::cout << "  m. number of L2 write misses:\t\t\t" <<(L2_SIZE?l2Cache->numWriteMisses:0) << "\n";
        std::cout << "  n. L2 miss rate:\t\t\t\t" << std::fixed << std::setprecision(4) << (L2_SIZE?(l2Cache->numReads > 0 ? static_cast<double>(l2Cache->numReadMisses) / l2Cache->numReads : 0):0) << "\n";
        std::cout << "  o. number of writebacks from L2:\t\t" << (L2_SIZE?l2Cache->numWritebacks :0) << "\n";
        std::cout << "  p. total memory traffic:\t\t\t" << (L2_SIZE?(l2Cache->numReads + l2Cache->numWrites + l2Cache->numWritebacks) :0)<< "\n";
    // Print statistics
    // if (VC_NUM_BLOCKS>0) l1Cache.printVictimStatistics();
    // if (l2Cache) l2Cache->printStatistics();
        
    std::cout<<"===== Simulation results (performance) =====\n" ;
    std::cout<<"1. average access time:\t\t\t"<<L1accessTime<<std::endl;
    std::cout<<"2. energy-delay product:\t\t\t"<<L1Energy<<std::endl;
    std::cout<<"3. total area:\t\t\t"<<L1Area<<std::endl;
    // Clean up
    // if (l1VictimCache) delete l1VictimCache;
    if (l2Cache) delete l2Cache;
    
    return 0;
}
