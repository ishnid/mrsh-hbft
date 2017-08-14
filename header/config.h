/* 
 * File:   config.h
 *
 * Created on 1. Mai 2012, 12:15
 */

#ifndef CONFIG_H
#define	CONFIG_H

#define MAX_BF_SIZE_IN_BYTES 1073741824

// define LOGGING to enable logging information (printed to stdout)
#define LOGGING                 // enable logging (relatively verbose!)
//#define FIXED_TREE              // enable fixed-sized tree (variable otherwise)
#define FINGERPRINT_LEAVES      // enable/disable calculation of fingerprints at leaves and linear searching.

#define ROLLING_WINDOW          7
//#define BLOCK_SIZE              320 // mrsh_net is 64, mrsh_v2 is 160
#define SKIPPED_BYTES           mode->block_size/3 // mrsh_net is BLOCK_SIZE/3, mrsh_v2 is BLOCK_SIZE/4
#define MIN_ENTROPY				0

//#define MIN_RUN					16 // mrsh_net is 8

// TODO: this should not be necessary long-term.
#define BF_SIZE_IN_BYTES		5242880 // 41943040 //33554432 //16384	//Filte 32 MiB
#define SUBHASHES               5

#define SKIP_FIRST				1	//Skip first block which often contains header info
#define SKIP_LAST				1   //Skip last block which often contains footer info

#define AUTO_BUILD_BF_TREE      1  // automatically build BF tree as soon as search() is called

// From mrsh_v2 for fingerprinting. Values not changed.
#define PROBABILITY             0.99951172 //Attention: 1 - ( 1/BLOOMFILTERBITSIZE )
#define MAXBLOCKS               160
#define MINBLOCKS				8
// End mrsh_v2 constants

#define THRESHOLD               1   // how similar do fingerprints need to be?

typedef unsigned long long  uint64; 
typedef unsigned char       uchar;
typedef unsigned int        uint32;
typedef unsigned short      ushort16;
typedef unsigned int		uint256[8];
typedef unsigned long long  uint256r[5];

typedef short bool;
#define true 1
#define false 0

typedef struct{
    bool generateBF;
    bool readDB;
    bool helpmessage;
    bool recursive;
    bool calc_size;
    bool addToDB;
    bool combine;
    int min_run;
    int block_size;
    bool fixed;
} MODES;

/*typedef struct {
    unsigned int foo[8];
}uint256;*/

#define MIN(a,b) (a < b ? a : b)

extern MODES *mode; //= {.compare = false}


#endif	/* CONFIG_H */

