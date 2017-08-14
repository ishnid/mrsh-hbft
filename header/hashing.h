/* 
 * File:   hashing.h
 *
 * Created on 5. Juni 2012, 13:22
 */

#ifndef HASHING_H
#define	HASHING_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "bloomfilter.h"
#include "helper.h"
#include "filehash.h"


int 		*hashAndDo(BLOOMFILTER *bf, FILE_CONTENTS *fc, short fill);
int 		*hashPacketBuffer(BLOOMFILTER *bf, const unsigned char *packet, const size_t length);
int         hashFileToBF(BLOOMFILTER *bf, FILE *handle);
int*        hashFileAndCompare(BLOOMFILTER *bf, FILE *handle);
double 		entropy(int freqArray[], int size);
void 		createResultsSummary(BLOOMFILTER *bf, uint256 hash_val, int *results_summary);

uint32      roll_hashx(unsigned char c, uchar window[], uint32 rhData[]);

#endif	/* HASHING_H */

void add_file_hash_to_bf(BLOOMFILTER *, FILE_HASH *);

FILE_HASH *hash_file(FILE_CONTENTS *);