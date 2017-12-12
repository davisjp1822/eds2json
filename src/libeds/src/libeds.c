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

#define KEY_BUF_LEN 1024
#define VAL_BUF_LEN 4096

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


ERR_LIBEDS_t convert_section2json(const PARSABLE_EDS_SECTIONS_t s_type, 
						const char * const input_buf, 
						char * const output_buf, 
						const size_t output_buf_size,
						size_t * const num_json_chars)
{
	size_t json_chars = 0;
	char section_name[128] = {0};

	/**
	*
	* Parsing of each section assumes that we are being parsed in a fresh string with no spaces or newlines.
	* We start with the first character of the string and move to the equal sign. Once we are on the equal sign, 
	* we store one side as key the other side as value. We continue to store to value of it until we get to a ;.
	* then we clear the arrays and start over.  
	*
	* Each type of EDS section initializes output_buf with the appropriate section key.
	*/

	switch(s_type)
	{
		case(EDS_FILE):
		{
			char n[] = "File";
			strncpy(section_name, n, strlen(n));
			section_name[strlen(section_name)] = '\0';
			break;
		}

		case(EDS_DEVICE):
		{
			char n[] = "Device";
			strncpy(section_name, n, strlen(n));
			section_name[strlen(section_name)] = '\0';
			break;
		}

		case(EDS_DEVICE_CLASSIFICATION):
		{
			char n[] = "DeviceClassification";
			strncpy(section_name, n, strlen(n));
			section_name[strlen(section_name)] = '\0';
			break;
		}

		case(EDS_PARAMS):
		{	
			char n[] = "Params";
			strncpy(section_name, n, strlen(n));
			section_name[strlen(section_name)] = '\0';
			break;
		}

		case(EDS_ASSEMBLY):
		{
			char n[] = "Assembly";
			strncpy(section_name, n, strlen(n));
			section_name[strlen(section_name)] = '\0';
			break;
		}

		case(EDS_CONNECTION_MANAGER):
		{

			char n[] = "ConnectionManager";
			strncpy(section_name, n, strlen(n));
			section_name[strlen(section_name)] = '\0';
			break;
		}

		case(EDS_PORT):
		{

			char n[] = "Port";
			strncpy(section_name, n, strlen(n));
			section_name[strlen(section_name)] = '\0';
			break;
		}

		case(EDS_CAPACITY):
		{
			char n[] = "Capacity";
			strncpy(section_name, n, strlen(n));
			section_name[strlen(section_name)] = '\0';
			break;
		}

		case(EDS_DLR_CLASS):
		{
			char n[] = "DLRClass";
			strncpy(section_name, n, strlen(n));
			section_name[strlen(section_name)] = '\0';
			break;
		}

		case(EDS_ETHERNET_LINK_CLASS):
		{
			char n[] = "EthernetLinkClass";
			strncpy(section_name, n, strlen(n));
			section_name[strlen(section_name)] = '\0';
			break;
		}

		// the type is unknown, so return with an error
		default:
		{
			return ERR_PARSEFAIL;
			break;
		}
	}

	// now do the heavy lifting
	size_t i = 0;
	int16_t key_i = 0;
	int16_t val_i = 0;

	bool storing_val = false;
	bool write_pair = false;
	bool output_buf_overflowed = false;

	//char sectionId[] = "\"File\":{";
	char key_buf[KEY_BUF_LEN];
	char val_buf[VAL_BUF_LEN];

	memset(key_buf, 0, KEY_BUF_LEN*sizeof(char));
	memset(val_buf, 0, VAL_BUF_LEN*sizeof(char));

	//init json output
	char sectionId[128] = {0};
	snprintf(sectionId, 128, "\"%s\":{", section_name);
	sectionId[strlen(sectionId)] = '\0';

	if(output_buf_size > strlen(sectionId))
	{
		strncpy(output_buf, sectionId, output_buf_size-1);
		output_buf[output_buf_size-1] = '\0';
		json_chars += strlen(sectionId);
	}

	// output_buf is way too small, return an error
	// but still count JSON characters
	else
	{
		json_chars += strlen(sectionId);
		output_buf_overflowed = true;
	}

	for(i=0; i < strlen(input_buf); i++)
	{
		// if storing_val is false, that means that we have not found an = yet, so store to key_buf
		if(!storing_val)
		{
			if(input_buf[i] != '=')
			{
				if(key_i < KEY_BUF_LEN)
				{
					key_buf[key_i] = input_buf[i];
					++key_i;
				}
			}
			else
			{
				storing_val = true;
			}
		}

		// if storing_val is true, we are now storing a value
		// include logic for skipping = and handling ;
		else if(storing_val)
		{
			// we shouldn't be seeing an = here, but check anyway.
			if(input_buf[i] != '=' && input_buf[i] != ';')
			{	
				if(val_i < VAL_BUF_LEN)
				{	
					// make sure to escape quotes
					/*if(input_buf[i] == '"')
					{
						val_buf[val_i] = '\\';
						++val_i;
					}*/
					if(input_buf[i] != '"')
					{
						val_buf[val_i] = input_buf[i];
						++val_i;
					}
				}	
			}

			// if we reach the ; - set storing_val to false so that we start at the top and write out the pair to
			// output_buf
			else
			{
				storing_val = false;
				write_pair = true;
			}
		}

		// once we get to the end, if write_pair was stored in "else if(storing_val)", format the key/value pair
		// as format "key_buf:val_buf," and append to output_buf.
		// also, update json_chars
		if(write_pair)
		{

			// the 11 is for the escape chars for the quotes around each, ':', and ending ',', plus \0
			size_t len = strlen(key_buf) + strlen(val_buf) + 11;
			char temp[len];
			memset(temp, 0, len*sizeof(char));

			// create the full key value pair, and add to output_buf
			// after, of course, checking to see if output_buf is large enough to hold the data
			snprintf(temp, strlen(temp)-1, "\"%s\":\"%s\",", key_buf, val_buf);

			if(output_buf_size > (json_chars + strlen(temp)))
			{
				strncat(output_buf, temp, output_buf_size-1);
				output_buf[output_buf_size-1] = '\0';
				json_chars += strlen(temp);
			}

			// if output_buf won't hold the string, update json_chars but don't copy
			// also set a flag saying that we are overflowed, so that we return properly
			else
			{
				json_chars += strlen(temp);
				output_buf_overflowed = true;
			}

			// reset all variables for key:value
			memset(key_buf, 0, KEY_BUF_LEN*sizeof(char));
			memset(val_buf, 0, VAL_BUF_LEN*sizeof(char));
			key_i = 0;
			val_i = 0;

			write_pair = false;
			storing_val = false;
		}
	}

	// finally, set json_chars and return. We are done!
	// don't forget to replace the trailing comma with a closing brace, and to add the null terminator
	if(!output_buf_overflowed)
	{
		*num_json_chars = json_chars;
		output_buf[(strlen(output_buf)-1)] = '}';
		output_buf[output_buf_size-1] = '\0';

		return 0;
	}
	else
	{
		*num_json_chars = json_chars;
		return ERR_OBUFF;
	}
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