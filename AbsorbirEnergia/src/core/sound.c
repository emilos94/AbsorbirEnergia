#include "sound.h"
#include <stdio.h>

#define OpenAL_ErrorCheck(message) {\
	ALenum error = alGetError();\
	if (error != AL_NO_ERROR) {\
		fprintf("OpenAL Error: %d with call for %s\n", error, message); \
	}\
}

#define alCall(function_call)\
function_call;\
OpenAL_ErrorCheck(#function_call)


void sound_initialize(void) {
	ALCchar* defaultDeviceString = alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);
	ALCdevice* device = alcOpenDevice(defaultDeviceString);

	ASSERT(device != NULL);

	ALCcontext* context = alcCreateContext(device, NULL);
	ASSERT(alcMakeContextCurrent(context));

	OpenAL_ErrorCheck("Setup OpenAl context");

	alCall(alListener3f(AL_POSITION, 0.f, 0.f, 0.f));
	alCall(alListener3f(AL_VELOCITY, 0.f, 0.f, 0.f));
	ALfloat forwardAndUpVectors[] = {
		1.0f, 0.f, 0.f, // forward
		0.f, 1.0f, 0.f // up
	};
	alCall(alListenerfv(AL_ORIENTATION, forwardAndUpVectors));
}

b8 match_descriptor(char* buffer, char* descriptor, u32 length) {
	for (u32 idx = 0; idx < length; idx++) {
		if (buffer[idx] != descriptor[idx]) return FALSE;
	}
	return TRUE;
}

SoundSource sound_source_load(MemoryArena* arena, char* path) {
	FILE* file = fopen(path, "rb");
	assert_msg(file, "Error opening audio file: %s\n", path);

	char buffer4[4] = { '0', '0', '0', '0' };

	fread(buffer4, 4, 1, file);
	ASSERT(match_descriptor(&buffer4, "RIFF", 4));

	u32 bits_per_sample = 0;
	u32 sample_rate = 0;
	u32 data_length = 0;
	char* data = 0;
	u32 read = 1;
	while (read) {
		read = fread(buffer4, sizeof(buffer4), 1, file);
		if (match_descriptor(&buffer4, "fmt", 3)) {
			u32 format_length = 0;
			fread(&format_length, 4, 1, file);

			fseek(file, 4, SEEK_CUR);
			fread(&sample_rate, 4, 1, file);
			fseek(file, 6, SEEK_CUR);
			fread(&bits_per_sample, 2, 1, file);

			u32 fmt_bytes_to_skip = format_length - 16;
			fseek(file, fmt_bytes_to_skip, SEEK_CUR);
		}
		else if (match_descriptor(&buffer4, "data", 4)) {
			fread(&data_length, 4, 1, file);
 			data = memory_AllocateArray(arena, char, data_length);
			fread(data, data_length, 1, file);
			read = 0;
		}
	}

	fclose(file);

	// We assume file is stereo
	ALuint format;
	if (bits_per_sample == 8) {
		format = AL_FORMAT_STEREO8;
	}
	else {
		format = AL_FORMAT_STEREO16;
	}

	// OpenAL
	ALuint buffer;
	alCall(alGenBuffers(1, &buffer));
	alCall(alBufferData(buffer, format, data, data_length, sample_rate));

	ALuint source;
	alCall(alGenSources(1, &source));
	alCall(alSource3f(source, AL_POSITION, 1.f, 0.f, 0.f));
	alCall(alSource3f(source, AL_VELOCITY, 0.f, 0.f, 0.f));
	alCall(alSourcef(source, AL_PITCH, 1.f));
	alCall(alSourcef(source, AL_GAIN, 1.f));
	alCall(alSourcei(source, AL_LOOPING, AL_FALSE));
	alCall(alSourcei(source, AL_BUFFER, buffer));

	arena->offset -= data_length;

	SoundSource sound_source;
	sound_source.source = source;
	sound_source.buffer = buffer;

	return sound_source;
}

void sound_volume_set(f32 volume);

void sound_play(SoundSource* source) {
	alCall(alSourcePlay(source->source));
}