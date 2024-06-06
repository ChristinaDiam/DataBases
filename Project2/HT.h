#ifndef _HT_H_
#define _HT_H_

#include "BF.h"
#include "Record.h"

/* We define the 'HT_info' structure. We will
 * use this structure to describe hash files
 */

typedef struct
{
    int fileDesc;
    char attrType;
    char *attrName;
    int attrLength;
    int numBuckets;

} HT_info;

/* The Interface of Hash file Management */

int HT_CreateIndex(char *fileName, char attrType, char *attrName, int attrLength, int buckets);
HT_info *HT_OpenIndex(char *fileName);
int HT_CloseIndex(HT_info *header_info);
int HT_InsertEntry(HT_info header_info, Record record);
int HT_DeleteEntry(HT_info header_info, void *value);
int HT_GetAllEntries(HT_info header_info, void *value);
int HashStatistics(char *filename);

#endif /* _HT_H_ */