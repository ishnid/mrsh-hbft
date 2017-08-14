/* 
 * File:   config.h
 *
 * Created on 1. Mai 2013, 12:15
 */

#ifndef MAIN_H
#define	MAIN_H

#include "../header/config.h"
#include "../header/hashing.h"
#include "../header/bloomfilter.h"

//FILE    *getFileHandle(char *filename);
void calculate_size(int filesize);
void evaluation(char *filename, int *results);
void comparePathToBloomfilter(BLOOMFILTER *bf, char *filename);
void addPathToBloomfilter(BLOOMFILTER *bf, char *filename);

#endif	/* MAIN_H */

