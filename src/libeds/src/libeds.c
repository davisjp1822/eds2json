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
#include <sys/file.h>
#include <stdbool.h>

/** 
*
* "Public" functions
*
*/

ERR_LIBEDS_t convert_eds2json(const char * const eds_file_path, char * const json_array, const size_t json_array_size)
{
	/**
	* This is the main caller for the EDS conversion. It takes the file from eds_file_path, opens the file,
	* locks it with flock() and starts reading character by character through the file. When we see a section header,
	* such as [Device], we parse out what type of section is about to be scanned, and then call convert_section2json
	* with the section data.
	*
	* convert_section2json returns a uint32_t which tells us the number of characters being returned. If json_array
	* can't fit the returned data, we return ERR_OBUFF.
	*/

	FILE *eds_file = fopen(eds_file_path, "r");

	if(eds_file == NULL)
	{
		fclose(eds_file);
		return ERR_EDSFILEFAIL;
	}

	// once the file is open, get a lock so that it doesn't change while we are reading it
	if(flock(fileno(eds_file), LOCK_EX | LOCK_UN) != 0)
	{
		fclose(eds_file);
		return ERR_EDSFILEFAIL;
	}

	// okay, the file is open, now start scanning and looking for a section header ...

	//while(!feof(eds_file))
	//{
		/** 
		* Rules of scanning:
		* 1. This function does not parse text, it just grabs sections and sends the resulting string to convert_section2json
		* 2. This function *does* remove comments. When it sees a $ that is not inside ""'s, it discards that data until seeing a \n
		* 3. The section name is what is inside of the []. The data is between ] and [.
		* 4. This function removes all newlines and other non-printable characters.
		*/

		char c = fgetc(eds_file);
	//}

	// clean-up after ourselves
	fclose(eds_file);
	return 0;
}


uint32_t convert_section2json(const PARSABLE_EDS_SECTIONS_t s_type, 
						const char * const input_buf, 
						char * const output_buf, 
						const size_t output_buf_size)
{
	return 0;
}

uint32_t get_unparsed_sections()
{
	return 0;
}

int8_t err_string(const ERR_LIBEDS_t err_code, char * const err_string, const size_t err_string_size)
{
	// if our string is smaller than 128 characters, return -1
	if(err_string_size < 128)
	{
		return -1;
	}

	else
	{
		char retString[128] = {0};

		switch(err_code) {
		
			case ERR_OBUFF:
			{
				char em[] = "Error (libeds): insufficient capacity in receiving buffer";
				strncpy(em, retString, strlen(em));
				retString[(sizeof(retString-1))] = '\0';

				strncpy(err_string, retString, strlen(retString+1));

				return 0;
				break;
			}

			case ERR_PARSEFAIL:
			{
				char em[] = "Error (libeds): error parsing EDS section data";
				strncpy(em, retString, strlen(em));
				retString[(sizeof(retString-1))] = '\0';

				strncpy(err_string, retString, strlen(retString+1));

				return 0;
				break;
			}

			case ERR_EDSFILEFAIL:
			{
				char em[] = "Error (libeds): error opening EDS file";
				strncpy(em, retString, strlen(em));
				retString[(sizeof(retString-1))] = '\0';

				strncpy(err_string, retString, strlen(retString+1));

				return 0;
				break;
			}

			default:
			{
				char em[] = "Error (libeds): unknown error";
				strncpy(em, retString, strlen(em));
				retString[(sizeof(retString-1))] = '\0';

				strncpy(err_string, retString, strlen(retString+1));

				return -2;
				break;
			}
		}
	}
}

/** 
*
* "Private" functions
*
*/