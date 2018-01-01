/**

	@file simple_example.c
	@author John Davis <jd@three-ml.com>
	@date 30 Nov 2017
	@brief eds2json - Converts an ODVA EDS file to a JSON object.

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

#include "simple_example.h"
#include "libeds.h"

#include <stdio.h>
#include <stdint.h>

int8_t main(int8_t argc, char **argv)
{
	fprintf(stderr, "%s\n", "Hello world from eds2json!");
	return 0;
}