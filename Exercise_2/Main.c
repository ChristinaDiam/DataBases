#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "HP.h"
#include "HT.h"
#include "SHT.h"

/***********************************************************************
 * Saves all records from the file with name 'filename' in a Hash file *
 *    and creates a secondary Hash File in which each record may be    *
 *                      searched by their surname                      *
 ***********************************************************************/

void secondaryHashTest(const char *filename, int bucketsNum)
{
	/* Initilizing the Block file level */

	BF_Init();

	/* We create a Hash file with name <filename>.hash */

	char hashFileName[strlen(filename) + 2];
	char temp[6] = ".hash";

	memcpy(hashFileName, filename, strlen(filename) - 4);
	memcpy(hashFileName + strlen(filename) - 4, temp, strlen(temp));

	hashFileName[strlen(filename) + 1] = '\0';

	int creation_result = HT_CreateIndex(hashFileName, 'i', "id", 2, bucketsNum);

	if(creation_result < 0)
	{
		printf("Could not create a primary hash file\n");
		return;
	}

	/* We open the created Hash file */

	printf("\n");
	HT_info *info = HT_OpenIndex(hashFileName);

	if(info == NULL)
	{
		printf("Could not open the primary hash file\n");
		return;
	}

	/* We create a secondary Hash file with name <filename>.sht */

	char secHashFileName[strlen(filename) + 1];
	strcpy(temp, ".sht");

	memcpy(secHashFileName, filename, strlen(filename) - 4);
	memcpy(secHashFileName + strlen(filename) - 4, temp, strlen(temp));

	secHashFileName[strlen(filename)] = '\0';

	creation_result = SHT_CreateSecondaryIndex(secHashFileName, "surname", 7, bucketsNum, hashFileName);

	if(creation_result < 0)
	{
		printf("Could not create a secondary hash file\n");
		return;
	}

	/* We open the created secondary Hash file */

	SHT_info *sec_info = SHT_OpenSecondaryIndex(secHashFileName);

	if(sec_info == NULL)
	{
		printf("Could not open the secondary hash file\n");
		return;
	}

	/* We open the file with the records */

	FILE *fp = fopen(filename, "r");

	/* If the opening was unsuccessful, we return immediatelly */

	if(fp == NULL)
	{
		printf("\nError opening file\n");
		exit(EXIT_SUCCESS);
	}

	/* We read each record from the file
	 * and we save it the hash file
	 */

	while(fgetc(fp) != EOF)
	{
		/* We will use this 'Record' variable to save all
		 * the attributes of a record in a single variable
		 */

		Record r;

		/* Helper variables */

		int i = 0;
		char c;
		char buf[8];

		/* We read the ID of the current record */

		while((c = fgetc(fp)) != ',')
			buf[i++] = c;
		buf[i] = '\0';
		r.id = atoi(buf);

		/* We bypass punctuation */

		fgetc(fp);

		/* We read the name of the current record */

		i = 0;
		while((c = fgetc(fp)) != '\"')
			r.name[i++] = c;
		r.name[i] = '\0';

		/* We bypass punctuation */

		fgetc(fp);
		fgetc(fp);

		/* We read the surname of the current record */

		i = 0;
		while((c = fgetc(fp)) != '\"')
			r.surname[i++] = c;
		r.surname[i] = '\0';

		/* We bypass punctuation */

		fgetc(fp);
		fgetc(fp);

		/* We read the address of the current record */

		i = 0;
		while((c = fgetc(fp)) != '\"')
			r.address[i++] = c;
		r.address[i] = '\0';

		/* We go to the next line and pass the punctuation */

		fscanf(fp, "%s", buf);
		fgetc(fp);

		/* We save the formed record in the Hash file */

		int insertionBlock = HT_InsertEntry(*info, r);

		if(insertionBlock < 0)
		{
			printf("Could not insert a record in the hash file\n");
			return;
		}

		/* We initialize a 'SecondaryRecord' structure
		 * and insert that in the secondary hash file
		 */

		SecondaryRecord sr;
		sr.record = r;
		sr.blockId = insertionBlock;

		int insertionResult = SHT_SecondaryInsertEntry(*sec_info, sr);

		if(insertionResult < 0)
		{
			printf("Could not insert a record in the secondary hash file\n");
			return;
		}
	}

	/* We close the records file */

	fclose(fp);

	/* We retrieve all records with ID '4' */

	int keyID = 4;
	printf("Entries with ID = %d\n", keyID);
	int total_blocks = HT_GetAllEntries(*info, &keyID);

	if(total_blocks == -1)
		printf("Error looking for key \"%d\"\n", keyID);

	else
		printf("Total blocks read to find all records with key \"%d\": %d\n", keyID, total_blocks);

	/* We retrieve all records with surname 'surname_9' */

	char sur[SURNAME_SIZE] = "surname_991";
	printf("\nEntries with surname = %s\n", sur);
	total_blocks = SHT_SecondaryGetAllEntries(*sec_info, *info, sur);

	if(total_blocks == -1)
		printf("Error looking for key \"%s\"\n", sur);

	else
		printf("Total blocks read to find all records with key \"%s\": %d\n", sur, total_blocks);

	/* We close the primary Hash file */

	int close_result = HT_CloseIndex(info);

	if(close_result < 0)
	{
		printf("Could not close the primary hash file\n");
		return;
	}

	/* We close the secondary Hash file */

	close_result = SHT_CloseSecondaryIndex(sec_info);

	if(close_result < 0)
	{
		printf("Could not close the secondary hash file\n");
		return;
	}

	/* We print the statistics of the primary Hash file */

	printf("\n");
	int statistics_result = HashStatistics(hashFileName);

	if(statistics_result < 0)
	{
		printf("Could not produce statistics of the primary hash file\n");
		return;
	}

	/* We print the statistics of the secondary Hash file */

	printf("\n");
	statistics_result = SecondaryHashStatistics(secHashFileName);

	if(statistics_result < 0)
	{
		printf("Could not produce statistics of the secondary hash file\n");
		return;
	}

	printf("\n");
}

/***********************************************************************
 * Saves all records from the file with name 'filename' in a Hash file *
 ***********************************************************************/

void hashTest(const char *filename, int bucketsNum)
{
	/* Initilizing the Block file level */

	BF_Init();

	/* We create a Hash file with name <filename>.hash */

	char hashFileName[strlen(filename) + 2];
	char temp[5] = ".hash";

	memcpy(hashFileName, filename, strlen(filename) - 4);
	memcpy(hashFileName + strlen(filename) - 4, temp, strlen(temp));

	hashFileName[strlen(filename) + 1] = '\0';

	int creation_result = HT_CreateIndex(hashFileName, 'i', "id", 2, bucketsNum);

	if(creation_result < 0)
	{
		printf("Could not create hash file\n");
		return;
	}

	/* We open the created Hash file */

	printf("\n");
	HT_info *info = HT_OpenIndex(hashFileName);

	if(info == NULL)
	{
		printf("Could not open the hash file\n");
		return;
	}

	/* We open the file with the records */

	FILE *fp = fopen(filename, "r");

	/* If the opening was unsuccessful, we return immediatelly */

	if(fp == NULL)
	{
		printf("\nError opening file\n");
		exit(EXIT_SUCCESS);
	}

	/* We read each record from the file
	 * and we save it the hash file
	 */

	while(fgetc(fp) != EOF)
	{
		/* We will use this 'Record' variable to save all
		 * the attributes of a record in a single variable
		 */

		Record r;

		/* Helper variables */

		int i = 0;
		char c;
		char buf[8];

		/* We read the ID of the current record */

		while((c = fgetc(fp)) != ',')
			buf[i++] = c;
		buf[i] = '\0';
		r.id = atoi(buf);

		/* We bypass punctuation */

		fgetc(fp);

		/* We read the name of the current record */

		i = 0;
		while((c = fgetc(fp)) != '\"')
			r.name[i++] = c;
		r.name[i] = '\0';

		/* We bypass punctuation */

		fgetc(fp);
		fgetc(fp);

		/* We read the surname of the current record */

		i = 0;
		while((c = fgetc(fp)) != '\"')
			r.surname[i++] = c;
		r.surname[i] = '\0';

		/* We bypass punctuation */

		fgetc(fp);
		fgetc(fp);

		/* We read the address of the current record */

		i = 0;
		while((c = fgetc(fp)) != '\"')
			r.address[i++] = c;
		r.address[i] = '\0';

		/* We go to the next line and pass the punctuation */

		fscanf(fp, "%s", buf);
		fgetc(fp);

		/* We save the formed record in the Hash file */

		int insertionResult = HT_InsertEntry(*info, r);

		if(insertionResult < 0)
		{
			printf("Could not insert record to the hash file\n");
			return;
		}
	}

	/* We close the records file */

	fclose(fp);

	/* We retrieve all records with ID '4' */

	int keyID = 4;
	printf("Entries with ID = %d\n", keyID);
	int total_blocks = HT_GetAllEntries(*info, &keyID);

	if(total_blocks == -1)
		printf("Error looking for key \"%d\"\n", keyID);

	else
		printf("Total blocks read to find all records with key \"%d\": %d\n", keyID, total_blocks);

	/* We close the created Hash file */

	int close_result = HT_CloseIndex(info);

	if(close_result < 0)
	{
		printf("Could not close the hash file\n");
		return;
	}

	/* We print the statistics of the Hash file */

	printf("\n");
	int statistics_result = HashStatistics(hashFileName);

	if(statistics_result < 0)
	{
		printf("Could not produce statistics for file %s\n", hashFileName);
		return;
	}

	printf("\n");
}

/***********************************************************************
 * Saves all records from the file with name 'filename' in a Heap file *
 ***********************************************************************/

void heapTest(const char *filename)
{
	/* Initilizing the Block file level */

	BF_Init();

	/* We create a Heap file with name <filename>.heap */

	char heapFileName[strlen(filename) + 2];
	char temp[5] = ".heap";

	memcpy(heapFileName, filename, strlen(filename) - 4);
	memcpy(heapFileName + strlen(filename) - 4, temp, strlen(temp));

	heapFileName[strlen(filename) + 1] = '\0';

	int creation_result = HP_CreateFile(heapFileName, 'i', "id", 2);

	if(creation_result < 0)
	{
		printf("Could not create heap file\n");
		return;
	}

	/* We open the created Heap file */

	printf("\n");
	HP_info *info = HP_OpenFile(heapFileName);

	if(info == NULL)
	{
		printf("Could not open heap file\n");
		return;
	}

	/* We open the file with the records */

	FILE *fp = fopen(filename, "r");

	/* If the opening was unsuccessful, we return immediatelly */

	if(fp == NULL)
	{
		printf("\nError opening file\n");
		exit(EXIT_SUCCESS);
	}

	/* We read each record from the file
	 * and we save it the hash file
	 */

	while(fgetc(fp) != EOF)
	{
		/* We will use this 'Record' variable to save all
		 * the attributes of a record in a single variable
		 */

		Record r;

		/* Helper variables */

		int i = 0;
		char c;
		char buf[8];

		/* We read the ID of the current record */

		while((c = fgetc(fp)) != ',')
			buf[i++] = c;
		buf[i] = '\0';
		r.id = atoi(buf);

		/* We bypass punctuation */

		fgetc(fp);

		/* We read the name of the current record */

		i = 0;
		while((c = fgetc(fp)) != '\"')
			r.name[i++] = c;
		r.name[i] = '\0';

		/* We bypass punctuation */

		fgetc(fp);
		fgetc(fp);

		/* We read the surname of the current record */

		i = 0;
		while((c = fgetc(fp)) != '\"')
			r.surname[i++] = c;
		r.surname[i] = '\0';

		/* We bypass punctuation */

		fgetc(fp);
		fgetc(fp);

		/* We read the address of the current record */

		i = 0;
		while((c = fgetc(fp)) != '\"')
			r.address[i++] = c;
		r.address[i] = '\0';

		/* We go to the next line and pass the punctuation */

		fscanf(fp, "%s", buf);
		fgetc(fp);

		/* We save the formed record in the Heap file */

		int insertionResult = HP_InsertEntry(*info, r);

		if(insertionResult < 0)
		{
			printf("Could not insert record to the heap file\n");
			return;
		}
	}

	/* We close the records file */

	fclose(fp);

	/* We retrieve all records with ID '4' */

	int keyID = 4;
	printf("Entries with ID = %d\n", keyID);
	int lastBlockID = HP_GetAllEntries(*info, &keyID);

	if(lastBlockID == -1)
		printf("Key \"%d\" was not found\n", keyID);

	else
		printf("Last Block where key \"%d\" was found: %d\n", keyID, lastBlockID);

	printf("\n");

	/* We close the created Heap file */

	int close_result = HP_CloseFile(info);

	if(close_result < 0)
	{
		printf("Could not close the heap file\n");
		return;
	}
}

/**********************************************
 * Prints the correct ways to run the program *
 **********************************************/

void printCorrectUse(const char *filename)
{
	printf("\nUsage: %s -f <recordsFile> -t heap\n", filename);
	printf("\nor\n");
	printf("\nUsage: %s -f <recordsFile> -t hash -b <buckets>\n", filename);
	printf("\nor\n");
	printf("\nUsage: %s -f <recordsFile> -t +sht -b <buckets>\n\n", filename);
}

/*****************
 * Main function *
 *****************/

int main(int argc, char const *argv[])
{
	/* We examine the input arguments and we print an
	 * informative message in case of wrong usage
	 */

	if((argc != 5 && argc != 7) || strcmp(argv[1], "-f") ||
		strcmp(argv[3], "-t") || (argc == 7 && strcmp(argv[5], "-b")))
	{
		printCorrectUse(argv[0]);
		return 0;
	}

	/* We retrieve the name of the records file */

	char recordsFile[strlen(argv[2] + 1)];
	sscanf(argv[2], "%s", recordsFile);

	/* We retrieve the desired type of file
	 * for the records (heap or hash)
	 */

	char fileType[5];
	sscanf(argv[4], "%s", fileType);

	/* Case the desired type is "hash" */

	if(!strcmp(fileType, "hash"))
	{
		if(argc == 5)
		{
			printCorrectUse(argv[0]);
			return 0;
		}

		int buckets = atoi(argv[6]);
		hashTest(recordsFile, buckets);
	}

	/* Case the desired type is "heap" */

	else if(!strcmp(fileType, "heap"))
	{
		if(argc == 7)
		{
			printCorrectUse(argv[0]);
			return 0;
		}

		heapTest(recordsFile);
	}

	/* Case the desired type is "+sht" */

	else if(!strcmp(fileType, "+sht"))
	{
		if(argc == 5)
		{
			printCorrectUse(argv[0]);
			return 0;
		}

		int buckets = atoi(argv[6]);
		secondaryHashTest(recordsFile, buckets);
	}

	/* Case of invalid file type */

	else
		printf("\nUnknown file type\n\n");

	return 0;
}
