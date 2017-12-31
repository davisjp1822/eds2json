/**

	@file libeds.h
	@author John Davis <jd@three-ml.com>
	@date 30 Nov 2017
	@brief Header file for libeds - an open source ODVA EDS file parser.

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

#ifndef _LIBEDS_H
#define _LIBEDS_H

#include <stddef.h>
#include <stdint.h>

// got these values from EZ-EDS. The actual length is 30,000 chars - the +1 is for the null terminator
#define KEY_BUF_LEN 30001
#define VAL_BUF_LEN 30001

// how many chars we want to read into memory - should be enough to read in the entire EDS File
// if not, the program will return ERR_OBUFF and you can tweak this as necessary
#define LARGE_BUF 1000000

// the length of the longest EDS Section name string
// which, happens to be "Connection Configuration", which is 24 char. This gives us some buffer.
#define EDS_SECTION_NAME_LEN 32

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Error codes returned by libeds.
 *
 * A full list of the error codes returned by libeds. May be called by using the following helper function:
 * @code
 * void err_string(const ERR_LIBEDS_t err_code, char * const err_string)
 * @endcode
 *
 */ 
typedef enum
{
	ERR_OBUFF = 1,
	ERR_PARSEFAIL,
	ERR_EDSFILEFAIL,
	ERR_EDSFILENOTFOUND,
	ERR_EDSFILEINUSE
} ERR_LIBEDS_t;

/**
 * 
 * @brief Valid parseable EDS sections.
 *
 */ 
typedef enum
{
	EDS_FILE = 1,
	EDS_DEVICE,
	EDS_DEVICE_CLASSIFICATION,
	EDS_PARAMS,
	EDS_ASSEMBLY,
	EDS_CONNECTION_MANAGER,
	EDS_PORT,
	EDS_CAPACITY,
	EDS_DLR_CLASS,
	EDS_ETHERNET_LINK_CLASS
} PARSABLE_EDS_SECTIONS_t;

/**
 * 
 * @brief Certain data types require special processing for their value strings.
 *
 * The types below tell the _parsing_ functions whether or not additional parsing needs to be done before
 * returning a complete JSON string to the caller. 
 *
 */ 
typedef enum
{
	DATATYPE_SPEC_NONE,
	DATATYPE_SPEC_PARAM,
	DATATYPE_SPEC_ENUM
} SPECIAL_DATA_TYPES_t;

/**
 *
 * @ brief Defines what types of EDS sections are currently supported for parsing.
 *
 * Corresponding parsing logic MUST be added to _parse_eds_keyval() and _parse_comma_delimited_val() before
 * adding sections to this list!
 *
 */
const char * const eds_parsable_section_names[] = 
{
	"File",
	"Device",
	"Device Classification",
	"Params",
	"Port",
	"Capacity",
	"DLR Class",
	"Ethernet Link Class"
};

/**
 * @brief Converts an EDS file created by EZ-EDS to a JSON object.
 *
 * This function works best with EDS files created by EZ-EDS. The returned JSON object will be an exact replica
 * of the EDS file, just in a more usable format.
 *
 * @param eds_file_path Absolute path to the EDS file to be converted.
 * @param json_array Pointer to char array that will hold the converted EDS file
 * @param json_array_size Size of json_array char array
 * @param json_chars the number of characters in the returned JSON array. If json_array is too small to hold all of
 * the characters, this value will still be populated so that the caller knows how large to make the receiving buffer.
 * @return Returns 0 if success
 * @see https://www.rockwellautomation.com/resources/downloads/rockwellautomation/pdf/sales-partners/technology-licensing/Logix_EDS_AOP_Guidelines.pdf
 *
 */

ERR_LIBEDS_t convert_eds2json(const char * const eds_file_path, 
								char * const json_array, 
								const size_t json_array_size,
								size_t *json_chars);

/**
* 
* @brief The following function parses EDS file sections as defined by PARSABLE_EDS_SECTIONS_t.
* 
* The function is passed the char buffer (input_buf) that is in between the closing ] of the header and the starting [
* of the next header. This string typically comes from convert_eds2json. 
*
* Once parsed, they write the JSON string to output_buf and return the size of output_buf.
* 
* @param s_type EDS section as defined by PARSABLE_EDS_SECTIONS_t
* @param input_buf Pointer to array of char that is the data between  the closing ']'of the header and the starting '[' of the next header.
* @param output_buf Pointer to array of char that will hold the output JSON.
* @param output_buf_size Size of the output buffer. Should *always* be considerably larger than sizeof(input_buf)!
* @note output_buf_size should ALWAYS be larger than sizeof(input_buf) by a considerable margin!!
* @param num_json_chars How many JSON characters are present in the output. If output_buffer is too small, the function
* still runs and provides the resulting number of JSON characters in *num_json_chars. Use this to size output_buf.
*
* @return Success: Returns 0 if success
* @return Fail: ERR_OBUFF if output_buf not large enough
* @return Fail: ERR_PARSEFAIL if input_buf is not a valid input string
*
*/
ERR_LIBEDS_t convert_section2json(const PARSABLE_EDS_SECTIONS_t s_type, 
									const char * const input_buf, 
									char * const output_buf, 
									const size_t output_buf_size,
									size_t * const num_json_chars);

/** 
 * @ brief Returns a human readable string for error code err_code.
 * 
 * Retruns an explanation for the error codes as specified in ERR_LIBEDS_t.
 *
 * @param err_code The error code to be described.
 * @param err_string A string of the appropriate size (128 characters) to receieve the description.
 * @note err_string should be 128 characters!
 * @return -2 if err_code is unknown
 * @return -1 if err_string is less than 128 characters.
 * @return 0 if err_string was set succesfully
 *
 */
int8_t err_string(const ERR_LIBEDS_t err_code, char * const err_string, const size_t err_string_size);

#ifdef __cplusplus
}
#endif

#endif /* _LIBEDS_H */