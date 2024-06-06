#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "HP.h"

/**********************************************************
 * Prints the information saved in an 'HP_info' structure *
 **********************************************************/

static void printFileInfo(HP_info *info)
{
	printf("Information about the file:\n");
	printf("File descriptor:  %d\n", info->fileDesc);
	printf("Attribute type:   %c\n", info->attrType);
	printf("Attribute name:   %s\n", info->attrName);
	printf("Attribute length: %d\n", info->attrLength);
	printf("Size in blocks:   %d\n", BF_GetBlockCounter(info->fileDesc));
	printf("\n");
}

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

/***********************
 * Creates a Heap file *
 ***********************/

int HP_CreateFile(char *fileName, char attrType, char *attrName, int attrLength)
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
	 * We save the type of the file (this is a heap file)
	 */

	memcpy(block, "Heap" , 5);

	/* We save the type of the key */

	memcpy(block + 5, &attrType, 1);

	/* We save the length of the key */

	memcpy(block + 6, &attrLength, sizeof(int));

	/* We save the content of the key */

	memcpy(block + 6 + sizeof(int), attrName, attrLength);

	/* We save the block in the disk */

	int write_result = BF_WriteBlock(fileDesc, 0);

	if(write_result < 0)
	{
		BF_PrintError("Error writing block");
		return -1;
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

/**************************************************************************************************
 * Opens a Heap file and returns a custom "file descriptor" (a pointer to an 'HP_info' structure) *
 **************************************************************************************************/

HP_info *HP_OpenFile(char *fileName)
{
	/* String which will save the type of the file with name 'fileName' */

	char typeOfFile[5];

	/* We allocate memory for the 'HP_info' structure */

    HP_info *hp_info = (HP_info *) malloc(sizeof(HP_info));

	/* Opening the file */

	hp_info->fileDesc = BF_OpenFile(fileName);

	if(hp_info->fileDesc < 0)
	{
		BF_PrintError("Error opening file");
		free(hp_info);
		return NULL;
	}

	/* Pointer to the block we will read */

	void *block;

	/* We read the first block (block '0') */

	int read_result = BF_ReadBlock(hp_info->fileDesc, 0, &block);

	if(read_result < 0)
	{
		BF_PrintError("Error reading block");
		free(hp_info);
		return NULL;
	}

	/* We read the type of the file */

	memcpy(typeOfFile, block, 5);

	/* We compare the type of the file with the type "Heap".
	 * If the strings are not same, this is not a heap file
	 */

	if(strcmp(typeOfFile, "Heap"))
	{
		printf("This is not a heap file!\n");
		free(hp_info);
		return NULL;
	}

	/* We read the type of the key */
	
	memcpy(&(hp_info->attrType), block + 5, 1);

	/* We read the length of the key */

	memcpy(&(hp_info->attrLength), block + 6, sizeof(int));

	/* We allocate memory to save the content of the key */

	hp_info->attrName = malloc(hp_info->attrLength + 1);

	/* We read the content of the key */

	memcpy(hp_info->attrName, block + 6 + sizeof(int), hp_info->attrLength + 1);
	hp_info->attrName[hp_info->attrLength] = '\0';

	/* We call the helper function to print the created 'HP_info' custom file descriptor */

	printFileInfo(hp_info);

	/* We return the address of the 'HP_info' structure in the heap */

    return hp_info;
}

/****************************************************************
 * Closes the Heap file designated by the pointer 'header_info' *
 ****************************************************************/

int HP_CloseFile(HP_info *header_info)
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

	/* We free the allocated memory for the 'HP_info' structure */

    free(header_info->attrName);
    free(header_info);

    return 0;
}

/*****************************************************************
 * Inserts a Record in the Heap file designated by 'header_info' *
 *****************************************************************/

int HP_InsertEntry(HP_info header_info, Record record)
{
	/* First, we retrieve the number of blocks in the file */

	int numOfBlocks = BF_GetBlockCounter(header_info.fileDesc);
	int blockID;

	if(numOfBlocks < 0)
	{
		BF_PrintError("Error reading the number of blocks");
		return -1;
	}

	/* Case 1: The file has 1 block, which means it has no records,
	 * ^^^^^^
	 * as the first block of a file saves information about the file
	 */

	if(numOfBlocks == 1)
	{
		/* We allocate a new block */

		int allocation_result = BF_AllocateBlock(header_info.fileDesc);

		if(allocation_result < 0)
		{
			BF_PrintError("Error allocating block");
			return -1;
		}

		/* Pointer to the block we just allocated */

		void *block;

		/* We read that block (block '1') */

		int read_result = BF_ReadBlock(header_info.fileDesc, 1, &block);

		if(read_result < 0)
		{
			BF_PrintError("Error reading block");
			return -1;
		}

		/* We create a variable that indicates the number of records
		 * saved in the block. This will be the 'numOfRecords' variable
		 */

		int numOfRecords = 1;

		/* We copy the content of 'numOfRecords' to the block. The quantity
		 * of saved records in the block will be the first data in the block
		 */

		memcpy(block, &numOfRecords, sizeof(int));

		/* We copy the attributes of the record to be inserted in the block,
		 * exactly after the first 4 bytes, where 'numOfRecords' is saved
		 */

		memcpy(block + sizeof(int), &record.id, sizeof(int));
		memcpy(block + 2*sizeof(int), record.name, NAME_SIZE);
		memcpy(block + 2*sizeof(int) + NAME_SIZE, record.surname, SURNAME_SIZE);
		memcpy(block + 2*sizeof(int) + NAME_SIZE + SURNAME_SIZE, record.address, ADDRESS_SIZE);

		/* We save the contents of the block in the disk */

		int write_result = BF_WriteBlock(header_info.fileDesc, 1);

		if(write_result < 0)
		{
			BF_PrintError("Error writing block");
			return -1;
		}

		/* We will return the ID of the first block,
		 * as we saved there the new record
		 */

		blockID = 1;
	}

	/* Case 2: The file has at least 2 blocks,
	 * ^^^^^^
	 * which means it has saved some records
	 */

	else
	{
		void *block;

		/* We read the last block that was allocated */

		int read_result = BF_ReadBlock(header_info.fileDesc, numOfBlocks - 1, &block);

		if(read_result < 0)
		{
			BF_PrintError("Error reading block");
			return -1;
		}

		/* We retrieve the number of records saved in that block */

		int numOfRecords;
		memcpy(&numOfRecords, block, sizeof(int));

		/* Subcase 2-1: There is enough space in the last
		 * ^^^^^^^^^^^
		 * block so the record to be inserted be saved
		 */

		if(numOfRecords < (BLOCK_SIZE - sizeof(int)) / sizeof(Record))
		{
			/* We increase the number of records saved in the block by 1
			 * and we transfer the new value of 'numOfRecords' to the block
			 */

			numOfRecords++;
			memcpy(block, &numOfRecords, sizeof(int));

			/* We save the contents of the record to be inserted after the first 4 bytes, which
			 * are used to save 'numOfRecords' and the first 'numOfRecords - 1' records that are
			 * already saved in the block. We want to add the information serially, one record
			 * after another, so we must take into account the space used to save the previous
			 * (the older) records
			 */

			memcpy(block + sizeof(int) + (numOfRecords - 1) * sizeof(Record), &record.id, sizeof(int));
			memcpy(block + 2*sizeof(int) + (numOfRecords - 1) * sizeof(Record), record.name, NAME_SIZE);

			memcpy(block + 2*sizeof(int) + NAME_SIZE +
				(numOfRecords - 1) * sizeof(Record), record.surname, SURNAME_SIZE);

			memcpy(block + 2*sizeof(int) + NAME_SIZE + SURNAME_SIZE +
				(numOfRecords - 1) * sizeof(Record), record.address, ADDRESS_SIZE);

			/* We save the contents of the block in the disk */

			int write_result = BF_WriteBlock(header_info.fileDesc, numOfBlocks - 1);

			if(write_result < 0)
			{
				BF_PrintError("Error writing block");
				return -1;
			}

			/* We will return the ID of the last block,
		 	 * as we saved there the new record
		 	 */

			blockID = numOfBlocks - 1;
		}

		/* Subcase 2-2: There is not enough space in the last
		 * ^^^^^^^^^^^
		 * block and we must allocate a new block to save the record
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

			/* Pointer to the block we just allocated */

			void *block;

			/* We read that block */

			int read_result = BF_ReadBlock(header_info.fileDesc, numOfBlocks, &block);

			if(read_result < 0)
			{
				BF_PrintError("Error reading block");
				return -1;
			}

			/* We create a variable that indicates the number of records
		 	 * saved in the block. This will be the 'numOfRecords' variable
		 	 */

			int numOfRecords = 1;

			/* We copy the content of 'numOfRecords' to the block. The quantity
			 * of saved records in the block will be the first data in the block
			 */

			memcpy(block, &numOfRecords, sizeof(int));

			/* We copy the attributes of the record to be inserted in the block,
		 	 * exactly after the first 4 bytes, where 'numOfRecords' is saved
		 	 */

			memcpy(block + sizeof(int), &record.id, sizeof(int));
			memcpy(block + 2*sizeof(int), record.name, NAME_SIZE);
			memcpy(block + 2*sizeof(int) + NAME_SIZE, record.surname, SURNAME_SIZE);
			memcpy(block + 2*sizeof(int) + NAME_SIZE + SURNAME_SIZE, record.address, ADDRESS_SIZE);

			/* We save the contents of the block in the disk */

			int write_result = BF_WriteBlock(header_info.fileDesc, numOfBlocks);

			if(write_result < 0)
			{
				BF_PrintError("Error writing block");
				return -1;
			}

			/* We will return the ID of the last block, as we saved there the new
		 	 * record. As we allocated a new block and the variable 'numOfBlocks'
		 	 * is not updated, the last block is now the one with ID 'numOfBlocks'
		 	 * instead of the one with ID 'numOfBlocks - 1'
		 	 */

			blockID = numOfBlocks;
		}
	}

    return blockID;
}

/*******************************************************************
 * Removes a Record from the Heap file designated by 'header_info' *
 *******************************************************************/

int HP_DeleteEntry(HP_info header_info, void *value)
{
	/* First, we retrieve the number of blocks in the file */

	int numOfBlocks = BF_GetBlockCounter(header_info.fileDesc);

	/* If the file has 1 block, that means the file has no records
	 * (the first block always saves information about the file)
	 */

	if(numOfBlocks == 1)
		return -1;

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
			/* We retrieve the ID of the current record */

			int currentID;
			memcpy(&currentID, block + sizeof(int) + j * sizeof(Record), sizeof(int));

			/* If the 'value' is 'NULL' or it is the same as
			 * the ID of the record, we delete that record
			 */

			if(value == NULL || *((int *) value) == currentID)
			{
				/* We retrieve the contents of the record to be deleted */

				Record key_record;
				key_record.id = currentID;

				memcpy(key_record.name, block + sizeof(int) + j * sizeof(Record) +
					sizeof(int), NAME_SIZE);

				memcpy(key_record.surname, block + sizeof(int) + j * sizeof(Record) +
					sizeof(int) + NAME_SIZE, SURNAME_SIZE);

				memcpy(key_record.address, block + sizeof(int) + j * sizeof(Record) +
					sizeof(int) + NAME_SIZE + SURNAME_SIZE, ADDRESS_SIZE);

				/* If that record is a 'NULL' record, there is nothing to be deleted.
				 * This is the reason why we wanted to retrieve its contents from the block.
				 */

				if(isNull(key_record))
					continue;

				/* To delete the record, we set it to be a 'NULL' record */

				nullify(&key_record);

				/* Now we transfer the new contents of the (nullified) record to the block */

				memcpy(block + sizeof(int) + j * sizeof(Record), &key_record.id, sizeof(int));

				memcpy(block + sizeof(int) + j * sizeof(Record) +
					sizeof(int), key_record.name, NAME_SIZE);

				memcpy(block + sizeof(int) + j * sizeof(Record) + sizeof(int) +
					NAME_SIZE, key_record.surname, SURNAME_SIZE);

				memcpy(block + sizeof(int) + j * sizeof(Record) + sizeof(int) +
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

	/* If this code is reached, that means we have not found the desired
	 * record for deletion, as there is no record with ID equal to 'value'.
	 * In this case, we return -1
	 */

	return -1;
}

/*********************************************************************************
 * Prints all entries with key 'value' from the file designated by 'header_info' *
 *********************************************************************************/

int HP_GetAllEntries(HP_info header_info, void *value)
{
	/* First, we retrieve the number of blocks in the file */

	int numOfBlocks = BF_GetBlockCounter(header_info.fileDesc);

	/* If the file has 1 block, that means the file has no records
	 * (the first block always saves information about the file)
	 */

	if(numOfBlocks == 1)
		return -1;

	/* From block '1' to the last block we will look the records
	 * of each block and search the desired record for printing
	 *
	 * The 'flag' variable will hold the ID of the last block
	 * that contained a desired record for printing
	 */

	int i, flag = -1;
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
			/* We retrieve the ID of the current record */

			int currentID;
			memcpy(&currentID, block + sizeof(int) + j * sizeof(Record), sizeof(int));

			/* If the 'value' is 'NULL' or it is the same as
			 * the ID of the record, we print that record
			 */

			if(value == NULL || *((int *) value) == currentID)
			{
				/* We retrieve the contents of the record from the block */

				Record key_record;
				key_record.id = currentID;

				memcpy(key_record.name, block + sizeof(int) + j * sizeof(Record) +
					sizeof(int), NAME_SIZE);

				memcpy(key_record.surname, block + sizeof(int) + j * sizeof(Record) +
					sizeof(int) + NAME_SIZE, SURNAME_SIZE);

				memcpy(key_record.address, block + sizeof(int) + j * sizeof(Record) +
					sizeof(int) + NAME_SIZE + SURNAME_SIZE, ADDRESS_SIZE);

				/* If that record is a 'NULL' record, there is nothing to print */

				if(isNull(key_record))
					continue;

				/* Else we print the desired record */

				printRecord(key_record);

				/* We assign the ID of the current block to the 'flag' variable */

				flag = i;
			}
		}
	}

	/* Finally, we return the 'flag', which holds the ID of
	 * the last block from which we printed a desired record
	 */

	return flag;
}
