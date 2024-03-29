#include "file_loader.h"


FileResult* file_ReadFileToCharArray(MemoryArena* arena, char* path)
{
	char* buffer = 0;
	u32 length;
	FILE* file = fopen(path, "rb");
	ASSERT(file);

	fseek(file, 0, SEEK_END);
	length = ftell(file);
	fseek(file, 0, SEEK_SET);

	buffer = memory_AllocateArray(arena, char, length + 1);
	ASSERT(buffer);
	fread(buffer, sizeof(char), length, file);
	
	fclose(file);
	
	buffer[length] = '\0';
	FileResult* fileResult = memory_struct_zero_allocate(arena, FileResult);
	ASSERT(fileResult);

	fileResult->text = buffer;
	fileResult->length = length;

	return fileResult;
}

FileImageResult* file_LoadImage(MemoryArena* arena, char* path)
{
	FileImageResult* fileImageResult = memory_struct_zero_allocate(arena, FileImageResult);

	unsigned char* data = stbi_load(path, &fileImageResult->width, &fileImageResult->height, &fileImageResult->nrChannels, 0);
	if (!data)
	{
		printf("Failed to load image '%s' !", path);
		ASSERT(FALSE);
	}

	void* arenaData = memory_MemoryArenaCopyBuffer(arena, data, fileImageResult->width * fileImageResult->height * fileImageResult->nrChannels);
	stbi_image_free(data);
	fileImageResult->text = arenaData;
	return fileImageResult;
}


s32 file_char_array_find_indexof(FileResult* file_result, u32 file_offset, char* text_to_find, u32 text_length)
{
	ASSERT(file_result->length >= file_offset + text_length);

	char* file_text = file_result->text + file_offset;
	while (!mystr_char_array_equals(file_text, text_to_find, text_length))
	{
		file_text++;
		file_offset++;

		if (file_offset + text_length > file_result->length)
		{
			return -1;
		}
	}

	return file_offset;
}