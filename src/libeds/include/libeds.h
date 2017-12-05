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
	ERR_OBUFF,
	ERR_PARSEFAIL
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
	EDS_PORT
} PARSABLE_EDS_SECTIONS_t;


/**
 * @brief Converts an EDS file created by EZ-EDS to a JSON object.
 *
 * This function works best with EDS files created by EZ-EDS. The returned JSON object will be an exact replica
 * of the EDS file, just in a more usable format.
 *
 * @param eds_file_path Absolute path to the EDS file to be converted.
 * @param json_array Pointer to char array that will hold the converted EDS file
 * @param json_array_size Size of json_array char array.
 * @see https://www.rockwellautomation.com/resources/downloads/rockwellautomation/pdf/sales-partners/technology-licensing/Logix_EDS_AOP_Guidelines.pdf
 *
 */

void convert_eds2json(const char * const eds_file_path, char * const json_array, const size_t json_array_size);

/**
 * @brief Returns an array of strings detailing the sections of the EDS file that were not parsed. This is useful
 * primarily for debugging EDS files that were not fully parsed.
 * 
 * Mainly used for debuggging and general info, this function returns the names of the sections in the EDS file
 * that were not parsed.
 *
 * @ return an array of strings that are names of the unparsed sections in the EDS file.
*/
char ** get_unparsed_sections();

/** 
 * @ brief Returns a human readable string for error code err_code.
 * 
 * Retruns an explanation for the error codes as specified in ERR_LIBEDS_t.
 *
 * @param err_code The error code to be described.
 * @param err_string A string of the appropriate size to receieve the description.
 * @note err_string should be 128 characters!
 *
 */
void err_string(const ERR_LIBEDS_t err_code, char * const err_string);

#endif /* _LIBEDS_H */