#ifndef _HP_H_
#define _HP_H_

#include "BF.h"
#include "Record.h"

/* We define the 'HP_info' structure. We will
 * use this structure to describe heap files
 */

typedef struct
{
    int fileDesc;
    char attrType;
    char *attrName;
    int attrLength;

} HP_info;

/* The Interface of Heap file Management */

int HP_CreateFile(char *fileName, char attrType, char *attrName, int attrLength);
HP_info *HP_OpenFile(char *fileName);
int HP_CloseFile(HP_info *header_info);
int HP_InsertEntry(HP_info header_info, Record record);
int HP_DeleteEntry(HP_info header_info, void *value);
int HP_GetAllEntries(HP_info header_info, void *value);

#endif /* _HP_H_ */
