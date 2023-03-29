#include "mystr.h"

Mystr* mystr_create(MemoryArena* arena, char* text, u32 length)
{
	Mystr* str = memory_struct_zero_allocate(arena, Mystr);
	str->text = memory_MemoryArenaCopyBuffer(arena, text, length);
	str->length = length;
	return str;
}

b8 mystr_equals(Mystr* left, Mystr* right)
{
	b8 same_length = left->length == right->length;
	if (same_length)
	{
		for (u32 i = 0; i < left->length; i++)
		{
			if (left->text[i] != right->text[i])
			{
				return FALSE;
			}
		}

		return TRUE;
	}

	return FALSE;
}

b8 mystr_char_array_equals(char* either, char* other, u32 length)
{
	for (u32 i = 0; i < length; i++)
	{
		if (either[i] != other[i])
		{
			return FALSE;
		}
	}

	return TRUE;
}


b8 mystr_u32_parse(char* char_arr, u32* destination)
{
	s32 index = 0;
	b8 is_number = TRUE;

	u32 ascii_zero = (u32)'0';
	u32 ascii_nine = (u32)'9';

	while (is_number)
	{
		u32 number = (u32)char_arr[index];
		is_number = number >= ascii_zero && number <= ascii_nine;
		if (is_number)
		{
			index++;
		}
		else
		{
			index--;
		}
	}

	b8 first_char_not_number = index == -1;
	if (first_char_not_number)
	{
		return FALSE;
	}

	u32 tens_modifier = 1;
	u32 result = 0;
	for (s32 i = index; i >= 0; i--)
	{
		u32 char_number_value = (u32)char_arr[i] - ascii_zero;
		result += tens_modifier * char_number_value;
		tens_modifier *= 10;
	}

	destination[0] = result;
	return TRUE;
}

b8 mystr_char_is_number(char c)
{
	u32 ascii_value = (u32)c;
	return ascii_value >= (u32)'0' && ascii_value <= (u32)'9';
}

b8 mystr_s32_parse(char* char_arr, s32* destination)
{
	s32 index = 0;
	b8 is_number = TRUE;

	u32 ascii_zero = (u32)'0';
	u32 ascii_nine = (u32)'9';

	b8 is_negative = char_arr[0] == '-';
	if (is_negative)
	{
		index++;
		
		if (!mystr_char_is_number(char_arr[1]))
		{
			return FALSE;
		}
	}

	while (is_number)
	{
		is_number = mystr_char_is_number(char_arr[index]);
		if (is_number)
		{
			index++;
		}
		else
		{
			index--;
		}
	}

	u32 tens_modifier = 1;
	s32 result = 0;
	for (s32 i = index; i >= 0; i--)
	{
		u32 char_number_value = (u32)char_arr[i] - ascii_zero;
		result += tens_modifier * char_number_value;
		tens_modifier *= 10;
	}

	if (is_negative)
	{
		result *= -1;
	}

	destination[0] = result;
	return TRUE;
}


s32 mystr_char_array_find_indexof(char* arr_to_search, u32 arr_text_length, char* search_text, u32 search_text_length)
{
	ASSERT(arr_text_length >= search_text_length);

	s32 offset = 0;
	while (!mystr_char_array_equals(arr_to_search + offset, search_text, search_text_length))
	{
		offset++;

		if (offset + search_text_length > arr_text_length)
		{
			return -1;
		}
	}

	return offset;
}
