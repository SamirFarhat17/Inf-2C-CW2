/***************************************************************************
 * *    Inf2C-CS Coursework 2: Cache Simulation
 * *
 * *    Instructor: Boris Grot
 * *
 * *    TA: Siavash Katebzadeh
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>
#include <math.h>
#include <time.h>
/* Do not add any more header files */

/*
 * Various structures
 */
typedef enum {FIFO, LRU, Random} replacement_p;

const char* get_replacement_policy(uint32_t p) {
    switch(p) {
    case FIFO: return "FIFO";
    case LRU: return "LRU";
    case Random: return "Random";
    default: assert(0); return "";
    };
    return "";
}

typedef struct {
    uint32_t address;
} mem_access_t;

// These are statistics for the cache and should be maintained by you.
typedef struct {
    uint32_t cache_hits;
    uint32_t cache_misses;
} result_t;


/*
 * Parameters for the cache that will be populated by the provided code skeleton.
 */

replacement_p replacement_policy = FIFO;
uint32_t associativity = 0;
uint32_t number_of_cache_blocks = 0;
uint32_t cache_block_size = 0;


/*
 * Each of the variables below must be populated by you.
 */
uint32_t g_num_cache_tag_bits = 0;
uint32_t g_cache_offset_bits= 0;
result_t g_result;


/* Reads a memory access from the trace file and returns
 * 32-bit physical memory address
 */
mem_access_t read_transaction(FILE *ptr_file) {
    char buf[1002];
    char* token = NULL;
    char* string = buf;
    mem_access_t access;

    if (fgets(buf, 1000, ptr_file)!= NULL) {
        /* Get the address */
        token = strsep(&string, " \n");
        access.address = (uint32_t)strtoul(token, NULL, 16);
        return access;
    }

    /* If there are no more entries in the file return an address 0 */
    access.address = 0;
    return access;
}

void print_statistics(uint32_t num_cache_tag_bits, uint32_t cache_offset_bits, result_t* r) {
    /* Do Not Modify This Function */

    uint32_t cache_total_hits = r->cache_hits;
    uint32_t cache_total_misses = r->cache_misses;
    printf("CacheTagBits:%u\n", num_cache_tag_bits);
    printf("CacheOffsetBits:%u\n", cache_offset_bits);
    printf("Cache:hits:%u\n", r->cache_hits);
    printf("Cache:misses:%u\n", r->cache_misses);
    printf("Cache:hit-rate:%2.1f%%\n", cache_total_hits / (float)(cache_total_hits + cache_total_misses) * 100.0);
}

////////////////////////////////////////////////////////////////////////////////
/* Note: For all my comments the comment is related to what is occuring above it.*/
uint32_t index_number = 0;
//Initialize index number as 0 in case of conflicts.

uint32_t offset_bits(uint32_t cache_block_size) {
    uint32_t comparator = 1;
    // if 2^(comparator) is less than the size of the cache blocks the most significant digit may not have been found
    uint32_t most_significant_digit = 0;
    while (comparator < cache_block_size) {
        comparator *= 2;
        most_significant_digit++;
    //checks till comparator is larger than cache block size, once that happens the most significant digit has been found.
    }
    return most_significant_digit;

}
//Function to return number of offset bits.

uint32_t index_bits(uint32_t number_of_cache_blocks, uint32_t associativity) {
    uint32_t set_number = number_of_cache_blocks/associativity;
    uint32_t comparator = 1;
    uint32_t most_significant_digit = 0;
    while (comparator < set_number) {
        comparator *= 2;
        most_significant_digit++;
        //while trhe comparator is not larger than the number of sets the MSB has not been found.
    }
    return most_significant_digit;
}
//Function to return number of index bits.

uint32_t tag_bits(uint32_t offset_bits, uint32_t index_bits) {
    uint32_t tag_number = 32 - (offset_bits + index_bits);
    return tag_number;
}
//Function returning tag bits.

uint32_t extract_tag_bits(mem_access_t access){
    uint32_t address = access.address;
    uint32_t tag = address>>(g_cache_offset_bits + index_bits(number_of_cache_blocks, associativity));
    //right shift by all the bits except the tag bits, leaving only the tag.
    return tag;
}
//Function returning the tag bits.

uint32_t extract_index_bits(mem_access_t access) {
    uint32_t address = access.address;
    uint32_t mask = (1 << index_bits(number_of_cache_blocks, associativity))-1;
    //create mask for further manipulation, since index is between the tag and offset, more complicated procedure needed. Hence the mask.
    uint32_t right_shift = address >> g_cache_offset_bits;
    //right shift the mask by the offset to just leave remaining index bits.
    uint32_t index = right_shift &mask;
    return index;
}
//Function that returns the index bits.

////////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv) {
    time_t t;
    /* Intializes random number generator */
    /* Important: *DO NOT* call this function anywhere else. */
    srand((unsigned) time(&t));
    /* ----------------------------------------------------- */
    /* ----------------------------------------------------- */

    /*
     *
     * Read command-line parameters and initialize configuration variables.
     *
     */
    int improper_args = 0;
    char file[10000];
    if (argc < 6) {
        improper_args = 1;
        printf("Usage: ./mem_sim [replacement_policy: FIFO LRU Random] [associativity: 1 2 4 8 ...] [number_of_cache_blocks: 16 64 256 1024] [cache_block_size: 32 64] mem_trace.txt\n");
    } else  {
        /* argv[0] is program name, parameters start with argv[1] */
        if (strcmp(argv[1], "FIFO") == 0) {
            replacement_policy = FIFO;
        } else if (strcmp(argv[1], "LRU") == 0) {
            replacement_policy = LRU;
        } else if (strcmp(argv[1], "Random") == 0) {
            replacement_policy = Random;
        } else {
            improper_args = 1;
            printf("Usage: ./mem_sim [replacement_policy: FIFO LRU Random] [associativity: 1 2 4 8 ...] [number_of_cache_blocks: 16 64 256 1024] [cache_block_size: 32 64] mem_trace.txt\n");
        }
        associativity = atoi(argv[2]);
        number_of_cache_blocks = atoi(argv[3]);
        cache_block_size = atoi(argv[4]);
        strcpy(file, argv[5]);
    }
    if (improper_args) {
        exit(-1);
    }
    assert(number_of_cache_blocks == 16 || number_of_cache_blocks == 64 || number_of_cache_blocks == 256 || number_of_cache_blocks == 1024);
    assert(cache_block_size == 32 || cache_block_size == 64);
    assert(number_of_cache_blocks >= associativity);
    assert(associativity >= 1);

    printf("input:trace_file: %s\n", file);
    printf("input:replacement_policy: %s\n", get_replacement_policy(replacement_policy));
    printf("input:associativity: %u\n", associativity);
    printf("input:number_of_cache_blocks: %u\n", number_of_cache_blocks);
    printf("input:cache_block_size: %u\n", cache_block_size);
    printf("\n");

    /* Open the file mem_trace.txt to read memory accesses */
    FILE *ptr_file;
    ptr_file = fopen(file,"r");
    if (!ptr_file) {
        printf("Unable to open the trace file: %s\n", file);
        exit(-1);
    }

    /* result structure is initialized for you. */
    memset(&g_result, 0, sizeof(result_t));

    /* Do not delete any of the lines below.
     * Use the following snippet and add your code to finish the task. */
////////////////////////////////////////////////////////////////////////////////
     g_cache_offset_bits = offset_bits(cache_block_size);
     uint32_t index_number = index_bits(number_of_cache_blocks, associativity);
     g_num_cache_tag_bits = tag_bits(g_cache_offset_bits, index_number);
     //Use previous methods to return the values for later manipulation.

typedef struct
  {
    int valid;
    uint32_t tag;
    int timestamp;
  } Block;
  //Structure of block, with useful fields.

typedef struct
  {
    Block *block;
  } CacheSet;
  //Structure of a set, which points to the blocks in them.

typedef struct
     {
        CacheSet *sets;
     } CacheStructure;
  //Structure of the whole chace, which points to its sets.

CacheStructure CacheMoney;
//Initialize instance of Cache.

uint32_t NumberOfSets = number_of_cache_blocks/associativity;
uint32_t BlocksPerSet = number_of_cache_blocks/NumberOfSets;
//Initialize the blocks per set and the amount of set

CacheMoney.sets = (CacheSet*)malloc(NumberOfSets * sizeof(CacheSet));
//dynamically allocate data for sets using malloc.
    for ( int i = 0; i < NumberOfSets; i++)
     {
       CacheMoney.sets[i].block = (Block*)malloc( sizeof(Block) * BlocksPerSet);
       //dynamically allocate data for blocks using malloc.
     }

for(int i = 0; i < number_of_cache_blocks/associativity; i++) {
    for (int j = 0; j < associativity; j ++) {
        CacheMoney.sets[i].block[j].valid = 0;
        CacheMoney.sets[i].block[j].timestamp = 0;
    }
}
//Set valid bits and timestamps of all blocks to 0.

///////////////////////////////////////////////////////////////////////////////
    mem_access_t access;
    /* Loop until the whole trace file has been read. */

    while(1) {
        access = read_transaction(ptr_file);
        // If no transactions left, break out of loop.
        if (access.address == 0)
            break;
///////////////////////////////////////////////////////////////////////////////

        uint32_t index =  extract_index_bits(access);
        uint32_t tag =  extract_tag_bits(access);
        //retrieves, index bits, and tag bits for analysis.

        int is_found = 0;
        int out_of_space = 1;
        //Initialize useful variables for analysis.

        if (replacement_policy == FIFO) {
            CacheSet set = CacheMoney.sets[index];
            //Initialize instance of set.
            for (int i = 0; i < associativity; i++){
                Block x = set.block[i];
                //Initialize every block of set.
                if(x.tag == tag) {
                //check if block matches tag.
                    is_found = 1;
                    g_result.cache_hits++;
                    //if tag matches add a hit.
                    for (int j = 0; j < associativity; j++) {
                        Block y = set.block[j];
                        if(y.valid) {
                            y.timestamp = y.timestamp + 1;
                        }
                        //Update all timestamps

                  }
                  break;
                }
            }
            if (!is_found) {
            //check if a matching tag hasn't been found.
                g_result.cache_misses++;
                //If not add a miss.
                for (int i = 0; i < associativity; i++) {
                    if (!set.block[i].valid) {
                    //check for available tags.
                    set.block[i].tag = tag;
                    set.block[i].valid = 1;
                        //update tag and validity accordingly.
                        for (int j = 0; j < associativity; j++) {
                                if(set.block[j].valid) {
                                  set.block[j].timestamp = set.block[j].timestamp + 1;
                                  //update all timestamps.
                              }
                            }
                        set.block[i].timestamp = 0;
                        //reset timestamp of block that has had data added to it.
                        out_of_space = 0;
                        //still space in cache.
                        break;
                    }
                }
                if(out_of_space) {
                //if no space is available.
                    int largest_timestamp = 0;
                    for (int i = 0; i < associativity; i++) {
                        if(largest_timestamp < set.block[i].timestamp) {
                                largest_timestamp = set.block[i].timestamp;
                        }
                    }
                    //algorithm to find largest timestamp.
                    for (int i = 0; i < associativity; i ++) {
                        if (largest_timestamp == set.block[i].timestamp) {
                            for (int j = 0; j < associativity; j++) {
                                if(set.block[j].valid) {
                                    set.block[j].timestamp = set.block[j].timestamp + 1;
                                    //update all timestamps.
                                }
                            }
                            set.block[i].timestamp = 0;
                            set.block[i].tag  = tag;
                            //block with largest timestamp gets replaced and has its timestamp reset.
                            break;
                        }
                    }
                }
            }
          }

//////////////////////////////////////////////////////////////////////////////
        if (replacement_policy == LRU) {
            CacheSet set = CacheMoney.sets[index];
            //Initialize instance of set.
            for (int i = 0; i < associativity; i++){
                Block x = set.block[i];
                //initialize every block of set.
                if(x.tag == tag) {
                //if tag is found.
                    is_found = 1;
                    for (int j = 0; j < associativity; j++) {
                        Block y = set.block[j];
                        if(y.valid) {
                            set.block[j].timestamp = set.block[j].timestamp + 1;
                        }
                    }
                    //update all timestamps.
                    set.block[i].timestamp = 0;
                    //set hit block to timestamp 0.
                    g_result.cache_hits++;
                    //tag has been found, hence hit.
                    break;
                }
            }
            if (!is_found) {
            //tag wasn't found.
                g_result.cache_misses++;
                //thus add a miss.
                for (int i = 0; i < associativity; i++) {
                    if (!set.block[i].valid) {
                    //find available space
                        for (int j = 0; j < associativity; j++) {
                                if(set.block[j].valid) {
                                  set.block[j].timestamp = set.block[j].timestamp + 1;
                              }
                            }
                        //update all timestamps accordingly.
                        set.block[i].tag = tag;
                        set.block[i].valid = 1;
                        set.block[i].timestamp = 0;
                        out_of_space = 0;
                        //load tag into block, set timestamp to 0, and change valid bit to show unavailability.
                        break;
                    }
                }
                if(out_of_space) {
                //if cache is full
                    int largest_timestamp = 0;
                    int index = 0;
                    for (int i = 0; i < associativity; i++) {
                        if(largest_timestamp < set.block[i].timestamp) {
                                largest_timestamp = set.block[i].timestamp;
                                index = i;
                        }
                    }
                    //find largest timestamp.
                    //index indicates which block has largest timestamp.
                    for (int j = 0; j < associativity; j++) {
                        if(set.block[j].valid) {
                            set.block[j].timestamp = set.block[j].timestamp + 1;
                        }
                    }
                    //update other blocks accordingly
                    set.block[index].timestamp = 0;
                    set.block[index].tag  = tag;
                    //replace tag and reset timestamp.
                }
            }
          }

///////////////////////////////////////////////////////////////////////////////
        if (replacement_policy == Random) {
            CacheSet set = CacheMoney.sets[index];
            //Initialize instance of set.
            for (int i = 0; i < associativity; i++){
                Block x = set.block[i];
                //Initialize every block in the set.
                if(x.tag == tag) {
                  is_found = 1;
                  g_result.cache_hits++;
                  //If matching tag is found add a hit.
                  break;
                }
            }
            if (!is_found) {
            //check if tag hasn't been found.
                g_result.cache_misses++;
                //if not add a miss.
                for (int i = 0; i < associativity; i++) {
                    if (!set.block[i].valid) {
                        //if an available block is found
                        out_of_space = 0;
                        set.block[i].tag = tag;
                        set.block[i].valid = 1;
                        out_of_space = 0;
                        //add tag, block is now unavailable, and there is still space in cache
                    }
              }
                if(out_of_space) {
                //check if space is not available in cache.
                  int to_be_replaced = rand()%associativity;
                  //if not select random integer within the associativity range.
                  set.block[to_be_replaced].tag = tag;
                  //replace block corresponding to that integer.
                }
            }
        }
    /*    for (int i = 0; i < NumberOfSets; i++) {
            printf("Set %d\n", i);
            CacheSet set = CacheMoney.sets[i];
            for (int j = 0; j < associativity; j++) {
                printf(" tag: %x      timestamp:%d\n",set.block[j].tag, set.block[j].timestamp);
            }
        }
        printf("------------------------------------------------------");*/ //prints cache (for testing and such)
    }
    for ( int i = 0; i < NumberOfSets; i++)
     {
       free (CacheMoney.sets[i].block) ;
       // free memory once program exits
     }
    free (CacheMoney.sets);
    //free things in reverse order to how they were dynamically allocated.

    /* Do not modify code below. */
    /* Make sure that all the parameters are appropriately populated. */
    print_statistics(g_num_cache_tag_bits, g_cache_offset_bits, &g_result);

    /* Close the trace file. */
    fclose(ptr_file);
    return 0;
}
