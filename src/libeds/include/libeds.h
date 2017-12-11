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
	ERR_EDSFILEFAIL
} ERR_LIBEDS_t;

/**
 * 
 * @brief Valid parseable EDS sections.
 *
 */ 
typedef enum
{
	EDS_FILE,
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
 * @brief Converts an EDS file created by EZ-EDS to a JSON object.
 *
 * This function works best with EDS files created by EZ-EDS. The returned JSON object will be an exact replica
 * of the EDS file, just in a more usable format.
 *
 * @param eds_file_path Absolute path to the EDS file to be converted.
 * @param json_array Pointer to char array that will hold the converted EDS file
 * @param json_array_size Size of json_array char array
 * @return Returns 0 if success
 * @see https://www.rockwellautomation.com/resources/downloads/rockwellautomation/pdf/sales-partners/technology-licensing/Logix_EDS_AOP_Guidelines.pdf
 *
 */

ERR_LIBEDS_t convert_eds2json(const char * const eds_file_path, char * const json_array, const size_t json_array_size);

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
* @param num_json_chars How many JSON characters are present in the output. If output_buffer is too small, the function
* still runs and gives an accurate number of characters. The function will still return an error, but num_json_chars will remain set.
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
 * @brief Returns an array of strings detailing the sections of the EDS file that were not parsed. This is useful
 * primarily for debugging EDS files that were not fully parsed.
 * 
 * Mainly used for debuggging and general info, this function returns the names of the sections in the EDS file
 * that were not parsed.
 *
 * @ return an array of strings that are names of the unparsed sections in the EDS file.
*/
uint32_t get_unparsed_sections();

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