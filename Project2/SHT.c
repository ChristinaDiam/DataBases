#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SHT.h"

/***********************
 * Prints a record 'r' *
 ***********************/

static void printRecord(Record r)
{
	printf("{%d, %s, %s, %s}\n", r.id, r.name, r.surname, r.address);
}

/*****************************
 * Function to hash a string *
 *****************************/

static int hashStrings(char *key, int numOfBuckets)
{
    unsigned int h = 0, a = 127;

    for(; *key != '\0'; key++)
        h = a * h + *key;

    return (h % numOfBuckets) + 1;
}

/*********************************
 * Creates a secondary Hash File *
 *********************************/

int SHT_CreateSecondaryIndex(char *sfileName, char *attrName, int attrLength, int buckets, char *fileName)
{
	/* We create a new Block file */

	int create_result = BF_CreateFile(sfileName);

	if(create_result < 0)
	{
		BF_PrintError("Error creating file");
		return -1;
	}

	/* We open the Block file we created */

	int fileDesc = BF_OpenFile(sfileName);

	if(fileDesc < 0)
	{
		BF_PrintError("Error opening file");
		return -1;
	}

	/* We allocate a new block */

	int allocation_result = BF_AllocateBlock(fileDesc);

	if(allocation_result < 0)
	{
		BF_PrintError("Error allocating block");
		return -1;
	}

	/* Pointer to the block we just allocated */

	void *block;

	/* We read the first block (block '0') */

	int read_result = BF_ReadBlock(fileDesc, 0, &block);

	if(read_result < 0)
	{
		BF_PrintError("Error reading block");
		return -1;
	}
	
	/* We save the information we need in the block we
	 * retrieved. This will be information about the file
	 *
	 * We save the type of the file (this is a hash file)
	 */

	memcpy(block, "SecondaryHash" , 14);

	/* We save the length of the key */

	memcpy(block + 14, &attrLength, sizeof(int));

	/* We save the content of the key */

	memcpy(block + 14 + sizeof(int), attrName, attrLength);

	/* We save the number of buckets */

	memcpy(block + 14 + sizeof(int) + attrLength, &buckets, sizeof(int));

	int fileLength = strlen(fileName);
	memcpy(block + 14 + sizeof(int) + attrLength + sizeof(int), &fileLength, sizeof(int));

	/* We save the name of the primary Hash File */

	memcpy(block + 14 + sizeof(int) + attrLength + sizeof(int) + sizeof(int), fileName, fileLength);

	/* We save the block in the disk */

	int write_result = BF_WriteBlock(fileDesc, 0);

	if(write_result < 0)
	{
		BF_PrintError("Error writing block");
		return -1;
	}

	/* Now we allocate as many blocks as the number of buckets */

	while(buckets--)
	{
		/* We allocate a new block */

		allocation_result = BF_AllocateBlock(fileDesc);

		if(allocation_result < 0)
		{
			BF_PrintError("Error allocating block");
			return -1;
		}

		/* We retrieve the number of blocks */

		int blocksNum = BF_GetBlockCounter(fileDesc);

		if(blocksNum < 0)
		{
			BF_PrintError("Error in block counter");
			return -1;
		}

		/* We read the new block */

		read_result = BF_ReadBlock(fileDesc, blocksNum - 1, &block);

		if(read_result < 0)
		{
			BF_PrintError("Error reading block");
			return -1;
		}

		/* We create a variable that indicates the number of records
		 * saved in the block. This will be the 'numOfRecords' variable
		 *
		 * We also create a variable that indicates to which block the current block is linked,
		 * so as when the current block is full of records we save a record in the linked block.
		 * This will be the 'nextBlock' variable, initialized to block zero. (Block '0' of course
		 * is not an appropriate block, but it will be changed in 'HT_InsertEntry' when the block
		 * is full of records. Also, when the linked block of a block is the block '0', that
		 * indicates there is no linked block)
		 */

		int numOfRecords = 0;
		int nextBlock = 0;

		/* We copy the content of 'numOfRecords' and 'nextBlock' to the block. The
		 * quantity of saved records in the block will be the first data in the block
		 * and the ID of the linked block will be the second
		 */

		memcpy(block, &numOfRecords, sizeof(int));
		memcpy(block + sizeof(int), &nextBlock, sizeof(int));

		/* We save the block in the disk */

		write_result = BF_WriteBlock(fileDesc, blocksNum - 1);

		if(write_result < 0)
		{
			BF_PrintError("Error writing block");
			return -1;
		}
	}

	/* Finally, we close the block file */

	int close_result = BF_CloseFile(fileDesc);

	if(close_result < 0)
	{
		BF_PrintError("Error closing file");
		return -1;
	}

	/* If this code is reached, everything
	 * was successful, so we return 0
	 */

	return 0;
}

/*******************************
 * Opens a secondary Hash File *
 *******************************/

SHT_info *SHT_OpenSecondaryIndex(char *sfileName)
{
	/* String which will save the type of the file with name 'fileName' */

	char typeOfFile[13];

	/* We allocate memory for the 'HT_info' structure */

    SHT_info *sht_info = (SHT_info *) malloc(sizeof(SHT_info));

    /* Opening the file */

	sht_info->fileDesc = BF_OpenFile(sfileName);

	if(sht_info->fileDesc < 0)
	{
		BF_PrintError("Error opening file");
		free(sht_info);
		return NULL;
	}

	/* Pointer to the block we will read */

	void *block;

	/* We read the first block (block '0') */

	int read_result = BF_ReadBlock(sht_info->fileDesc, 0, &block);

	if(read_result < 0)
	{
		BF_PrintError("Error reading block");
		free(sht_info);
		return NULL;
	}

	/* We read the type of the file */

	memcpy(typeOfFile, block, 14);
	typeOfFile[14] = '\0';

	/* We compare the type of the file with the type "SecondaryHash".
	 * If the strings are not same, this is not a secondary hash file
	 */

	if(strcmp(typeOfFile, "SecondaryHash"))
	{
		printf("This is not a secondary hash file!\n");
		free(sht_info);
		return NULL;
	}

	/* We read the length of the key */

	memcpy(&(sht_info->attrLength), block + 14, sizeof(int));

	/* We allocate memory to save the content of the key */

	sht_info->attrName = malloc(sht_info->attrLength + 1);

	/* We read the content of the key */

	memcpy(sht_info->attrName, block + 14 + sizeof(int), sht_info->attrLength);
	sht_info->attrName[sht_info->attrLength] = '\0';

	/* We read the number of buckets */

	memcpy(&(sht_info->numBuckets), block + 14 + sizeof(int) + sht_info->attrLength, sizeof(int));

	/* We read the length of the name of the primary Hash File */

	int fileLength;
	memcpy(&fileLength, block + 14 + sizeof(int) + sht_info->attrLength + sizeof(int), sizeof(int));

	/* We allocate memory to save the name of the file */

	sht_info->fileName = malloc(fileLength + 1);

	/* We read the name of the primary Hash File */

	memcpy(sht_info->fileName, block + 14 + sizeof(int) + sht_info->attrLength + sizeof(int) + sizeof(int), fileLength);
	sht_info->fileName[fileLength] = '\0';

    return sht_info;
}

/********************************
 * Closes a secondary Hash File *
 ********************************/

int SHT_CloseSecondaryIndex(SHT_info *header_info)
{
	/* If the pointer is 'NULL', we return -1 immediatelly */

	if(header_info == NULL)
		return -1;

	/* We close the file */

	int close_result = BF_CloseFile(header_info->fileDesc);

	if(close_result < 0)
	{
		BF_PrintError("File was not closed successfully");
		return -1;
	}

	/* We free the allocated memory for the 'HT_info' structure */

    free(header_info->attrName);
    free(header_info->fileName);
    free(header_info);

    return 0;
}

/***************************************************************************************************
 * Inserts a secondary record to the secondary Hash File designated by the structure 'header_info' *
 ***************************************************************************************************/

int SHT_SecondaryInsertEntry(SHT_info header_info, SecondaryRecord record)
{
	/* Each record of the secondary hash file is a reference to a struct 'Record' that
	 *                                             ^^^^^^^^^
	 * is saved in the primary hash file. The reference consists of two integers, one
	 * indicates the block ID of the primary hash file where the record is saved and
	 *           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	 * another indicates the position of that record in the block. For example, the
	 *                   ^^^^^^^^^^^^
	 * reference {5, 3} indicates that a saved record is the 3rd record of the 5th
	 * block of the primary hash file. If a record reference may be represented as
	 * {Xi, Xj}, then the variable 'blockPos' below is 'Xi' and 'recordPos' is 'Xj'
	 */

	int blockPos = record.blockId;
	int recordPos = 0;

	/* First, we have to open the primary hash file to access
	 * the given block (with block ID equal to 'blockPos')
	 */

	HT_info *primary_info = HT_OpenIndex(header_info.fileName);

	if(primary_info == NULL)
	{
		printf("Could not retrieve primary file info\n");
		return -1;
	}

	/* We read the block with ID 'blockId' */

	void *block;
	int read_result = BF_ReadBlock(primary_info->fileDesc, blockPos, &block);

	if(read_result < 0)
	{
		BF_PrintError("Error reading block");
		return -1;
	}

	/* We copy the number of records of that block to 'recordPos'.
	 *         ^^^^^^^^^^^^^^^^^^^^^
	 * The logic behind this is the fact that the most recent entry
	 * in the primary hash file is also the last entry in the block
	 * and, arithmetically, its position in the block (which is the
	 * desired information for us) is equal to the number of records
	 * of that block
	 */

	memcpy(&recordPos, block, sizeof(int));

	/* Since we retrieved the position of the record, we do not
	 * need the primary hash file anymore, so we close it
	 */

	int close_result = HT_CloseIndex(primary_info);

	if(close_result < 0)
	{
		printf("Could not close the primary Hash File\n");
		return -1;
	}

	/* Now the reference {Xi, Xj} has been formed and
	 * we need to save it in the secondary hash file
	 *
	 * Initially, we hash the surname of the record
	 * and get the bucket where we have to store it
	 *
	 * 'lastBlock' will represent the previous value of 'keyBlock'
	 */

	int keyBlock = hashStrings(record.record.surname, header_info.numBuckets);
	int lastBlock = keyBlock;

	/* 'keyBlock' is initially the ID of the first block in the bucket. If it
	 * has enough capacity to store the reference, we store it there, else we
	 * go to the linked block (the next block of the bucket) and examine in the
	 * same way if there is enough capacity. If there is no block in the bucket
	 * that has adequate capacity, we have to allocate a new block and link it
	 */

	while(keyBlock != 0)
	{
		/* Here we access the block with ID 'keyBlock' */

		read_result = BF_ReadBlock(header_info.fileDesc, keyBlock, &block);

		if(read_result < 0)
		{
			BF_PrintError("Error reading block");
			return -1;
		}

		/* We retrieve the number of records of the block with ID 'keyBlock' */

		int numOfRecords;
		memcpy(&numOfRecords, block, sizeof(int));

		/* Case 1: There is enough space in 'keyBlock'
		 * ^^^^^^
		 * so as to save the desired record reference
		 */

		if(numOfRecords < (BLOCK_SIZE - 2 * sizeof(int)) / (2 * sizeof(int)))
		{
			/* We store the information of the record reference to the 'keyBlock' */

			memcpy(block + 2 * sizeof(int) + numOfRecords * 2 * sizeof(int), &blockPos, sizeof(int));
			memcpy(block + 2 * sizeof(int) + numOfRecords * 2 * sizeof(int) + sizeof(int), &recordPos, sizeof(int));

			/* We increase the number of saved records by 1 */

			numOfRecords++;
			memcpy(block, &numOfRecords, sizeof(int));

			/* We save the block in the disk */

			int write_result = BF_WriteBlock(header_info.fileDesc, keyBlock);

			if(write_result < 0)
			{
				BF_PrintError("Error writing block");
				return -1;
			}

			/* Since we applied the insertion, at this point we return '0' */

			return 0;
		}

		/* Case 2: There is not enough space in 'keyBlock'
		 * ^^^^^^           ^^^
		 * to save the desired record reference
		 */

		/* We save the current value of 'keyBlock' and update 'keyBlock' with the linked block */

		lastBlock = keyBlock;
		memcpy(&keyBlock, block + sizeof(int), sizeof(int));
	}

	/* If this part is reached, there is no block in the bucket with enough capacity.
	 *
	 * Here we have to allocate a new block, link it to the last
	 *                                       ^^^^
	 * block of the bucket and save the record in the new block
	 *                         ^^^^
	 */

	int allocation_result = BF_AllocateBlock(header_info.fileDesc);

	if(allocation_result < 0)
	{
		BF_PrintError("Error allocating block");
		return -1;
	}

	/* The allocated block is the last block of the file, so
	 * we find its ID with the help of 'BF_GetBlockCounter'
	 */

	int blocksNum = BF_GetBlockCounter(header_info.fileDesc);

	if(blocksNum < 0)
	{
		BF_PrintError("Error in block counter");
		return -1;
	}

	/* We link the allocated block to the last block of the bucket */

	int linkedBlock = blocksNum - 1;
	memcpy(block + sizeof(int), &linkedBlock, sizeof(int));

	/* Since we changed its content (specifically its linked block), we
	 * save the previous block in the disk (the last one with no capacity)
	 */

	int write_result = BF_WriteBlock(header_info.fileDesc, lastBlock);

	if(write_result < 0)
	{
		BF_PrintError("Error writing block");
		return -1;
	}

	/* We read the allocated block */

	void *new_block;
	read_result = BF_ReadBlock(header_info.fileDesc, blocksNum - 1, &new_block);

	if(read_result < 0)
	{
		BF_PrintError("Error reading block");
		return -1;
	}

	/* We initialize the two attributes of the new block:
	 * - The number of records is 1, because we will store the current reference
	 * - The linked block is 0, as there is no linked block for the allocated block
	 */

	int numOfRecords = 1;
	int nextBlock = 0;

	/* Here we save these two attributes (in the first 8 bytes) */

	memcpy(new_block, &numOfRecords, sizeof(int));
	memcpy(new_block + sizeof(int), &nextBlock, sizeof(int));

	/* Here we save the current reference (which
	 * will be the first reference in this block)
	 */

	memcpy(new_block + sizeof(int) + sizeof(int), &blockPos, sizeof(int));
	memcpy(new_block + sizeof(int) + sizeof(int) + sizeof(int), &recordPos, sizeof(int));

	/* We save the recently allocated block in the disk */

	write_result = BF_WriteBlock(header_info.fileDesc, linkedBlock);

	if(write_result < 0)
	{
		BF_PrintError("Error writing block");
		return -1;
	}

	/* At this point the insertion is complete and we return '0' */

	return 0;
}

/**********************************************************
 * Prints all records with key 'value' from the secondary *
 *   Hash file designated by the structure 'header_info'  *
 **********************************************************/

int SHT_SecondaryGetAllEntries(SHT_info header_info_sht, HT_info header_info_ht, void *value)
{
	/* If the 'value' is 'NULL', we print all entries from the primary hash file */

	if(value == NULL)
		return HT_GetAllEntries(header_info_ht, NULL);

	/* We hash the given 'value' to retrieve the desired bucket for search.
	 * Also, we initialize a block counter to value '0'
	 */

	int keyBlock = hashStrings((char *) value, header_info_sht.numBuckets);
	int counter = 0;

	/* As long as we have not reached the end of the bucket */

	while(keyBlock != 0)
	{
		/* Here we access the block with ID 'keyBlock' */

		void *block;
		int read_result = BF_ReadBlock(header_info_sht.fileDesc, keyBlock, &block);

		if(read_result < 0)
		{
			BF_PrintError("Error reading block");
			return -1;
		}

		/* We increase the counter by 1, since we read a new block */

		counter++;

		/* We retrieve the number of records in 'keyBlock' */

		int recordsInBlock;
		memcpy(&recordsInBlock, block, sizeof(int));

		int i;
		for(i = 0; i < recordsInBlock; i++)
		{
			/* We retrieve the current (ith) record of 'keyBlock' */

			int blockPos;
			int recordPos;

			memcpy(&blockPos, block + 2 * sizeof(int) + i * 2 * sizeof(int), sizeof(int));
			memcpy(&recordPos, block + 2 * sizeof(int) + i * 2 * sizeof(int) + sizeof(int), sizeof(int));

			/* We read the block with ID 'blockPos' from the primary hash file */

			read_result = BF_ReadBlock(header_info_ht.fileDesc, blockPos, &block);

			if(read_result < 0)
			{
				BF_PrintError("Error reading block");
				return -1;
			}

			/* We retrieve the appropriate record from that block */

			Record key_record;

			memcpy(&(key_record.id), block + 2 * sizeof(int) +
				(recordPos - 1) * (sizeof(int) + NAME_SIZE + SURNAME_SIZE + ADDRESS_SIZE), sizeof(int));

			memcpy(key_record.name, block + 2 * sizeof(int) +
				(recordPos - 1) * (sizeof(int) + NAME_SIZE + SURNAME_SIZE + ADDRESS_SIZE) + sizeof(int), NAME_SIZE);

			memcpy(key_record.surname, block + 2 * sizeof(int) +
				(recordPos - 1) * (sizeof(int) + NAME_SIZE + SURNAME_SIZE + ADDRESS_SIZE) + sizeof(int) + NAME_SIZE, SURNAME_SIZE);

			memcpy(key_record.address, block + 2 * sizeof(int) + (recordPos - 1) * (sizeof(int) +
				NAME_SIZE + SURNAME_SIZE + ADDRESS_SIZE) + sizeof(int) + NAME_SIZE + SURNAME_SIZE, ADDRESS_SIZE);

			/* If the given 'value' is equal to the 'surname' field of the acquired record, we print it */

			if(!strcmp(key_record.surname, (char *) value))
				printRecord(key_record);

			/* We access the block with ID 'keyBlock' again, since it was overwritten by the previous read */

			read_result = BF_ReadBlock(header_info_sht.fileDesc, keyBlock, &block);

			/* We read the 'keyBlock' again to refresh the 'block' pointer */

			if(read_result < 0)
			{
				BF_PrintError("Error reading block");
				return -1;
			}
		}

		/* We repeat the process with the linked block */

		memcpy(&keyBlock, block + sizeof(int), sizeof(int));
	}

	/* Finally, we return the number of read blocks */

	return counter;
}

/******************************************************************************************
 * Returns the number of records in a bucket that starts from the block with ID 'blockID' *
 ******************************************************************************************/

static unsigned int numRecords(SHT_info *header_info, int blockID)
{
	/* If the given 'blockID' is zero, there are no records */

	if(blockID == 0)
		return 0;

	/* Pointer to the block we will read */

	void *block;

	/* We read the block with ID 'blockID' */

	int read_result = BF_ReadBlock(header_info->fileDesc, blockID, &block);

	if(read_result < 0)
	{
		BF_PrintError("Error reading block");
		return -1;
	}

	/* We retrieve the number of records and the ID of the linked block */

	int recordsInBlock;
	int linkedBlock;

	memcpy(&recordsInBlock, block, sizeof(int));
	memcpy(&linkedBlock, block + sizeof(int), sizeof(int));

	/* We return the number of records in the current block plus
	 * the number of records in the rest of the linked blocks
	 */

	return recordsInBlock + numRecords(header_info, linkedBlock);
}

/*****************************************************************************************
 * Returns the number of blocks in a bucket that starts from the block with ID 'blockID' *
 *****************************************************************************************/

static unsigned int blocksNumber(SHT_info *header_info, int blockID)
{
	/* If the given 'blockID' is zero, it does not count */

	if(blockID == 0)
		return 0;

	/* Pointer to the block we will read */

	void *block;

	/* We read the block with ID 'blockID' */

	int read_result = BF_ReadBlock(header_info->fileDesc, blockID, &block);

	if(read_result < 0)
	{
		BF_PrintError("Error reading block");
		return -1;
	}

	/* We retrieve the linked block */

	int linkedBlock;
	memcpy(&linkedBlock, block + sizeof(int), sizeof(int));

	/* We return '1' for the current block and the quantity of the rest linked blocks */

	return 1 + blocksNumber(header_info, linkedBlock);
}

/******************************************************************************
 * Prints the hash statistics of the Secondary Hash file with name 'filename' *
 ******************************************************************************/

int SecondaryHashStatistics(char *filename)
{
	/* Firts, we open the file */

	SHT_info *header_info = SHT_OpenSecondaryIndex(filename);

	/* We retrieve the number of blocks in the file */

	int numOfBlocks = BF_GetBlockCounter(header_info->fileDesc);

	if(numOfBlocks < 0)
	{
		BF_PrintError("Error in block counter");
		return -1;
	}

	/* We retrieve the number of min/max records
	 * and average records in the buckets
	 *
	 * Here are the initializations of these variables
	 * (by using the first bucket)
	 */

	unsigned int maxRecords = numRecords(header_info, 1);
	unsigned int minRecords = maxRecords;
	double averageRecords = (double) maxRecords;
	int i;

	/* We continue to form these variables with the next buckets */

	for(i = 2; i <= header_info->numBuckets; i++)
	{
		unsigned int currentRecordsNum = numRecords(header_info, i);

		if(currentRecordsNum > maxRecords)
			maxRecords = currentRecordsNum;

		if(currentRecordsNum < minRecords)
			minRecords = currentRecordsNum;

		averageRecords += (double) currentRecordsNum;
	}

	/* We divide the total records with the number of
	 * buckets to find the average records of each bucket
	 */

	averageRecords /= header_info->numBuckets;

	/* We retrieve the number of average blocks in each bucket */

	double averageBlocks = 0.0;

	/* First, we sum all the blocks of each bucket */

	for(i = 1; i <= header_info->numBuckets; i++)
		averageBlocks += (double) blocksNumber(header_info, i);

	/* Then we divide the total blocks with the number of buckets.
	 * In this way we find the average blocks of each bucket
	 */

	averageBlocks /= header_info->numBuckets;

	/* We print the statistics */

	printf("Hash statistics of file \"%s\":\n", filename);
	printf("Number of blocks:          %d\n", numOfBlocks);
	printf("Max Records in bucket:     %u\n", maxRecords);
	printf("Min Records in bucket:     %u\n", minRecords);
	printf("Average Records in bucket: %.2lf\n", averageRecords);
	printf("Average blocks per bucket: %.2lf\n", averageBlocks);

	/* We find the buckets with overflow blocks */

	unsigned int bucketsWithOverflowBlocks = 0;

	/* For each bucket of the file, we examine if a bucket has more than 1 blocks */

	for(i = 1; i <= header_info->numBuckets; i++)
	{
		unsigned int blocksNum = blocksNumber(header_info, i);

		/* The buckets with more than 1 block have overflow blocks */

		if(blocksNum > 1)
		{
			printf("Bucket \"%d\" has %u overflow blocks\n", i, blocksNum - 1);
			bucketsWithOverflowBlocks++;
		}
	}

	/* We display the total number of buckets with overflow blocks */

	printf("There is a total of %u buckets with overflow blocks\n", bucketsWithOverflowBlocks);

	/* Finally, we close the file */

	SHT_CloseSecondaryIndex(header_info);

	return 0;
}
