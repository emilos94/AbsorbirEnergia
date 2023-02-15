#include "texture.h"


Texture graphics_TextureLoad(MemoryArena* arena, char* path)
{
	FileImageResult* fileImageResult = file_LoadImage(arena, path);

	Texture texture;
	texture.width = fileImageResult->width;
	texture.height = fileImageResult->height;

	GLCall(glGenTextures(1, &texture.textureId));
	GLCall(glBindTexture(GL_TEXTURE_2D, texture.textureId));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture.width, texture.height, 0, GL_RGB, GL_UNSIGNED_BYTE, fileImageResult->fileContents));

	return texture;
}

void graphics_TextureBind(Texture* texture)
{
	GLCall(glActiveTexture(GL_TEXTURE0));
	GLCall(glBindTexture(GL_TEXTURE_2D, texture->textureId));
}

void graphics_TextureDestroy(Texture* texture)
{
	GLCall(glDeleteTextures(1, &texture->textureId));
}
