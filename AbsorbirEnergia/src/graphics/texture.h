#ifndef TEXTURE_H
#define TEXTURE_H

#include <GLAD/glad.h>
#include "../core/definitions.h"
#include "../core/memory_arena.h"
#include "../core/file_loader.h"
#include "opengl_utils.h"

struct Texture
{
	GLuint textureId;
	u32 width, height;
};
typedef struct Texture Texture;

Texture graphics_TextureLoad(MemoryArena* arena, char* path);
void graphics_TextureBind(Texture* texture);
void graphics_TextureDestroy(Texture* texture);

#endif // !TEXTURE_H
