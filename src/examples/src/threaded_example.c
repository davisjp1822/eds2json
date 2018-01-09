/**

	@file threaded_example.c
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

#include "threaded_example.h"
#include "libeds.h"

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

#define MAX_THREADS 500

void print_usage();
void *converting_thread(void *eds_file_path);


int main(int argc, char **argv)
{

	int8_t opt = 0;
	char *eds_file_paths = NULL;

	while ((opt = getopt(argc, argv, "hf:")) != -1)
	{
		switch(opt)
		{
			case('f'):
			{
				eds_file_paths = optarg;
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
				if(optopt == 'f')
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
	if(!eds_file_paths)
	{
		fprintf(stderr, "%s", "You must specify a valid path to a file with paths to EDS files!\n");
		return 1;
	}

	// parse the file and spool up threads
	FILE *fp = NULL;

	size_t num_files = 2400;
	size_t file_counter = 0;
	ssize_t chars_read = 0;
	char **files_list = NULL;

	char *line_ptr = NULL;
	size_t line_len = 0;

	fp = fopen(eds_file_paths, "r");
    
    if (fp == NULL)
    {
        return -1;
    }

    // read each file name into memory, reallocate it to files_list, and then increment num_files
    // start with allocating memory for at least one file name
	files_list = malloc(sizeof(char *) * num_files);

    while ((chars_read = getline(&line_ptr, &line_len, fp)) != -1) 
    {
		files_list[file_counter] = malloc(sizeof(char) * line_len);
    	memset(files_list[file_counter], 0, sizeof(char) * line_len);

    	memcpy(files_list[file_counter], line_ptr, line_len);
    	++file_counter;
    }

    // time to do some work
    size_t i = 0;
    size_t j = 0;
    pthread_t threads[MAX_THREADS];
    size_t thread_counter = 0;

    for(j=0; j < file_counter; j++)
    {
		pthread_create(&threads[thread_counter], NULL, converting_thread, files_list[j]);
		++thread_counter;

    	if(thread_counter == MAX_THREADS-1)
    	{
   			/* wait for the threads to finish */
    		for (i=0; i < MAX_THREADS; i++) 
    		{
    			pthread_join(threads[i], NULL);
    			//fprintf(stdout, "Joined Thread %lu\n", i);
    		}

    		thread_counter = 0;
    	}
    }

    // free memory
    for(i=0; i < file_counter; i++)
    {
    	free(files_list[i]);
    }

    free(files_list);
    fclose(fp);
    free(line_ptr);

	return 0;
}

void print_usage()
{
	const char *s = "\nlibeds2json Threaded Test Program\n"
					"A Three Martini Lunch (ThreeML) production, vintage 2018\n"
					"Usage:\n"
					"------\n"
					"-f path to file containing full paths to EDS files to be converted to JSON\n"
					"-h prints this message\n"
					"\n"
					"-f is always required\n"
					"If -s is not specified, this program will output the JSON to files with the same name as the input EDS file but with the extension .json\n\n";

	fprintf(stderr, "%s", s);
}

void *converting_thread(void *eds_file_path)
{
	char json_output[LARGE_BUF] = {0};
	size_t output_json_chars = 0;

	// create a temporary buffer and process out the non-printable characters
	// once done, call convert_eds2json
	char *efp = (char *)eds_file_path;

	char temp_buf[strlen(efp)+1];
	memset(temp_buf, 0, sizeof(temp_buf));

	memcpy(temp_buf, efp, sizeof(temp_buf));
	temp_buf[strcspn(temp_buf, "\r\n")] = 0;
	temp_buf[strlen(temp_buf)] = '\0';

	ERR_LIBEDS_t err = convert_eds2json(temp_buf, json_output, sizeof(json_output), &output_json_chars);

	// return the error if there is an issue
	if(err != 0)
	{
		pthread_exit(NULL);
	}

	// otherwise, just write the file
	else
	{
		FILE *f = NULL;

		// .json.conv
		size_t filename_len = strlen(temp_buf)+11;
		char filename[filename_len];

		memset(filename, 0, sizeof(filename));

		snprintf(filename, filename_len, "%s.json.conv", temp_buf);

		f = fopen(filename, "w");
    
	    if (f == NULL)
	    {
	        pthread_exit(NULL);
	    }

	    size_t fbuf_size = strlen(json_output)+4;
	    char fbuf[fbuf_size];

	    memset(fbuf, 0, fbuf_size);
	    snprintf(fbuf, fbuf_size, "{%s}\n", json_output);

	    fwrite(fbuf, fbuf_size-1, 1, f);
	    fclose(f);

		pthread_exit(NULL);
	}
}
