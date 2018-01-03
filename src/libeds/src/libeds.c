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
 * @brief Handles special parsing rules for EDS file sections.
 *
 * Some EDS sections have special handling rules. This is the function that takes care of them.
 *
 * @param type EDS file section type
 * @param val_buf buffer holding value (of key:value) buffer data. Is always of size VAL_BUF_LEN
 * @param output_buf buffer holding the output of the JSON formatted value
 * @param output_buf_size size of output_buf
 * @param json_chars number of json chars returned
 * @return Success: 0
 * @return Fail: error code of type ERR_LIBEDS_t
 * @note If there are no special rules for handling this section, json_chars is set to 0 and the function returns 0
 *
 */
ERR_LIBEDS_t _parse_comma_delimited_val(const SPECIAL_DATA_TYPES_t type, 
											const char * const val_buf, 
											char * const output_buf,
											const size_t output_buf_size,
											size_t *json_chars);

/**
 * @brief Creates a JSON object from the EDS Key=value; string.
 *
 * This function is the first stage of processing for EDS data sections. For many sections,
 * such as File and Device, the output from this function is good enough to be used standalone.
 * For special sections, such as Params, the output from this function needs to be fed into 
 * _parsing_special_key_value_to_JSON so that further processing may be done.
 *
 * @note key_buf and value_buf are constants defined by KEY_BUF_LEN and LARGE_BUF, respectively
 * @param input_buf formatted input string with the section data. Processing is done in eds2json()
 * @param key_buf buffer of size KEY_BUF_LEN to hold the output key name
 * @param value_buf buffer of size LARGE_BUF to hold the output value data
 * @param json_chars number of json chars returned
 * @param return_type the type of data returned as specified in SPECIAL_DATA_TYPES_t
 * @return Success: 0
 * @return Fail: error code of type ERR_LIBEDS_t
 *
 */
ERR_LIBEDS_t _parse_eds_keyval(const char * const input_buf,
								char * const key_buf,
								char * const val_buf,
								char * const output_buf, 
								const size_t output_buf_size,
								size_t *json_chars);

/**
* @brief Provides a PARSABLE_EDS_SECTIONS_t for the string section_name
*
* @param section_name Name of the section copied verbatim from the EDS file, such as File or Device
* @return SUCCESS: value corresponding to enum PARSABLE_EDS_SECTIONS_t
* @return FAIL: 0
*
*/
PARSABLE_EDS_SECTIONS_t _section_enum_from_section_name(const char * const section_name);

/********************** 
*
* Header Scope functions
*
***********************/
ERR_LIBEDS_t convert_eds2json(const char * const eds_file_path, 
								char * const json_array, 
								const size_t json_array_size,
								size_t *json_chars)
{
	/**
	* 
	* This is the main caller for the EDS conversion. It takes the file from eds_file_path, opens the file,
	* locks it with flock() and starts reading character by character through the file. When we see a section header,
	* such as [Device], we parse out what type of section is about to be scanned, and then call convert_section2json
	* with the section data.
	*
	* The general operation is as follows:
	*
	* 1. Once the file is open, now start scanning and looking for a section header.
	* 2. Once a section header is found, compare it to eds_parsable_section_names to see if it is something that can be parsed.
	* 3. Store the section text in a temp buffer of size LARGE_BUF until the section is read complete, and then call on convert_section2json to 
	* 		do the actual conversion
	*
	* 4. Concatenate the result from convert_section2json() to json_array.
	*
	* ===PARSING RULES===
	* 1. Remove all tabs and spaces not inside of quotes!
	* 2. Leave newlines intact (but remove \r and other non-printables)!
	* 3. Remove comments ($---->\n) but leave $'s inside of quotes alone!
	* 4. Remove spaces before and after = signs!
	* 5. Section names are always enclosed in []'s
	* 6. Section data is always between ] and [
	*
	*/

	typedef enum
	{	
		// initial state - reading char by char until a name is found
		eds_file_parsing_searching,

		// parsing the name of the section (in between the brackets), e.g. [Foo]
		eds_file_parsing_section_name,

		// parsing the section contents
		eds_file_parsing_section_contents,

		// still parsing the section contents, but now reading inside a pair of quotes
		// basically, we allow spaces, tabs, and all manner of other symbols
		eds_file_parsing_section_contents_inside_quotes,

		// currently parsing a comment - skip everything between $ and \n
		eds_file_parsing_comment,

	} file_parsing_state_t;

	PARSABLE_EDS_SECTIONS_t section_type = EDS_FILE;

	bool output_buf_overflowed = false;
	file_parsing_state_t current_state = eds_file_parsing_searching;
	file_parsing_state_t previous_state = eds_file_parsing_searching;
	
	char section_name_buf[EDS_SECTION_NAME_LEN] = {0};
	size_t section_name_buf_idx = 0;

	char section_data_buf[LARGE_BUF] = {0};
	size_t section_data_buf_idx = 0;

	// open the file
	FILE *eds_file = fopen(eds_file_path, "r");
	if(eds_file == NULL)
	{
		return ERR_EDSFILENOTFOUND;
	}

	
	// once the file is open, get a lock so that it doesn't change while we are reading it
	if(flock(fileno(eds_file), LOCK_EX) != 0)
	{
		fclose(eds_file);
		return ERR_EDSFILEINUSE;
	}

	// file is opened and ready to go. start parsing!
	while(true)
	{
		char c = fgetc(eds_file);

		switch(current_state)
		{
			case(eds_file_parsing_searching):
			{
				if (c == '$')
				{
					previous_state = current_state;
					current_state = eds_file_parsing_comment;
				}
				else if(c == '[')
				{
					current_state = eds_file_parsing_section_name;
				}
				else
				{	
					current_state = eds_file_parsing_searching;
				}

				break;
			}

			case(eds_file_parsing_section_name):
			{
				// once at the end ], compare section_name_buf to eds_parsable_section_names to see
				// if we should process this name
				bool section_match = false;

				// while not at the end of the name, read and save char to the buffer
				if(c != ']')
				{
					if(strlen(section_name_buf)+1 < EDS_SECTION_NAME_LEN)
					{
						section_name_buf[section_name_buf_idx] = c;
						++section_name_buf_idx;
					}

					else
					{
						output_buf_overflowed = true;
					}

					current_state = eds_file_parsing_section_name;
				}

				if(c == ']')
				{
					size_t i = 0;
					size_t len = sizeof(eds_parsable_section_names)/sizeof(char *);

					for(i=0; i< len; i++)
					{
						int8_t result = strncmp(eds_parsable_section_names[i], section_name_buf, strlen(eds_parsable_section_names[i]));
						
						// if we find a comparison, proceed to parsing contents
						if(result == 0)
						{
							section_match = true;

							if(strlen(section_name_buf)+1 < EDS_SECTION_NAME_LEN)
							{
								section_name_buf[strlen(section_name_buf)] = '\0';
							}

							else
							{
								section_name_buf[strlen(section_name_buf)-1] = '\0';
								output_buf_overflowed = true;
							}
					
							break;
						}

						// if no match, revert to searching
						//TODO: test if [ in EDS variable name
						else
						{
							section_match = false;
						}
					}

					// evaluate and set state based on result of section matching
					if(section_match)
					{
						current_state = eds_file_parsing_section_contents;
					}
					else
					{
						//reset all variables
						memset(section_name_buf, 0, EDS_SECTION_NAME_LEN*sizeof(char));
						memset(section_data_buf, 0, LARGE_BUF*sizeof(char));
						section_name_buf_idx = 0;
						section_data_buf_idx = 0;

						current_state = eds_file_parsing_searching;
					}
				}

				break;
			}

			case(eds_file_parsing_section_contents):
			{
				switch(c)
				{
					// this is the end of this section and the beginning of the next
					// convert this section and then restart
					case('['):
					{
						//+2 is for null terminating chars
						size_t len = strlen(section_name_buf) + strlen(section_data_buf) + 2;

						if(len < json_array_size)
						{
							// these are helpful for debugging
							//strncat(json_array, section_name_buf, strlen(section_name_buf));
							//strncat(json_array, section_data_buf, strlen(section_data_buf));
							section_type = _section_enum_from_section_name(section_name_buf);

							ERR_LIBEDS_t err = convert_section2json(section_type, section_data_buf, json_array, json_array_size, json_chars);

							if(err != 0)
							{
								return err;
							}
						}

						else
						{
							output_buf_overflowed = true;
						}

						//reset all variables
						memset(section_name_buf, 0, EDS_SECTION_NAME_LEN*sizeof(char));
						memset(section_data_buf, 0, LARGE_BUF*sizeof(char));
						section_name_buf_idx = 0;
						section_data_buf_idx = 0;
						section_type = EDS_FILE;

						current_state = eds_file_parsing_section_name;
						break;
					}

					// if starting a quoted section, move states
					case('"'):
					{
						current_state = eds_file_parsing_section_contents_inside_quotes;
						break;
					}

					// if starting a comment, move to that state
					case('$'):
					{
						previous_state = current_state;
						current_state = eds_file_parsing_comment;
						break;
					}

					// always insert a newline after a ; (if there isn't already one)
					case(';'):
					{
						size_t idx = 0;
						if(section_data_buf_idx > 0)
						{
							++idx;
						}

						if(section_data_buf[section_data_buf_idx-idx] == '\n')
						{
							break;
						}

						else
						{
							if(strlen(section_data_buf)+2 < LARGE_BUF)
							{
								section_data_buf[section_data_buf_idx] = ';';
								++section_data_buf_idx;

								section_data_buf[section_data_buf_idx] = '\n';
								++section_data_buf_idx;
							}
							else
							{
								output_buf_overflowed = true;
							}
						}

						current_state = eds_file_parsing_section_contents;
						break;
					}

					// new lines only come after a ;
					case('\n'):
					{
						break;
					}

					// ignore these characters
					case('\r'):
					case('\t'):
					{
						break;
					}

					// no other conditions met, just add the character to our output buffer
					default:
					{
						// if c is a space, but outside of quotes, then ignore it
						if(c != ' ')
						{
							if(strlen(section_data_buf)+1 < LARGE_BUF)
							{
								section_data_buf[section_data_buf_idx] = c;
								++section_data_buf_idx;
							}
							else
							{
								output_buf_overflowed = true;
							}

							current_state = eds_file_parsing_section_contents;
							break;
						}

						else
						{
							current_state = eds_file_parsing_section_contents;
							break;
						}
					}
				}

				break;
			}

			case(eds_file_parsing_section_contents_inside_quotes):
			{
				if(c != '"' && c != '\n' && c != '\r' && c != ',' && c != '\t')
				{
					if(strlen(section_data_buf)+1 < LARGE_BUF)
					{
						section_data_buf[section_data_buf_idx] = c;
						++section_data_buf_idx;
					}
					else
					{
						output_buf_overflowed = true;
					}
				}
				
				if(c == '"')
				{
					current_state = eds_file_parsing_section_contents;
				}

				break;
			}

			case(eds_file_parsing_comment):
			{
				if(c == '\n')
				{
					current_state = previous_state;
					break;
				}

				else
				{
					current_state = eds_file_parsing_comment;
					break;
				}

				break;
			}

			default:
			{
				return ERR_PARSEFAIL;
				break;
			}
		}

		// if we get to the end of the file, we have to process whatever was in the buffer when we got here.
		// otherwise, the loop stops at case(eds_file_parsing_section_contents) b/c it can't find a starting '[', and whatever
		// is in the buffer is not processed
		if(feof(eds_file))
		{
			if(strlen(section_data_buf) > 0)
			{
				size_t len = strlen(section_name_buf) + strlen(section_data_buf) + 2;

				if(len < json_array_size)
				{
					section_type = _section_enum_from_section_name(section_name_buf);

					ERR_LIBEDS_t err = convert_section2json(section_type, section_data_buf, json_array, json_array_size, json_chars);

					if(err != 0)
					{
						return err;
					}
				}

				else
				{
					output_buf_overflowed = true;
				}

				//reset all variables
				memset(section_name_buf, 0, EDS_SECTION_NAME_LEN*sizeof(char));
				memset(section_data_buf, 0, LARGE_BUF*sizeof(char));
				section_name_buf_idx = 0;
				section_data_buf_idx = 0;
				section_type = EDS_FILE;
			}
			
			// kill the while loop
			break;
		}
	}

	// uh-oh. buffer overflow. return an error
	if(output_buf_overflowed)
	{
		fclose(eds_file);
		return ERR_OBUFF;
	}

	// SUCCESS: clean-up after ourselves
	fclose(eds_file);
	return 0;
}


ERR_LIBEDS_t convert_section2json(const PARSABLE_EDS_SECTIONS_t s_type, 
									const char * const input_buf, 
									char * const output_buf, 
									const size_t output_buf_size,
									size_t * const num_json_chars)
{
	char *section_key = 0;
	char section_name[EDS_SECTION_NAME_LEN] = {0};

	// generic error var
	ERR_LIBEDS_t err = 0;

	// according to EZ-EDS the max value for keys and values is 30,000 characters, so we init some
	// large buffers here...
	char key_buf[KEY_BUF_LEN];
	char val_buf[VAL_BUF_LEN];

	memset(key_buf, 0, KEY_BUF_LEN*sizeof(char));
	memset(val_buf, 0, VAL_BUF_LEN*sizeof(char));

	/**
	*
	* The actual JSON is created by the _parser* helper functions. This calling function just stitches everything together, 
	* giving it a JSON section name and handling the braces.
	*
	*/

	switch(s_type)
	{
		case(EDS_FILE):
		{
			char *n = "File";
			section_key = n;
			break;
		}

		case(EDS_DEVICE):
		{
			char *n = "Device";
			section_key = n;
			break;
		}

		case(EDS_DEVICE_CLASSIFICATION):
		{
			char *n = "DeviceClassification";
			section_key = n;
			break;
		}

		case(EDS_PARAMS):
		{	
			char *n = "Params";
			section_key = n;
			break;
		}

		case(EDS_ASSEMBLY):
		{
			char *n = "Assembly";
			section_key = n;
			break;
		}

		case(EDS_CONNECTION_MANAGER):
		{

			char *n = "ConnectionManager";
			section_key = n;
			break;
		}

		case(EDS_PORT):
		{

			char *n = "Port";
			section_key = n;
			break;
		}

		case(EDS_CAPACITY):
		{
			char *n = "Capacity";
			section_key = n;
			break;
		}

		case(EDS_DLR_CLASS):
		{
			char *n = "DLRClass";
			section_key = n;
			break;
		}

		case(EDS_ETHERNET_LINK_CLASS):
		{
			char *n = "EthernetLinkClass";
			section_key = n;
			break;
		}

		// the type is unknown, so return with an error
		default:
		{
			return ERR_PARSEFAIL;
			break;
		}
	}

	/** 
	 * SECTION PARSING STEP 1
	 * ======================
	 * Start the JSON section object
	 * if the output buffer is not large enough to hold at least the section name,
	 * set the error state but still return the appropriate amount of json_chars
	 *
	 */
	if(strlen(output_buf) > 0)
	{
		snprintf(section_name, EDS_SECTION_NAME_LEN, ",\"%s\":{", section_key);
	}
	else
	{
		snprintf(section_name, EDS_SECTION_NAME_LEN, "\"%s\":{", section_key);
	}

	// test buffer size here. if this fails, continue, b/c we still want to return the proper number of json chars
	if(output_buf_size > strlen(section_name))
	{
		strncat(output_buf, section_name, EDS_SECTION_NAME_LEN);
	}
	
	*num_json_chars += strlen(section_name);

	/** 
	 * SECTION PARSING STEP 2
	 * ======================
	 * Call _parse_eds_keyval() to get val_buf and key_buf
	 *
	 */
	err = _parse_eds_keyval(input_buf, 
							key_buf, 
							val_buf, 
							output_buf, 
							output_buf_size,
							num_json_chars);

	/** 
	 * SECTION PARSING STEP 3
	 * ======================
	 * Close the JSON and return
	 * If there was any type of error in the call chain, don't write to output_buf
	 * Just return the number of calculated JSON characters (which has been calculated the entire way)
	 */

	if(err != 0)
	{
		return err;
	}

	else
	{
		// num_json_chars doesn't need to be incremented for the closing } b/c the string always ends with a 
		// comma (from _parse_eds_keyval()), so the replacement is a one-for-one swap that doesn't
		// affect the count
		output_buf[(strlen(output_buf)-1)] = '}';
		output_buf[output_buf_size-1] = '\0';
		return 0;
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

			case ERR_EDSFILENOTFOUND:
			{
				const char em[128] = "Error (libeds): EDS file not found";
				snprintf(err_string, err_string_size, "%s", em);

				return 0;
				break;
			}

			case ERR_EDSFILEINUSE:
			{
				const char em[128] = "Error (libeds): EDS file in use";
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
* Local Scope functions
*
***********************/
ERR_LIBEDS_t _parse_eds_keyval(const char * const input_buf,
								char * const key_buf,
								char * const val_buf,
								char * const output_buf, 
								const size_t output_buf_size,
								size_t *json_chars)
{
	size_t i = 0;
	int32_t key_i = 0;
	int32_t val_i = 0;
	bool output_buf_overflowed = false;
	SPECIAL_DATA_TYPES_t spec_type = DATATYPE_SPEC_NONE;

	// this needs to be large enough to hold a full length key:val + 10 JSON chars that are added in write_pair
	char temp_out_buf[VAL_BUF_LEN+KEY_BUF_LEN+10] = {0};

	//if storing_val is false, we are parsing a key. false? parsing a value
	bool storing_val = false;
	bool write_pair = false;

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
					if(input_buf[i] != '\n')
					{
						key_buf[key_i] = input_buf[i];
						++key_i;
					}
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
				++i;
			}

			// if not at the end of the line, keep reading
			else
			{
				if(val_i < VAL_BUF_LEN)
				{	
					// avoid double quoting key values that are quoted in the EDS file
					// we don't want to carry over the quotation marks from the EDS file, we provide
					// them ourselves below
					//
					// also strip out all escape chars that are represented as string literals in the file
					switch(input_buf[i])
					{
						//strip these values out entirely
						case('"'):
						case('\n'):
						{
							break;
						}

						// for escaped chars, replace them with a space and increment i to skip over the escaped character at i+1
						case('\\'):
						{
							if(i+1 < strlen(input_buf))
							{
								if(	input_buf[i+1] == 'n')
								{
									val_buf[val_i] = ' ';
									++i;
									++val_i;
								}

								else if(
									input_buf[i+1] == 'a' ||
									input_buf[i+1] == 'b' ||
									input_buf[i+1] == 'f' ||
									input_buf[i+1] == 'r' ||
									input_buf[i+1] == 't' ||
									input_buf[i+1] == 'v' ||
									input_buf[i+1] == '"' ||
									input_buf[i+1] == '\'' ||
									input_buf[i+1] == '\\' ||
									input_buf[i+1] == '?')
								{
									++i;
								}
							} 
							
							break;
						}

						default:
						{
							if(input_buf[i] != '"' && input_buf[i] != '\n')
							{
								val_buf[val_i] = input_buf[i];
								++val_i;
							}

							break;
						}
					}			
				}

				// not enough room ...
				else
				{
					output_buf_overflowed = true;
				}
			}
		}

		// once we get to the end, if write_pair was stored in "else if(storing_val)", format the key/value pair
		// as format "key_buf:val_buf," and append to output_buf.
		// also, update json_chars
		if(write_pair)
		{

			// check the key to see if it is one of the types that requires special processing	
			strncmp(key_buf, "Param", 5) == 0 ? spec_type = DATATYPE_SPEC_PARAM : DATATYPE_SPEC_NONE;
			strncmp(key_buf, "ProxiedParam", 12) == 0 ? spec_type = DATATYPE_SPEC_PARAM : DATATYPE_SPEC_NONE;
			strncmp(key_buf, "ProxyParam", 10) == 0 ? spec_type = DATATYPE_SPEC_PARAM : DATATYPE_SPEC_NONE;
			strncmp(key_buf, "Enum", 4) == 0 ? spec_type = DATATYPE_SPEC_ENUM : DATATYPE_SPEC_NONE;
			strncmp(key_buf, "ProxiedEnum", 11) == 0 ? spec_type = DATATYPE_SPEC_ENUM : DATATYPE_SPEC_NONE;
			strncmp(key_buf, "ProxyEnum", 9) == 0 ? spec_type = DATATYPE_SPEC_ENUM : DATATYPE_SPEC_NONE;

			switch(spec_type)
			{
				case(DATATYPE_SPEC_NONE):
				{
					snprintf(temp_out_buf, sizeof(temp_out_buf)-1, "\"%s\":\"%s\",", key_buf, val_buf);
					break;
				}

				case(DATATYPE_SPEC_ENUM):
				{	
					// do some parsing first to get the correctly parsed value
					char alt_val_buf[VAL_BUF_LEN] = {0};
					size_t j = 0;

					ERR_LIBEDS_t err = _parse_comma_delimited_val(spec_type, 
													val_buf, 
													alt_val_buf, 
													VAL_BUF_LEN, 
													&j);

					if(err == 0)
					{
						snprintf(temp_out_buf, sizeof(temp_out_buf)-1, ",\"%s\":{%s}},", key_buf, alt_val_buf);
					}
					else
					{
						return err;
					}
				
					break;
				}

				case(DATATYPE_SPEC_PARAM):
				{	
					// do some parsing first to get the correctly parsed value
					char alt_val_buf[VAL_BUF_LEN] = {0};
					size_t j = 0;

					ERR_LIBEDS_t err = _parse_comma_delimited_val(spec_type, 
													val_buf, 
													alt_val_buf, 
													VAL_BUF_LEN, 
													&j);

					if(err == 0)
					{
						snprintf(temp_out_buf, sizeof(temp_out_buf)-1, "\"%s\":{%s},", key_buf, alt_val_buf);
					}
					else
					{
						return err;
					}

					break;
				}

				default:
				{
					return ERR_EDSFILEFAIL;
				}
			}

			// check to see if output_buf is large enough to hold the data 
			if((strlen(temp_out_buf) + strlen(output_buf)) < output_buf_size)
			{
				// to make the enum data part of the param object, offset it back two chars to remove the closing } of the 
				// preceeding Param. The extra formatting above takes care of making this valid JSON.
				if(spec_type == DATATYPE_SPEC_ENUM)
				{
					memcpy(output_buf+(strlen(output_buf)-2), temp_out_buf, strlen(temp_out_buf));
					
					// since we are moving back the enum string, remove 2 chars from the JSON count
					*json_chars -= 2;
				}

				else
				{
					strncat(output_buf, temp_out_buf, strlen(temp_out_buf));
				}
				
				*json_chars += strlen(temp_out_buf);
			}

			// if output_buf won't hold the string, update json_chars but don't copy
			// also set a flag saying that we are overflowed, so that we return properly
			else
			{
				*json_chars += strlen(temp_out_buf);
				output_buf_overflowed = true;
			}

			// reset all variables
			memset(key_buf, 0, KEY_BUF_LEN*sizeof(char));
			memset(val_buf, 0, VAL_BUF_LEN*sizeof(char));
			memset(temp_out_buf, 0, VAL_BUF_LEN*sizeof(char));
			key_i = 0;
			val_i = 0;

			write_pair = false;
			storing_val = false;
		}
	}

	// if the loop did overflow, notify the caller
	if(output_buf_overflowed)
	{
		return ERR_OBUFF;
	}	

	else
	{
		return 0;
	}
}

ERR_LIBEDS_t _parse_comma_delimited_val(const SPECIAL_DATA_TYPES_t type, 
											const char * const val_buf, 
											char * const output_buf,
											const size_t output_buf_size,
											size_t *json_chars)
{	
	/**
	*
	* The Params section switches to using comma delimiters for the individual params.
	* The general structure of this section is to parse the comma delimited string into a JSON object.
	* The param field is a fixed delineation of parameters, always in the same order.
	* So, we should be able to (a) parse out the comma-separted substrings into a holding array, and then
	* (b) build JSON with the key value array and the data in the holding array.
	*
	*/
	if(type == DATATYPE_SPEC_PARAM)
	{
		const int8_t num_params_vals = 21;
		const int8_t num_key_vals = 24;
		
		int8_t params_val_idx = 0;
		int32_t val_string_idx = 0;

		size_t i = 0;

		bool output_buf_overflowed = false;

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
					if(strlen(params_vals[params_val_idx])+1 < VAL_BUF_LEN)
					{
						params_vals[params_val_idx][val_string_idx] = val_buf[i];
						++val_string_idx;
					}

					else
					{
						output_buf_overflowed = true;
					}
					
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
				if(val_buf[i+1] == ',' || val_buf[i+1] == '\0')
				{
					char *s = 0;

					if(val_buf[i+1] == '\0')
					{
						char ss[] = "0";
						s = ss;
					}

					else
					{
						char ss[] = "null";
						s = ss;
					}

					if(strlen(params_vals[params_val_idx]) + strlen(s) + 1 < VAL_BUF_LEN)
					{
						snprintf(params_vals[params_val_idx], VAL_BUF_LEN, "%s", s);
					}

					else
					{
						output_buf_overflowed = true;
					}
				}

				// since we were just adding characters make sure that the string for the value is terminated
				else
				{
					params_vals[params_val_idx][val_string_idx] = '\0';
				}
			}

			// stop parsing when we see a semicolon
			else if(val_buf[i] == '\0')
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

			else
			{	
				output_buf_overflowed = true;
			}

			*json_chars += strlen(s);
		}

		// no error, return 0
		if(output_buf_overflowed)
		{
			return ERR_OBUFF;
		}
		else
		{
			return 0;
		}
	}

	else if(type == DATATYPE_SPEC_ENUM)
	{
		size_t i = 0;
		size_t output_buf_idx = 0;
		bool reading_bit_desc = false;
		bool output_buf_overflowed = false;

		for(i=0; i < strlen(val_buf); i++)
		{
			// data - append to output_buf (assuming it is large enough), and ignore certain characters
			if(val_buf[i] != ',')
			{
				// first pass, start with an opening quotation mark
				if(i == 0)
				{
					if(output_buf_idx+1 < output_buf_size)
					{
						output_buf[output_buf_idx] = '"';
						++output_buf_idx;
						++*json_chars;
					}

					else
					{	
						++*json_chars;
						output_buf_overflowed = true;
					}
				}

				// exclude new lines and quotations from the data
				if(val_buf[i] != '"' && val_buf[i] != '\n')
				{
					if(output_buf_idx+1 < output_buf_size)
					{
						output_buf[output_buf_idx] = val_buf[i];
						++output_buf_idx;
						++*json_chars;
					}

					else
					{	
						++*json_chars;
						output_buf_overflowed = true;
					}
				}
			}

			// first comma, this splits the difference between the bit value and the bit desc
			// converted to "value":"bit desc" in json
			else if(val_buf[i] == ',' && reading_bit_desc == false)
			{
				reading_bit_desc = true;

				if(output_buf_idx+3 < output_buf_size)
				{
					output_buf[output_buf_idx] = '"';
					++output_buf_idx;

					output_buf[output_buf_idx] = ':';
					++output_buf_idx;

					output_buf[output_buf_idx] = '"';
					++output_buf_idx;

					*json_chars += 3;
				}

				else
				{	
					*json_chars += 3;
					output_buf_idx += 3;
					output_buf_overflowed = true;
				}
			}

			// second comma is the delimeter between values
			// converted to ..","other_val":"foo desc"
			else if(val_buf[i] == ',' && reading_bit_desc == true)
			{
				reading_bit_desc = false;

				if(output_buf_idx+3 < output_buf_size)
				{
					output_buf[output_buf_idx] = '"';
					++output_buf_idx;

					output_buf[output_buf_idx] = ',';
					++output_buf_idx;

					output_buf[output_buf_idx] = '"';
					++output_buf_idx;

					*json_chars += 3;
				}
				
				else
				{	
					*json_chars += 3;
					output_buf_overflowed = true;
				}
			}

			// unexpected character, this would be an error
			else
			{	
				*json_chars = 0;
				return ERR_PARSEFAIL;
			}

			// end of data - close the quotation mark for the last entry
			// don't have to increment json_chars here as we are replacing the semicolon that was already
			// in place (since we aren't ignoring semi-colons up top)
			if(i == strlen(val_buf)-1 && !output_buf_overflowed)
			{
				if(strlen(output_buf)+1 < output_buf_size)
				{
					output_buf[output_buf_idx] = '"';
					++*json_chars;
					++output_buf_idx;
				}

				else
				{	
					++output_buf_idx;
					output_buf_overflowed = true;
				}
			}
		}

		// if the output buffer was overflowed, report as such
		// otherwise, return success
		if(!output_buf_overflowed)
		{
			output_buf[strlen(output_buf)] = '\0';
			return 0;
		}

		else
		{
			output_buf[strlen(output_buf)] = '\0';
			return ERR_OBUFF;
		}
	}

	// default - there are no special rules for handling this section, so set json_chars to 0 and return
	else
	{
		*json_chars = 0;
		return ERR_PARSEFAIL;
	}

	// should never reach here
	return 0;
}

PARSABLE_EDS_SECTIONS_t _section_enum_from_section_name(const char * const section_name)
{
	size_t len = sizeof(eds_parsable_section_names)/sizeof(char *);
	size_t i = 0;
	int8_t result = 0;

	for(i=0; i< len; i++)
	{
		result = strncmp(eds_parsable_section_names[i], section_name, strlen(section_name));

		if(result == 0)
		{
			switch(i)
			{
				case(0):
				{
					return EDS_FILE;
					break;
				}

				case(1):
				{
					return EDS_DEVICE;
					break;
				}

				case(2):
				{
					return EDS_DEVICE_CLASSIFICATION;
					break;
				}

				case(3):
				{
					return EDS_PARAMS;
					break;
				}

				case(4):
				{
					return EDS_PORT;
					break;
				}

				case(5):
				{
					return EDS_CAPACITY;
					break;
				}

				case(6):
				{
					return EDS_DLR_CLASS;
					break;
				}

				case(7):
				{
					return EDS_ETHERNET_LINK_CLASS;
					break;
				}
				case(8):
				{
					return EDS_CONNECTION_MANAGER;
					break;
				}
			}
		}
	}

	// if we made it to here, there is an error
	return ERR_EDSFILEFAIL;
}