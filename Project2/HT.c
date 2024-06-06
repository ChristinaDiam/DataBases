#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "HT.h"

/***********************
 * Prints a record 'r' *
 ***********************/

static void printRecord(Record r)
{
	printf("{%d, %s, %s, %s}\n", r.id, r.name, r.surname, r.address);
}

/********************************************************
 * Sets to record 'r' the attributes of a 'NULL' record *
 ********************************************************/

static void nullify(Record *r)
{
	r->id = 0;
	strcpy(r->name,    "NullString");
	strcpy(r->surname, "NullString");
	strcpy(r->address, "NullString");
}

/***********************************************
 * Examines if a record 'r' is a 'NULL' record *
 ***********************************************/

static int isNull(Record r)
{
	return (r.id == 0 && !strcmp(r.name, "NullString") &&
		!strcmp(r.surname, "NullString") && !strcmp(r.address, "NullString"));
}

/*******************************
 * Function to hash an integer *
 *******************************/

static int hashIntegers(int data, int numOfBuckets)
{
	return (data % numOfBuckets) + 1;
}

/***********************
 * Creates a Hash file *
 ***********************/

int HT_CreateIndex(char *fileName, char attrType, char *attrName, int attrLength, int buckets)
{
	/* We create a new Block file */

	int create_result = BF_CreateFile(fileName);

	if(create_result < 0)
	{
		BF_PrintError("Error creating file");
		return -1;
	}

	/* We open the Block file we created */

	int fileDesc = BF_OpenFile(fileName);

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

	memcpy(block, "Hash" , 5);

	/* We save the type of the key */

	memcpy(block + 5, &attrType, 1);

	/* We save the length of the key */

	memcpy(block + 6, &attrLength, sizeof(int));

	/* We save the content of the key */

	memcpy(block + 6 + sizeof(int), attrName, attrLength);

	/* We save the number of buckets */

	memcpy(block + 6 + sizeof(int) + attrLength, &buckets, sizeof(int));

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

/********************************************
 * Opens the Hash file with name 'filename' *
 ********************************************/

HT_info *HT_OpenIndex(char *fileName)
{
	/* String which will save the type of the file with name 'fileName' */

	char typeOfFile[5];

	/* We allocate memory for the 'HT_info' structure */

    HT_info *ht_info = (HT_info *) malloc(sizeof(HT_info));

    /* Opening the file */

	ht_info->fileDesc = BF_OpenFile(fileName);

	if(ht_info->fileDesc < 0)
	{
		BF_PrintError("Error opening file");
		free(ht_info);
		return NULL;
	}

	/* Pointer to the block we will read */

	void *block;

	/* We read the first block (block '0') */

	int read_result = BF_ReadBlock(ht_info->fileDesc, 0, &block);

	if(read_result < 0)
	{
		BF_PrintError("Error reading block");
		free(ht_info);
		return NULL;
	}

	/* We read the type of the file */

	memcpy(typeOfFile, block, 5);

	/* We compare the type of the file with the type "Hash".
	 * If the strings are not same, this is not a hash file
	 */

	if(strcmp(typeOfFile, "Hash"))
	{
		printf("This is not a hash file!\n");
		free(ht_info);
		return NULL;
	}

	/* We read the type of the key */
	
	memcpy(&(ht_info->attrType), block + 5, 1);

	/* We read the length of the key */

	memcpy(&(ht_info->attrLength), block + 6, sizeof(int));

	/* We allocate memory to save the content of the key */

	ht_info->attrName = malloc(ht_info->attrLength + 1);

	/* We read the content of the key */

	memcpy(ht_info->attrName, block + 6 + sizeof(int), ht_info->attrLength + 1);
	ht_info->attrName[ht_info->attrLength] = '\0';

	/* We read the number of buckets */

	memcpy(&(ht_info->numBuckets), block + 6 + sizeof(int) + ht_info->attrLength, sizeof(int));

    return ht_info;
}

/******************************************************************
 * Closes the Hash file designated by the structure 'header_info' *
 ******************************************************************/

int HT_CloseIndex(HT_info *header_info)
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
    free(header_info);

    return 0;
}

/******************************************************************************************
 * Inserts the record 'record' in the Hash file designated by the structure 'header_info' *
 ******************************************************************************************/

int HT_InsertEntry(HT_info header_info, Record record)
{
	/* The ID of the block which we will return if everything goes well */

	int blockID;

	/* We hash the given record ID */

	int keyBlock = hashIntegers(record.id, header_info.numBuckets);

	/* Pointer to the key block */

	void *block;

	/* The number of records saved in the key block */

	int numOfRecords;

	while(1)
	{
		/* We read the key block */

		int read_result = BF_ReadBlock(header_info.fileDesc, keyBlock, &block);

		if(read_result < 0)
		{
			BF_PrintError("Error reading block");
			return -1;
		}

		/* We retrieve the number of records saved in that
		 * block, as well as the ID of the linked block
		 */

		int linkedBlock;
		memcpy(&numOfRecords, block, sizeof(int));
		memcpy(&linkedBlock, block + sizeof(int), sizeof(int));

		if(linkedBlock == 0)
			break;

		keyBlock = linkedBlock;
	}

	/* Case 1: There is enough space in the last
	 * ^^^^^^
	 * block so the record to be inserted be saved
	 */

	if(numOfRecords < (BLOCK_SIZE - 2*sizeof(int)) / sizeof(Record))
	{
		/* We increase the number of records saved in the block by 1
		 * and we transfer the new value of 'numOfRecords' to the block
		 */

		numOfRecords++;
		memcpy(block, &numOfRecords, sizeof(int));

		/* We save the contents of the record to be inserted after the first 8 bytes (which
		 * are used to save 'numOfRecords' and 'nextBlock') and the first 'numOfRecords - 1'
		 * records that are already saved in the block. We want to add the information serially,
		 * one record after another, so we must take into account the space used to keep the
		 * previous (the older) records saved
		 */

		memcpy(block + 2*sizeof(int) + (numOfRecords - 1) * (sizeof(int) + NAME_SIZE + SURNAME_SIZE + ADDRESS_SIZE), &record.id, sizeof(int));
		memcpy(block + 3*sizeof(int) + (numOfRecords - 1) * (sizeof(int) + NAME_SIZE + SURNAME_SIZE + ADDRESS_SIZE), record.name, NAME_SIZE);

		memcpy(block + 3*sizeof(int) + NAME_SIZE +
			(numOfRecords - 1) * (sizeof(int) + NAME_SIZE + SURNAME_SIZE + ADDRESS_SIZE), record.surname, SURNAME_SIZE);

		memcpy(block + 3*sizeof(int) + NAME_SIZE + SURNAME_SIZE +
			(numOfRecords - 1) * (sizeof(int) + NAME_SIZE + SURNAME_SIZE + ADDRESS_SIZE), record.address, ADDRESS_SIZE);

		/* We save the contents of the block in the disk */

		int write_result = BF_WriteBlock(header_info.fileDesc, keyBlock);

		if(write_result < 0)
		{
			BF_PrintError("Error writing block");
			return -1;
		}

		/* We will return the ID of the key block,
		 * as we saved there the new record
		 */

		blockID = keyBlock;
	}

	/* Case 2: There is not enough space in the last
	 * ^^^^^^
	 * block so the record to be inserted be saved
	 */

	else
	{
		/* We allocate a new block */

		int allocation_result = BF_AllocateBlock(header_info.fileDesc);

		if(allocation_result < 0)
		{
			BF_PrintError("Error allocating block");
			return -1;
		}

		/* We retrieve the number of blocks */

		int blocksNum = BF_GetBlockCounter(header_info.fileDesc);

		if(blocksNum < 0)
		{
			BF_PrintError("Error in block counter");
			return -1;
		}

		/* We declare which will be the linked
		 * block and we save it in the first block
		 */

		int linked_block = blocksNum - 1;
		memcpy(block + sizeof(int), &linked_block, sizeof(int));

		/* We save the contents of the block in the disk */

		int write_result = BF_WriteBlock(header_info.fileDesc, keyBlock);

		if(write_result < 0)
		{
			BF_PrintError("Error writing block");
			return -1;
		}

		/* We read the new block */

		int read_result = BF_ReadBlock(header_info.fileDesc, linked_block, &block);

		if(read_result < 0)
		{
			BF_PrintError("Error reading block");
			return -1;
		}

		/* We create a variable that indicates the number of records
		 * saved in the block. This will be the 'numOfRecords' variable
		 *
		 * Also, we set the 'nextBlock' to be block '0' for the time
		 */

		int numOfRecords = 1;
		int nextBlock = 0;

		/* We copy the content of 'numOfRecords' and 'nextBlock' to the block. The
		 * quantity of saved records in the block will be the first data in the block
		 */

		memcpy(block, &numOfRecords, sizeof(int));
		memcpy(block + sizeof(int), &nextBlock, sizeof(int));

		/* We copy the attributes of the record to be inserted in the block, exactly
		 * after the first 8 bytes, where 'numOfRecords' and 'nextBlock' are saved
		 */

		memcpy(block + 2*sizeof(int), &record.id, sizeof(int));
		memcpy(block + 3*sizeof(int), record.name, NAME_SIZE);
		memcpy(block + 3*sizeof(int) + NAME_SIZE, record.surname, SURNAME_SIZE);
		memcpy(block + 3*sizeof(int) + NAME_SIZE + SURNAME_SIZE, record.address, ADDRESS_SIZE);

		/* We save the contents of the block in the disk */

		write_result = BF_WriteBlock(header_info.fileDesc, linked_block);

		if(write_result < 0)
		{
			BF_PrintError("Error writing block");
			return -1;
		}

		blockID = linked_block;
	}

	return blockID;
}

/****************************************************************************************************
 * Removes the record with key 'value' from the Hash file designated by the structure 'header_info' *
 ****************************************************************************************************/

int HT_DeleteEntry(HT_info header_info, void *value)
{
	if(value == NULL)
	{
		/* First, we retrieve the number of blocks in the file */

		int numOfBlocks = BF_GetBlockCounter(header_info.fileDesc);

		if(numOfBlocks < 0)
		{
			BF_PrintError("Error in block counter");
			return -1;
		}

		/* From block '1' to the last block we will look the records
	 	 * of each block and search the desired record for deletion
		 */

		int i;
		for(i = 1; i < numOfBlocks; i++)
		{
			/* Pointer to the block 'i' we will read */

			void *block;

			/* We read the 'i-th' block */

			int read_result = BF_ReadBlock(header_info.fileDesc, i, &block);

			if(read_result < 0)
			{
				BF_PrintError("Error reading block");
				return -1;
			}

			/* We retrieve the number of records saved in the block */

			int recordsNum;
			memcpy(&recordsNum, block, sizeof(int));

			/* Starting from the first record until the last, we compare
			 * the value of the key of the record to the given 'value'
			 */

			int j;
			for(j = 0; j < recordsNum; j++)
			{
				/* We retrieve the contents of the record to be deleted */

				Record key_record;

				int currentID;
				memcpy(&currentID, block + 2*sizeof(int) + j * sizeof(Record), sizeof(int));

				key_record.id = currentID;

				memcpy(key_record.name, block + 2*sizeof(int) + j * sizeof(Record) +
					sizeof(int), NAME_SIZE);

				memcpy(key_record.surname, block + 2*sizeof(int) + j * sizeof(Record) +
					sizeof(int) + NAME_SIZE, SURNAME_SIZE);

				memcpy(key_record.address, block + 2*sizeof(int) + j * sizeof(Record) +
					sizeof(int) + NAME_SIZE + SURNAME_SIZE, ADDRESS_SIZE);

				/* If that record is a 'NULL' record, there is nothing to be deleted.
				 * This is the reason why we wanted to retrieve its contents from the block.
				 */

				if(isNull(key_record))
					continue;

				/* To delete the record, we set it to be a 'NULL' record */

				nullify(&key_record);

				/* Now we transfer the new contents of the (nullified) record to the block */

				memcpy(block + 2*sizeof(int) + j * sizeof(Record), &key_record.id, sizeof(int));

				memcpy(block + 2*sizeof(int) + j * sizeof(Record) +
					sizeof(int), key_record.name, NAME_SIZE);

				memcpy(block + 2*sizeof(int) + j * sizeof(Record) + sizeof(int) +
					NAME_SIZE, key_record.surname, SURNAME_SIZE);

				memcpy(block + 2*sizeof(int) + j * sizeof(Record) + sizeof(int) +
					NAME_SIZE + SURNAME_SIZE, key_record.address, ADDRESS_SIZE);

				/* We save the contents of the block in the disk */

				int write_result = BF_WriteBlock(header_info.fileDesc, i);

				if(write_result < 0)
				{
					BF_PrintError("Error writing block");
					return -1;
				}

				/* Since we deleted the desired record, we return 0 */

				return 0;
			}
		}
	}

	else
	{
		int keyBlock = hashIntegers(*((int *) value), header_info.numBuckets);

		while(keyBlock != 0)
		{
			/* Pointer to the key block we will read */

			void *block;

			/* We read the key block */

			int read_result = BF_ReadBlock(header_info.fileDesc, keyBlock, &block);

			if(read_result < 0)
			{
				BF_PrintError("Error reading block");
				return -1;
			}

			/* We retrieve the number of records saved in the block */

			int recordsNum;
			memcpy(&recordsNum, block, sizeof(int));

			/* Starting from the first record until the last, we compare
			 * the value of the key of the record to the given 'value'
		 	 */

			int j;
			for(j = 0; j < recordsNum; j++)
			{
				int currentID;
				memcpy(&currentID, block + 2*sizeof(int) + j * sizeof(Record), sizeof(int));

				if(*((int *) value) == currentID)
				{
					/* We retrieve the contents of the record from the block */

					Record key_record;

					key_record.id = currentID;

					memcpy(key_record.name, block + 2*sizeof(int) + j * sizeof(Record) +
						sizeof(int), NAME_SIZE);

					memcpy(key_record.surname, block + 2*sizeof(int) + j * sizeof(Record) +
						sizeof(int) + NAME_SIZE, SURNAME_SIZE);

					memcpy(key_record.address, block + 2*sizeof(int) + j * sizeof(Record) +
						sizeof(int) + NAME_SIZE + SURNAME_SIZE, ADDRESS_SIZE);

					/* If that record is a 'NULL' record, there is nothing to print */

					if(isNull(key_record))
						continue;

					/* Else we delete the desired record */

					nullify(&key_record);

					/* Now we transfer the new contents of the (nullified) record to the block */

					memcpy(block + 2*sizeof(int) + j * sizeof(Record), &key_record.id, sizeof(int));

					memcpy(block + 2*sizeof(int) + j * sizeof(Record) +
						sizeof(int), key_record.name, NAME_SIZE);

					memcpy(block + 2*sizeof(int) + j * sizeof(Record) + sizeof(int) +
						NAME_SIZE, key_record.surname, SURNAME_SIZE);

					memcpy(block + 2*sizeof(int) + j * sizeof(Record) + sizeof(int) +
						NAME_SIZE + SURNAME_SIZE, key_record.address, ADDRESS_SIZE);

					/* We save the contents of the block in the disk */

					int write_result = BF_WriteBlock(header_info.fileDesc, keyBlock);

					if(write_result < 0)
					{
						BF_PrintError("Error writing block");
						return -1;
					}

					/* Since we deleted the desired record, we return 0 */

					return 0;
				}
			}

			/* If the record for deletion was not found in this block,
			 * we look for it the linked block of the current block.
			 */

			memcpy(&keyBlock, block + sizeof(int), sizeof(int));
		}
	}

	/* If this code is reached, that means we have not found the desired
	 * record for deletion, as there is no record with ID equal to 'value'.
	 * In this case, we return -1
	 */

	return -1;
}

/****************************************************************************************************
 * Prints all records with key 'value' from the Hash file designated by the structure 'header_info' *
 ****************************************************************************************************/

int HT_GetAllEntries(HT_info header_info, void *value)
{
	/* We initialize a counter of blocks to value '0' */

	int counter = 0;

	/* Case the 'value' is 'NULL' */

	if(value == NULL)
	{
		/* First, we retrieve the number of blocks in the file */

		int numOfBlocks = BF_GetBlockCounter(header_info.fileDesc);

		if(numOfBlocks < 0)
		{
			BF_PrintError("Error in block counter");
			return -1;
		}

		/* From block '1' to the last block we will look the records
	 	 * of each block and search the desired record for printing
	 	 *
		 * The 'counter' variable will hold the number of read blocks
	 	 */

		int i;
		for(i = 1; i < numOfBlocks; i++)
		{
			/* Pointer to the block 'i' we will read */

			void *block;

			/* We read the 'i-th' block */

			int read_result = BF_ReadBlock(header_info.fileDesc, i, &block);

			if(read_result < 0)
			{
				BF_PrintError("Error reading block");
				return -1;
			}

			/* We increase the 'counter' variable by 1, since we read another block */

			counter++;

			/* We retrieve the number of records saved in the block */

			int recordsNum;
			memcpy(&recordsNum, block, sizeof(int));

			/* Starting from the first record until the last, we compare
			 * the value of the key of the record to the given 'value'
			 */

			int j;
			for(j = 0; j < recordsNum; j++)
			{
				/* We retrieve the contents of the record from the block */

				Record key_record;

				int currentID;
				memcpy(&currentID, block + 2*sizeof(int) + j * (sizeof(int) + NAME_SIZE + SURNAME_SIZE + ADDRESS_SIZE), sizeof(int));

				key_record.id = currentID;

				memcpy(key_record.name, block + 2*sizeof(int) + j * (sizeof(int) + NAME_SIZE + SURNAME_SIZE + ADDRESS_SIZE) +
					sizeof(int), NAME_SIZE);

				memcpy(key_record.surname, block + 2*sizeof(int) + j * (sizeof(int) + NAME_SIZE + SURNAME_SIZE + ADDRESS_SIZE) +
					sizeof(int) + NAME_SIZE, SURNAME_SIZE);

				memcpy(key_record.address, block + 2*sizeof(int) + j * (sizeof(int) + NAME_SIZE + SURNAME_SIZE + ADDRESS_SIZE) +
					sizeof(int) + NAME_SIZE + SURNAME_SIZE, ADDRESS_SIZE);

				/* If that record is a 'NULL' record, there is nothing to print */

				if(isNull(key_record))
					continue;

				/* Else we print the desired record */

				printRecord(key_record);
			}
		}
	}	

	/* Case the 'value' is not 'NULL' */

	else
	{
		int keyBlock = hashIntegers(*((int *) value), header_info.numBuckets);

		while(keyBlock != 0)
		{
			/* Pointer to the key block we will read */

			void *block;

			/* We read the 'i-th' block */

			int read_result = BF_ReadBlock(header_info.fileDesc, keyBlock, &block);

			if(read_result < 0)
			{
				BF_PrintError("Error reading block");
				return -1;
			}

			/* We increase the 'counter' variable by 1, since we read another block */

			counter++;

			/* We retrieve the number of records saved in the block */

			int recordsNum;
			memcpy(&recordsNum, block, sizeof(int));

			/* Starting from the first record until the last, we compare
			 * the value of the key of the record to the given 'value'
		 	 */

			int j;
			for(j = 0; j < recordsNum; j++)
			{
				int currentID;
				memcpy(&currentID, block + 2*sizeof(int) + j * (sizeof(int) + NAME_SIZE + SURNAME_SIZE + ADDRESS_SIZE), sizeof(int));

				if(*((int *) value) == currentID)
				{
					/* We retrieve the contents of the record from the block */

					Record key_record;

					key_record.id = currentID;

					memcpy(key_record.name, block + 2*sizeof(int) + j * (sizeof(int) + NAME_SIZE + SURNAME_SIZE + ADDRESS_SIZE) +
						sizeof(int), NAME_SIZE);

					memcpy(key_record.surname, block + 2*sizeof(int) + j * (sizeof(int) + NAME_SIZE + SURNAME_SIZE + ADDRESS_SIZE) +
						sizeof(int) + NAME_SIZE, SURNAME_SIZE);

					memcpy(key_record.address, block + 2*sizeof(int) + j * (sizeof(int) + NAME_SIZE + SURNAME_SIZE + ADDRESS_SIZE) +
						sizeof(int) + NAME_SIZE + SURNAME_SIZE, ADDRESS_SIZE);

					/* If that record is a 'NULL' record, there is nothing to print */

					if(isNull(key_record))
						continue;

					/* Else we print the desired record */

					printRecord(key_record);
				}
			}

			/* We get all desired entries form the linked block as well */

			memcpy(&keyBlock, block + sizeof(int), sizeof(int));
		}
	}

	/* Finally, we return the number of read blocks */

	return counter;
}

/******************************************************************************************
 * Returns the number of records in a bucket that starts from the block with ID 'blockID' *
 ******************************************************************************************/

static unsigned int numRecords(HT_info *header_info, int blockID)
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

	unsigned int result = 0;
	int i;
	for(i = 0; i < recordsInBlock; i++)
	{
		/* We retrieve the contents of the current record from the block */

		Record key_record;

		int currentID;
		memcpy(&currentID, block + 2*sizeof(int) + i * (sizeof(int) + NAME_SIZE + SURNAME_SIZE + ADDRESS_SIZE), sizeof(int));

		key_record.id = currentID;

		memcpy(key_record.name, block + 2*sizeof(int) + i * (sizeof(int) + NAME_SIZE + SURNAME_SIZE + ADDRESS_SIZE) +
			sizeof(int), NAME_SIZE);

		memcpy(key_record.surname, block + 2*sizeof(int) + i * (sizeof(int) + NAME_SIZE + SURNAME_SIZE + ADDRESS_SIZE) +
			sizeof(int) + NAME_SIZE, SURNAME_SIZE);

		memcpy(key_record.address, block + 2*sizeof(int) + i * (sizeof(int) + NAME_SIZE + SURNAME_SIZE + ADDRESS_SIZE) +
			sizeof(int) + NAME_SIZE + SURNAME_SIZE, ADDRESS_SIZE);

		if(isNull(key_record))
			continue;

		result++;
	}

	return result + numRecords(header_info, linkedBlock);
}

/*****************************************************************************************
 * Returns the number of blocks in a bucket that starts from the block with ID 'blockID' *
 *****************************************************************************************/

static unsigned int blocksNumber(HT_info *header_info, int blockID)
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

	/* We retrieve the linked block */

	int linkedBlock;
	memcpy(&linkedBlock, block + sizeof(int), sizeof(int));

	return 1 + blocksNumber(header_info, linkedBlock);
}

/********************************************************************
 * Prints the hash statistics of the Hash file with name 'filename' *
 ********************************************************************/

int HashStatistics(char *filename)
{
	/* Firts, we open the file */

	HT_info *header_info = HT_OpenIndex(filename);

	/* We retrieve the number of blocks in the file */

	int numOfBlocks = BF_GetBlockCounter(header_info->fileDesc);

	if(numOfBlocks < 0)
	{
		BF_PrintError("Error in block counter");
		return -1;
	}

	/* We retrieve the number of min/max records and average records in the buckets */

	unsigned int maxRecords = numRecords(header_info, 1);
	unsigned int minRecords = maxRecords;
	double averageRecords = (double) maxRecords;
	int i;

	for(i = 2; i <= header_info->numBuckets; i++)
	{
		unsigned int currentRecordsNum = numRecords(header_info, i);

		if(currentRecordsNum > maxRecords)
			maxRecords = currentRecordsNum;

		if(currentRecordsNum < minRecords)
			minRecords = currentRecordsNum;

		averageRecords += (double) currentRecordsNum;
	}

	averageRecords /= header_info->numBuckets;

	/* We retrieve the number of average blocks in each bucket */

	double averageBlocks = 0.0;

	for(i = 1; i <= header_info->numBuckets; i++)
		averageBlocks += (double) blocksNumber(header_info, i);

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

	for(i = 1; i <= header_info->numBuckets; i++)
	{
		unsigned int blocksNum = blocksNumber(header_info, i);

		if(blocksNum > 1)
		{
			printf("Bucket \"%d\" has %u overflow blocks\n", i, blocksNum - 1);
			bucketsWithOverflowBlocks++;
		}
	}

	printf("There is a total of %u buckets with overflow blocks\n", bucketsWithOverflowBlocks);

	/* Finally, we close the file */

	HT_CloseIndex(header_info);

	return 0;
}