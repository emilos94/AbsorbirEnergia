#ifndef FILE_LOADER_H
#define FILE_LOADER_H

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS // enables use of fopen
#endif

#include "memory_arena.h"
#include <stb/stb_image.h>

#include <stdio.h>

struct FileResult
{
	char* fileContents;
	U32 length;
};
typedef struct FileResult FileResult;

struct FileImageResult
{
	char* fileContents;
	U32 width, height, nrChannels;
};
typedef struct FileImageResult FileImageResult;

FileResult* file_ReadFileToCharArray(MemoryArena* arena, char* path);
FileImageResult* file_LoadImage(MemoryArena* arena, char* path);

#endif