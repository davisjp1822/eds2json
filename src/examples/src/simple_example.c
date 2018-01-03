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
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

void print_usage();

int8_t main(int8_t argc, char **argv)
{

	int8_t opt = 0;
	char *eds_file_path = NULL;
	char *output_json_file_path = NULL;
	bool print_json_to_stdout = false;

	while ((opt = getopt(argc, argv, "hf:so:")) != -1)
	{
		switch(opt)
		{
			case('f'):
			{
				eds_file_path = optarg;
				break;
			}

			case('s'):
			{
				print_json_to_stdout = true;
				break;
			}

			case('o'):
			{
				output_json_file_path = optarg;
				break;
			}

			case('h'):
			{
				print_usage();
				return 0;
				break;
			}

			case('?'):
			{	
				if(optopt == 'f' || optopt == 'o')
				{
					fprintf(stderr, "Option %c requires an argument\n", optopt);
				}

				else
				{
					print_usage();
				}

				return 1;
				break;
			}

			default:
			{
				exit(-1);
			}
		}
	}

	// no arguments! print usage and exit. 
	if(argc == 1)
	{
		print_usage();
		return 1;
	}

	// must specify a file path!
	if(!eds_file_path)
	{
		fprintf(stderr, "%s", "You must specify a path to a valid EDS file!\n");
		return 1;
	}

	// now the good stuff
	char json_output[LARGE_BUF] = {0};
	size_t output_json_chars = 0;

	ERR_LIBEDS_t err = convert_eds2json(eds_file_path, json_output, sizeof(json_output), &output_json_chars);

	if(err != 0)
	{
		// 128 is the minimum length for error buffers
		char e_string[128] = {0};

		int8_t r = err_string(err, e_string, sizeof(e_string));
		
		if(r==0)
		{
			fprintf(stderr, "Fatal EDS Parsing Error: %s\n", e_string);
		}
		else
		{
			fprintf(stderr, "Fatal EDS Parsing Error: %s\n", "Unknown error and also error parsing libeds2json error code!");
		}

		return 1;
	}

	else
	{
		if(print_json_to_stdout)
		{
			fprintf(stdout, "{%s}\n", json_output);
			fflush(stdout);
		}

		if(output_json_file_path)
		{
			FILE *f = fopen(output_json_file_path, "w");

			if(f)
			{
				size_t fbuf_size = strlen(json_output)+4;
				char fbuf[fbuf_size];

				memset(fbuf, 0, fbuf_size);
				snprintf(fbuf, fbuf_size, "{%s}\n", json_output);


				fwrite(fbuf, fbuf_size-1, 1, f);
				fclose(f);
				return 0;
			}
			else
			{
				fprintf(stderr, "Fatal EDS Parsing Error: %s\n", strerror(errno));
				return 1;
			}
		}
	}

	return 0;
}

void print_usage()
{
	const char *s = "\nlibeds2json Simple Test Program\n"
					"A Three Martini Lunch (ThreeML) production, vintage 2018\n"
					"Usage:\n"
					"------\n"
					"-f <path to EDS file to get JSON-ified>\n"
					"-s prints JSON to stdout\n"
					"-o <path to output file> prints JSON to file at specified path\n"
					"-h prints this message\n"
					"\n"
					"-f is always required\n"
					"If -o or -s are not specified, this program just runs without any output (useful for timing)\n\n";

	fprintf(stderr, "%s", s);
}