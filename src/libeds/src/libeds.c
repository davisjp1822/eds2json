/**

	@file libeds.c
	@author John Davis <jd@three-ml.com>
	@date 30 Nov 2017
	@brief libeds - an open source ODVA EDS file parser.

	Copyright 2017 3ML LLC (www.three-ml.com)

	Licensed under the Apache License, Version 2.0 (the "License");
	you may not use this file except in compliance with the License.
	You may obtain a copy of the License at

	 http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.

**/

#include "libeds.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>

/**
* 
* @brief The following function parses EDS file sections as defined by PARSABLE_EDS_SECTIONS_t.
* 
* The function is passed the char buffer (input_buf) that is in between the closing ] of the header and the starting [
* of the next header.
*
* Once parsed, they write the JSON string to output_buf and return the size of output_buf.
* 
* @param s_type EDS section as defined by PARSABLE_EDS_SECTIONS_t
* @param input_buf Pointer to array of char that is the data between  the closing ']'of the header and the starting '[' of the next header.
* @param output_buf Pointer to array of char that will hold the output JSON.
* @param output_buf_size Size of the output buffer. Should *always* be considerably larger than sizeof(input_buf)!
* @note output_buf_size should ALWAYS be larger than sizeof(input_buf) by a considerable margin!!
*
* @return Success: number of characters in output_buf.
* @return Fail: ERR_OBUFF if output_buf not large enough
* @return Fail: ERR_PARSEFAIL if input_buf is not a valid input string
*
*/
uint32_t _parse_file(const PARSABLE_EDS_SECTIONS_t s_type, const char * const input_buf, char * const output_buf, const size_t output_buf_size); 

/** 
*
* "Public" functions
*
*/

void convert_eds2json(const char * const eds_file_path, char * const json_array, const size_t json_array_size)
{

}

char ** get_unparsed_sections()
{
	char *foo = "foobar";

	return &foo;
}

void err_string(const ERR_LIBEDS_t err_code, char * const err_string)
{
	char retString[128] = {0};

	switch(err_code) {
		
		case ERR_OBUFF:
		{
			char *em = "Error (libeds): insufficient capacity in receiving buffer";
			strncpy(em, retString, sizeof(em));
			retString[(sizeof(retString-1))] = '\0';
			break;
		}

		case ERR_PARSEFAIL:
		{
			char *em = "Error (libeds): error parsing EDS section data";
			strncpy(em, retString, sizeof(em));
			retString[(sizeof(retString-1))] = '\0';
			break;
		}


		default:
		{
			char *em = "Error (libeds): unknown error";
			strncpy(em, retString, sizeof(em));
			retString[(sizeof(retString-1))] = '\0';
			break;
		}
	}
}

/** 
*
* "Private" functions
*
*/
