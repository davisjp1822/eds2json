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

// got these values from EZ-EDS. The actual length is 30,000 chars - the +1 is for the null terminator
#define KEY_BUF_LEN 30001
#define VAL_BUF_LEN 30001

/**
 * @brief Handles special parsing rules for EDS file sections.
 *
 * Some EDS sections have special handling rules. This is the function that takes care of them.
 *
 * @param type EDS file section type
 * @param val_buf buffer holding value (of key:value) buffer data. Is always of size VAL_BUF_LEN
 * @param output_buf buffer holding the output of the JSON formatted value
 * @param output_buf_size size of output_buf
 * @return Success: Returns number of json chars in output_buf. If 0, then there is no special handling for this type.
 *
 */
size_t _parsing_specrules_handler(const PARSABLE_EDS_SECTIONS_t type, 
									const char * const val_buf, 
									char * const output_buf,
									const size_t output_buf_size);
/********************** 
*
* "Public" functions
*
***********************/
ERR_LIBEDS_t convert_eds2json(const char * const eds_file_path, char * const json_array, const size_t json_array_size)
{
	/**
	* This is the main caller for the EDS conversion. It takes the file from eds_file_path, opens the file,
	* locks it with flock() and starts reading character by character through the file. When we see a section header,
	* such as [Device], we parse out what type of section is about to be scanned, and then call convert_section2json
	* with the section data.
	*
	* To prep the data for parsing, this function *will* remove all spaces, tabs, and comments
	* Comments start with a $
	*
	*/

	FILE *eds_file = fopen(eds_file_path, "r");

	if(eds_file == NULL)
	{
		return ERR_EDSFILEFAIL;
	}

	/*
	// once the file is open, get a lock so that it doesn't change while we are reading it
	if(flock(fileno(eds_file), LOCK_EX | LOCK_UN) != 0)
	{
		fclose(eds_file);
		return ERR_EDSFILEFAIL;
	}*/

	// okay, the file is open, now start scanning and looking for a section header ...

	//while(!feof(eds_file))
	//{
		/** 
		* Rules of scanning:
		* 1. This function does not parse text, it just grabs sections and sends the resulting string to convert_section2json
		* 2. This function *does* remove comments. When it sees a $ that is not inside ""'s, it discards that data until seeing a \n
		* 3. The section name is what is inside of the []. The data is between ] and [.
		* 4. This function removes /r characters but leaves new lines!
		*/

		//char c = fgetc(eds_file);
	//}

	// clean-up after ourselves
	//fclose(eds_file);
	return 0;
}


ERR_LIBEDS_t convert_section2json(const PARSABLE_EDS_SECTIONS_t s_type, 
						const char * const input_buf, 
						char * const output_buf, 
						const size_t output_buf_size,
						size_t * const num_json_chars)
{
	size_t json_chars = 0;
	char *section_name = 0;

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
			char *n = "File";
			section_name = n;
			break;
		}

		case(EDS_DEVICE):
		{
			char *n = "Device";
			section_name = n;
			break;
		}

		case(EDS_DEVICE_CLASSIFICATION):
		{
			char *n = "DeviceClassification";
			section_name = n;
			break;
		}

		case(EDS_PARAMS):
		{	
			////TODO!!! What happens if there is no =?
			char *n = "Params";
			section_name = n;
			break;
		}

		case(EDS_ASSEMBLY):
		{
			char *n = "Assembly";
			section_name = n;
			break;
		}

		case(EDS_CONNECTION_MANAGER):
		{

			char *n = "ConnectionManager";
			section_name = n;
			break;
		}

		case(EDS_PORT):
		{

			char *n = "Port";
			section_name = n;
			break;
		}

		case(EDS_CAPACITY):
		{
			char *n = "Capacity";
			section_name = n;
			break;
		}

		case(EDS_DLR_CLASS):
		{
			char *n = "DLRClass";
			section_name = n;
			break;
		}

		case(EDS_ETHERNET_LINK_CLASS):
		{
			char *n = "EthernetLinkClass";
			section_name = n;
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
	int32_t key_i = 0;
	int32_t val_i = 0;

	bool storing_val = false;	/* if storing_val is false, we are parsing a key. false? parsing a value */
	bool write_pair = false;
	bool output_buf_overflowed = false;

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

	// now, actually do the parsing
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
			// if we reach the ; - set storing_val to false so that we start at the top and write out the pair to
			// output_buf
			if(input_buf[i] == ';' && input_buf[i+1] == '\n')
			{	
				storing_val = false;
				write_pair = true;

				// skip the newline char
				i++;
			}

			// if not at the end of the line, keep reading
			else
			{
				if(val_i < VAL_BUF_LEN)
				{	
					// avoid double quoting key values that are quoted in the EDS file
					// we don't want to carry over the quotation marks from the EDS file, we provide
					// them ourselves below
					if(input_buf[i] != '"')
					{
						val_buf[val_i] = input_buf[i];
						++val_i;
					}
				}
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

			// just create a pointer to a char array for now for parsing_specrules until we know
			// that there is something to put into this memory space due to special rules
			// set this pointer to null to be safe
			char *alternate_val_buf = 0;

			// if there are special rules for this section, execute them here
			// this function gets executed twice - once to get the number of resulting
			// json chars so that we can make a buffer, the second to actually write the json
			// out to the buffer
			size_t jc = 0;

			if((jc=_parsing_specrules_handler(s_type, val_buf, NULL, 0)) > 0)
			{
				char buf[jc+1];
				memset(buf, 0, (jc+1)*sizeof(char));

				_parsing_specrules_handler(s_type, val_buf, buf, jc);
				alternate_val_buf = buf;
			}

			// create the full key value pair, and add to output_buf
			if(alternate_val_buf != NULL)
			{
				snprintf(temp, strlen(temp)-1, "\"%s\":\"%s\",", key_buf, alternate_val_buf);
			}
			else
			{
				snprintf(temp, strlen(temp)-1, "\"%s\":\"%s\",", key_buf, val_buf);
			}

			// check to see if output_buf is large enough to hold the data 
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

int8_t err_string(const ERR_LIBEDS_t err_code, char * const err_string, const size_t err_string_size)
{
	// if our string is smaller than 128 characters, return -1
	if(err_string_size < 128)
	{
		return -1;
	}

	else
	{
		switch(err_code) {
		
			case ERR_OBUFF:
			{
				const char em[128] = "Error (libeds): insufficient capacity in receiving buffer";
				snprintf(err_string, err_string_size, "%s", em);

				return 0;
				break;
			}

			case ERR_PARSEFAIL:
			{
				const char em[128] = "Error (libeds): error parsing EDS section data";
				snprintf(err_string, err_string_size, "%s", em);

				return 0;
				break;
			}

			case ERR_EDSFILEFAIL:
			{
				const char em[128] = "Error (libeds): error opening EDS file";
				snprintf(err_string, err_string_size, "%s", em);

				return 0;
				break;
			}

			default:
			{
				const char em[128] = "Error (libeds): unknown error";
				snprintf(err_string, err_string_size, "%s", em);

				return -2;
				break;
			}
		}
	}
}

/********************** 
*
* "Private" functions
*
***********************/
size_t _parsing_specrules_handler(const PARSABLE_EDS_SECTIONS_t type, 
									const char * const val_buf, 
									char * const output_buf,
									const size_t output_buf_size)
{

	// all modes of operation will report back the number of json chars in the section
	size_t json_chars = 0;
	

	/**
	*
	* The Params section switches to using comma delimiters for the individual params.
	* The general structure of this section is to parse the comma delimited string into a JSON object.
	* The param field is a fixed delineation of parameters, always in the same order.
	* So, we should be able to (a) parse out the comma-separted substrings into a holding array, and then
	* (b) build JSON with the key value array and the data in the holding array.
	*
	*/
	if(type == EDS_PARAMS)
	{
		const int8_t num_params_vals = 21;
		const int8_t num_key_vals = 24;
		
		int8_t params_val_idx = 0;
		int32_t val_string_idx = 0;

		size_t i = 0;

		const char *key_names[] = {
									"Reserved", 
									"PathSize", 
									"Path", 
									"Descriptor", 
									"DataType",
									"DataSizeInBytes",
									"Name",
									"Units",
									"HelpString",
									"DataValues",
									"min",
									"max",
									"default",
									"Scaling",
									"mult",
									"div",
									"base",
									"offset",
									"Links",
									"mult",
									"div",
									"base",
									"offset",
									"DecimalPlaces"
								};

		// this array is an array of pointers to the actual data in the params
		char params_vals[num_params_vals][VAL_BUF_LEN];

		//initialize sub-strings in params_vals
		for(i=0; i < (size_t)num_params_vals; i++)
		{
			memset(params_vals[i], 0, VAL_BUF_LEN);
		}

		// now, loop through the val_buf separating out the data values into the correct spot in params_vals
		for(i=0; i < strlen(val_buf); i++)
		{
			// if not a comma (and not a quote mark), add character to correct spot in the value array
			if(val_buf[i] != ',')
			{
				if(val_buf[i] != '"' && val_buf[i] != '\n' && val_buf[i] != ';')
				{
					params_vals[params_val_idx][val_string_idx] = val_buf[i];
					++val_string_idx;
				}
			}

			// if the character is a comma, terminate the current data string and start a new section
			// also check for a null section (indicated by two commas in a row)
			else if(val_buf[i] == ',')
			{
				// reset val_string_idx and increment section
				val_string_idx = 0;
				++params_val_idx;

				// double comma, up in the sky!
				// also, look for a semicolon as this would indicate that we are at the end of the value
				// and need to handle a null value in this case as well.
				// ... then, record this value as "null"
				if((i+1 < output_buf_size) && (val_buf[i+1] == ',' || val_buf[i+1] == ';'))
				{
					if(val_buf[i+1] == ';')
					{
						const char *s = "0";
						snprintf(params_vals[params_val_idx], strlen(s)+1, "%s", s);
					}

					else
					{
						const char *s = "null";
						snprintf(params_vals[params_val_idx], strlen(s)+1, "%s", s);
					}
				}

				else
				{
					params_vals[params_val_idx][val_string_idx] = '\0';
				}
			}

			// stop parsing when we see a semicolon
			else if(val_buf[i] == ';')
			{
				break;
			}
		}

		// lastly, we now have two arrays - one with JSON key names (key_names), and the other with values (params_vals)
		// glue them together and make some lovely JSON!
		params_val_idx = 0;

		for(i=0; i < (size_t)num_key_vals; i++)
		{
			char s[VAL_BUF_LEN] = {0};
			size_t s_len = 0;

			switch(i)
			{
				case(9):
				case(13):
				case(18):
				{
					s_len = strlen(key_names[i]) + 5;
					snprintf(s, s_len, "\"%s\":{", key_names[i]);
					break;
				}

				case(12):
				case(17):
				case(22):
				{
					s_len = strlen(key_names[i]) + strlen(params_vals[params_val_idx]) + 8;
					snprintf(s, s_len, "\"%s\":\"%s\"},", key_names[i], params_vals[params_val_idx]);
					++params_val_idx;
					break;
				}

				case(23):
				{
					s_len = strlen(key_names[i]) + strlen(params_vals[params_val_idx]) + 6;
					snprintf(s, s_len, "\"%s\":\"%s\"", key_names[i], params_vals[params_val_idx]);
					++params_val_idx;
					break;
				}

				default:
				{
					s_len = strlen(key_names[i]) + strlen(params_vals[params_val_idx]) + 7;
					snprintf(s, s_len, "\"%s\":\"%s\",", key_names[i], params_vals[params_val_idx]);
					++params_val_idx;
					break;
				}

			}

			if(strlen(output_buf) + strlen(s) < output_buf_size)
			{
				strncat(output_buf, s, s_len);
			}

			json_chars += s_len-1;
		}

		return json_chars;
	}

	// default - there are no special rules for handling this section, so return 0
	else
	{
		return 0;
	}
}