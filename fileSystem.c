#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "fs.h"

FILE* filePtr;
struct partition* partition;
struct dentry* dentry;

void mountDisk(void);
void ls(void);
void openFile(void);
void readFile(void);
void closeDisk(void);

void main()
{
	mountDisk();
	ls();
	openFile();
	readFile();
	closeDisk();
}

void mountDisk(void)
{
	int i;
	partition = malloc(sizeof(struct partition));

	//linking file pointer
	filePtr = fopen("./disk.img", "rw");
	if(filePtr == NULL)
	{
		printf("Could not open file\n");
		exit(0);
	}

	//populating partition superblock
	fread(&partition->s.partition_type, sizeof(int), 1, filePtr);
	fread(&partition->s.block_size, sizeof(int), 1, filePtr);
	fread(&partition->s.inode_size, sizeof(int), 1, filePtr);
	fread(&partition->s.first_inode, sizeof(int), 1, filePtr);

	fread(&partition->s.num_inodes, sizeof(int), 1, filePtr);
	fread(&partition->s.num_inode_blocks, sizeof(int), 1, filePtr);
	fread(&partition->s.num_free_inodes, sizeof(int), 1, filePtr);

	fread(&partition->s.num_blocks, sizeof(int), 1, filePtr);
	fread(&partition->s.num_free_blocks, sizeof(int), 1, filePtr);
	fread(&partition->s.first_data_block, sizeof(int), 1, filePtr);
	fread(&partition->s.volume_name, sizeof(char), 24, filePtr);
	fread(&partition->s.padding, sizeof(char), 960, filePtr);
	printf("---SuperBlock Information---\n");
	printf("Partition type:--------- %d\n", partition->s.partition_type);
	printf("Block size:------------- %d\n", partition->s.block_size);
	printf("Inode size:------------- %d\n", partition->s.inode_size);
	printf("Directory file inode:--- %d\n", partition->s.first_inode);
	printf("Number of inodes:------- %d\n", partition->s.num_inodes);
	printf("Number of inode blocks:- %d\n", partition->s.num_inode_blocks);
	printf("Number of free inodes:-- %d\n", partition->s.num_free_inodes);
	printf("Number of blocks:------- %d\n", partition->s.num_blocks);
	printf("Number of free blocks:-- %d\n", partition->s.num_free_blocks);
	printf("First data block:------- %d\n", partition->s.first_data_block);
	printf("Partition name:--------- %s\n", partition->s.volume_name);
	printf("\n");

	//populating partition inode_table
	for(i = 0; i < 224; i++)
	{
		fread(&partition->inode_table[i].mode, sizeof(int), 1, filePtr);
		fread(&partition->inode_table[i].locked, sizeof(int), 1, filePtr);
		fread(&partition->inode_table[i].date, sizeof(int), 1, filePtr);
		fread(&partition->inode_table[i].size, sizeof(int), 1, filePtr);
		fread(&partition->inode_table[i].indirect_block, sizeof(int), 1, filePtr);
		fread(&partition->inode_table[i].blocks, sizeof(short), 6, filePtr);
		printf("---inode #%d---\n", i);
		printf("Mode:----------- %d\n", partition->inode_table[i].mode);
		printf("Locked:--------- %d\n", partition->inode_table[i].locked);
		printf("Date:----------- %d\n", partition->inode_table[i].date);
		printf("File Size:------ %d\n", partition->inode_table[i].size);
		printf("Indirect block:- %d\n", partition->inode_table[i].indirect_block);
		printf("Blocks:--------- ");
		for(int j = 0; j < 6; j++)
			printf("%d ", partition->inode_table[i].blocks[j]);
		printf("\n\n");
	}
}

void ls(void)
{
	printf("---ls---\n");
	int blockAddr;
	int blockCntr;
	int dentryCntr;
	int blockNum = (partition->inode_table[partition->s.first_inode].size + 0x3ff) / BLOCK_SIZE;

	for(blockCntr = 0; blockCntr < blockNum; blockCntr++)
	{
		blockAddr = (BLOCK_SIZE * partition->s.first_data_block) + (partition->inode_table[partition->s.first_inode].blocks[blockCntr] * BLOCK_SIZE);
		fseek(filePtr, blockAddr, SEEK_SET);
		for(dentryCntr = 0; dentryCntr < 1024; dentryCntr += 32)
		{
			dentry = malloc(sizeof(struct dentry));
			fread(&dentry->inode, sizeof(int), 1, filePtr);
			fread(&dentry->dir_length, sizeof(int), 1, filePtr);
			fread(&dentry->name_len, sizeof(int), 1, filePtr);
			fread(&dentry->file_type, sizeof(int), 1, filePtr);
			fread(&dentry->name, sizeof(char), 16, filePtr);
			printf("%s\n", dentry->name);
			free(dentry);
		}
	}
	return;
}

void openFile(void)
{
	char input[256];
	gets(input);
	int blockAddr;
	int blockCntr;
	int dentryCntr;
	int blockNum = (partition->inode_table[partition->s.first_inode].size + 0x3ff) / BLOCK_SIZE;

	for(blockCntr = 0; blockCntr < blockNum; blockCntr++)
	{
		blockAddr = (BLOCK_SIZE * partition->s.first_data_block) + (partition->inode_table[partition->s.first_inode].blocks[blockCntr] * BLOCK_SIZE);
		fseek(filePtr, blockAddr, SEEK_SET);
		for(dentryCntr = 0; dentryCntr < BLOCK_SIZE; dentryCntr += 32)
		{
			dentry = malloc(sizeof(struct dentry));
			fread(&dentry->inode, sizeof(int), 1, filePtr);
			fread(&dentry->dir_length, sizeof(int), 1, filePtr);
			fread(&dentry->name_len, sizeof(int), 1, filePtr);
			fread(&dentry->file_type, sizeof(int), 1, filePtr);
			fread(&dentry->name, sizeof(char), 16, filePtr);
			if(strcmp(input, dentry->name) == 0)
				return;
			free(dentry);
		}
	}
	printf("No such file\n");
	closeDisk();
}

void readFile(void)
{
	char data[BLOCK_SIZE];
	int blockCntr;
	int blockAddr;
	int blockNum = (partition->inode_table[partition->s.first_inode].size + 0x3ff) / BLOCK_SIZE;
	printf("File name:-------- %s\n", dentry->name);
	printf("Inode number:----- %d\n", dentry->inode);
	printf("Dentry Size:------ %d\n", dentry->dir_length);
	printf("File name length:- %d\n", dentry->name_len);
	printf("File Type:-------- %d\n", dentry->file_type);
	printf("Mode:------------- %d\n", partition->inode_table[dentry->inode].mode);
	printf("Locked:----------- %d\n", partition->inode_table[dentry->inode].locked);
	printf("Date:------------- %d\n", partition->inode_table[dentry->inode].date);
	printf("File Size:-------- %d\n", partition->inode_table[dentry->inode].size);
	printf("Indirect block:--- %d\n", partition->inode_table[dentry->inode].indirect_block);
	printf("Blocks:----------- ");
	for(int j = 0; j < 6; j++)
		printf("%d ", partition->inode_table[dentry->inode].blocks[j]);
	printf("\n");
	printf("Data Content:----- ");
	for(blockCntr = 0; blockCntr < blockNum; blockCntr++)
	{
		blockAddr = (BLOCK_SIZE * partition->s.first_data_block) + (partition->inode_table[dentry->inode].blocks[blockCntr] * BLOCK_SIZE);
		fseek(filePtr, blockAddr, SEEK_SET);
		fread(&data, sizeof(char), BLOCK_SIZE, filePtr);
		printf("%s", data);
	}
	return;
}

void closeDisk(void)
{
	fclose(filePtr);
	free(partition);
	exit(0);
}
