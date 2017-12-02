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

/**
 * @brief Converts an EDS file created by EZEDS to a JSON object.
 *
 * This function works best with EDS files created by EZEDS. The returned JSON object will be an exact replica
 * of the EDS file, just in a more usable format.
 *
 * @param eds_file_path Absolute path to the EDS file to be converted.
 * @return A pointer to a character array representing a JSON string representing the contents of the EDS file.
 * @see https://www.rockwellautomation.com/resources/downloads/rockwellautomation/pdf/sales-partners/technology-licensing/Logix_EDS_AOP_Guidelines.pdf
 *
 */

char *convert_eds2json(char *eds_file_path);

#endif /* _LIBEDS_H */