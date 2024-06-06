#ifndef _SHT_H_
#define _SHT_H_

#include "BF.h"
#include "HT.h"
#include "Record.h"

/* We define the 'SHT_info' structure. We will
 * use this structure to describe secondary hash files
 */

typedef struct
{
    int fileDesc;
    char *attrName;
    int attrLength;
    int numBuckets;
    char *fileName;

} SHT_info;

/* The Interface of Secondary Hash file Management */

int SHT_CreateSecondaryIndex(char *sfileName, char *attrName, int attrLength, int buckets, char *fileName);
SHT_info *SHT_OpenSecondaryIndex(char *sfileName);
int SHT_CloseSecondaryIndex(SHT_info *header_info);
int SHT_SecondaryInsertEntry(SHT_info header_info, SecondaryRecord record);
int SHT_SecondaryGetAllEntries(SHT_info header_info_sht, HT_info header_info_ht, void *value);
int SecondaryHashStatistics(char *filename);

#endif /* _SHT_H_ */