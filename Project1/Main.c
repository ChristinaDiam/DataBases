#include <stdio.h>
#include <stdlib.h>
#include "HP.h"
#include "HT.h"

void test_1()
{
	/* Initilizing the Block file level */

	BF_Init();

	/* We create a Heap file */

	HP_CreateFile("myTest.heap", 'i', "id", 2);

	/* We open the created Heap file */

	HP_info *info = HP_OpenFile("myTest.heap");

	/* We insert some records in the file */

	Record r1 = {2, "Name1", "Surname1", "Address1"};
	Record r2 = {5, "Name2", "Surname2", "Address2"};
	Record r3 = {5, "Name3", "Surname3", "Address3"};
	Record r4 = {5, "Name4", "Surname4", "Address4"};
	Record r5 = {8, "Name5", "Surname5", "Address5"};
	Record r6 = {5, "Name6", "Surname6", "Address6"};
	Record r7 = {8, "Name7", "Surname7", "Address7"};
	Record r8 = {5, "Name8", "Surname8", "Address8"};

	HP_InsertEntry(*info, r1);
	HP_InsertEntry(*info, r2);
	HP_InsertEntry(*info, r3);
	HP_InsertEntry(*info, r4);
	HP_InsertEntry(*info, r5);
	HP_InsertEntry(*info, r6);
	HP_InsertEntry(*info, r7);
	HP_InsertEntry(*info, r8);

	/* We retrieve all records with ID '5' */

	int keyID = 5;
	printf("\nEntries with ID = %d\n", keyID);
	int lastBlockID = HP_GetAllEntries(*info, &keyID);
	printf("Last Block where key \"%d\" was found: %d\n", keyID, lastBlockID);

	/* We retrieve all records with ID '4' */

	keyID = 4;
	printf("\nEntries with ID = %d\n", keyID);
	lastBlockID = HP_GetAllEntries(*info, &keyID);
	printf("Last Block where key \"%d\" was found: %d\n", keyID, lastBlockID);

	/* We retrieve all records */

	printf("\nAll entries:\n");
	HP_GetAllEntries(*info, NULL);

	/* We delete the first record with ID '8' */

	int keyForDeletion = 8;
	printf("\nDeleting the first appearance of key \"%d\":\n", keyForDeletion);
	HP_DeleteEntry(*info, &keyForDeletion);
	HP_GetAllEntries(*info, NULL);

	/* We close the created Heap file */

	HP_CloseFile(info);
}

void test_2()
{
	/* Initilizing the Block file level */

	BF_Init();

	/* We create a Hash file */

	HT_CreateIndex("myHash.hash", 'i', "id", 2, 8);

	/* We open the created Hash file */

	HT_info *info = HT_OpenIndex("myHash.hash");

	/* We insert some records in the file */

	Record r1 = {2, "Name1", "Surname1", "Address1"};
	Record r2 = {5, "Name2", "Surname2", "Address2"};
	Record r3 = {5, "Name3", "Surname3", "Address3"};
	Record r4 = {5, "Name4", "Surname4", "Address4"};
	Record r5 = {8, "Name5", "Surname5", "Address5"};
	Record r6 = {5, "Name6", "Surname6", "Address6"};
	Record r7 = {5, "Name7", "Surname7", "Address7"};
	Record r8 = {13,"Name8", "Surname8", "Address8"};

	HT_InsertEntry(*info, r1);
	HT_InsertEntry(*info, r2);
	HT_InsertEntry(*info, r3);
	HT_InsertEntry(*info, r4);
	HT_InsertEntry(*info, r5);
	HT_InsertEntry(*info, r6);
	HT_InsertEntry(*info, r7);
	HT_InsertEntry(*info, r8);

	/* We retrieve all records */

	printf("All entries:\n");
	HT_GetAllEntries(*info, NULL);

	/* We retrieve all records with ID '5' */

	int keyID = 5;
	printf("\nEntries with ID = %d\n", keyID);
	int lastBlockID = HT_GetAllEntries(*info, &keyID);
	printf("Last Block where key \"%d\" was found: %d\n", keyID, lastBlockID);

	/* We delete the first record with ID '13' */

	int keyForDeletion = 13;
	printf("\nDeleting the first appearance of key \"%d\":\n", keyForDeletion);
	HT_DeleteEntry(*info, &keyForDeletion);
	HT_GetAllEntries(*info, NULL);

	/* We close the created Hash file */

	HT_CloseIndex(info);

	printf("\n");
	HashStatistics("myHash.hash");
	printf("\n");
}

void test_3()
{
	printf("\nSize of Record: %lu\n", sizeof(Record));

	printf("\nAlternative size of Record: %lu\n\n",
		sizeof(int) + NAME_SIZE + SURNAME_SIZE + ADDRESS_SIZE);

	printf("\nMax Record in hash block: %lf\n\n",
		((double) (BLOCK_SIZE - 2*sizeof(int))) / ((double) sizeof(Record)));
}

void test_4()
{
	/* Initilizing the Block file level */

	BF_Init();

	/* We create a Hash file */

	HT_CreateIndex("myHash.hash", 'i', "id", 2, 8);

	/* We open the created Hash file */

	HT_info *info = HT_OpenIndex("myHash.hash");

	FILE *fp = fopen("records1K.txt", "r");

	if(fp == NULL)
	{
		printf("\nError opening file\n");
		exit(EXIT_SUCCESS);
	}

	while(fgetc(fp) != EOF)
	{
		Record r;

		int i = 0;
		char c;
		char buf[8];
		while((c = fgetc(fp)) != ',')
			buf[i++] = c;
		buf[i] = '\0';
		r.id = atoi(buf);

		fgetc(fp);

		i = 0;
		while((c = fgetc(fp)) != '\"')
			r.name[i++] = c;
		r.name[i] = '\0';

		fgetc(fp);
		fgetc(fp);

		i = 0;
		while((c = fgetc(fp)) != '\"')
			r.surname[i++] = c;
		r.surname[i] = '\0';

		fgetc(fp);
		fgetc(fp);

		i = 0;
		while((c = fgetc(fp)) != '\"')
			r.address[i++] = c;
		r.address[i] = '\0';

		fscanf(fp, "%s", buf);

		fgetc(fp);

		HT_InsertEntry(*info, r);
	}

	fclose(fp);

	/* We retrieve all records with ID '4' */

	int keyID = 4;
	printf("\nEntries with ID = %d\n", keyID);
	int lastBlockID = HT_GetAllEntries(*info, &keyID);
	printf("Last Block where key \"%d\" was found: %d\n", keyID, lastBlockID);

	/* We retrieve all records */

	printf("\nAll entries:\n");
	HT_GetAllEntries(*info, NULL);

	/* We close the created Hash file */

	HT_CloseIndex(info);

	printf("\n");
	HashStatistics("myHash.hash");
	printf("\n");
}

void test_5()
{
	/* Initilizing the Block file level */

	BF_Init();

	/* We create a Heap file */

	HP_CreateFile("myHeap.heap", 'i', "id", 2);

	/* We open the created Heap file */

	HP_info *info = HP_OpenFile("myHeap.heap");

	FILE *fp = fopen("records1K.txt", "r");

	if(fp == NULL)
	{
		printf("\nError opening file\n");
		exit(EXIT_SUCCESS);
	}

	while(fgetc(fp) != EOF)
	{
		Record r;

		int i = 0;
		char c;
		char buf[8];
		while((c = fgetc(fp)) != ',')
			buf[i++] = c;
		buf[i] = '\0';
		r.id = atoi(buf);

		fgetc(fp);

		i = 0;
		while((c = fgetc(fp)) != '\"')
			r.name[i++] = c;
		r.name[i] = '\0';

		fgetc(fp);
		fgetc(fp);

		i = 0;
		while((c = fgetc(fp)) != '\"')
			r.surname[i++] = c;
		r.surname[i] = '\0';

		fgetc(fp);
		fgetc(fp);

		i = 0;
		while((c = fgetc(fp)) != '\"')
			r.address[i++] = c;
		r.address[i] = '\0';

		fscanf(fp, "%s", buf);

		fgetc(fp);

		HP_InsertEntry(*info, r);
	}

	fclose(fp);

	/* We retrieve all records with ID '4' */

	int keyID = 4;
	printf("\nEntries with ID = %d\n", keyID);
	int lastBlockID = HP_GetAllEntries(*info, &keyID);
	printf("Last Block where key \"%d\" was found: %d\n", keyID, lastBlockID);

	/* We retrieve all records */

	printf("\nAll entries:\n");
	HP_GetAllEntries(*info, NULL);

	/* We close the created Heap file */

	HP_CloseFile(info);
}

int main(int argc, char const *argv[])
{
	//test_4();
	test_5();

	return 0;
}
