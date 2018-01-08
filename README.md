# *libeds2json*
*libeds2json* is a cross platform file processing program written in POSIX C. The goal of the library is to provide a fast, (computationally) cheap, and accurate API for converting EDS (Electronic Data Sheets) to JSON objects.

With each release, *libeds2json* is tested against the (roughly) 2400 files in [Rockwell Automation's EDS repository](https://www.rockwellautomation.com/global/support/networks/eds.page) and tested for JSON validity using [jsonlint](https://www.npmjs.com/package/jsonlint).

*libeds2json* is designed to work as both a shared library as well as a static include - the latter designed with embedded systems in-mind.

Currently, *libeds2json* supports more in-depth parsing of the Params and Enum sections, parsing these sections into more complete JSON objects. Future releases of *libeds2json* will incorporate the same level of parsing for other comma-delimited data structures such as the Assembly section. See the **Wish List** section for more information.

# Examples
Enough of the boring stuff. Want to see some examples of *libeds2json*'s output? Check the **EDS Files** directory for a batch of unadulterated EDS files and their corresponding JSON representations.

These files were generated using the *simple_example* example in this repository, used as follows:

```shell
# this parses the EDS file and drops the JSON into input_file.json
simple_example -f input_file.eds -o input_file.json

# want to test against jsonlint? You can do that like so (the -s prints the JSON to stdout):
simple_example -f input_file.eds -s | jsonlint
```
# Requirements
All you need to compile *libeds2json* is an up-to-date version of *cmake* and your favorite C compiler. If you would like to use the included unit tests, you will also need a valid C++ compiler. *libeds2json* has been tested with both *clang/llvm* as well as *gcc/g++*.

*libeds2json* does use the GoogleTest framework for unit testing. *cmake* takes care of downloading and compiling GoogleTest - no intervention is required on the part of the user.

# Performance
For speed and thread safety, all buffers are pre-allocated and dynamic memory allocation is not used. This has the benefit of making *libeds2json* easier to port to more constrained embedded systems - not to mention performance benefits from having pre-allocated memory - but, there are some large buffers that may need to be tweaked depending on your architecture.

All large static buffers are defined in *libeds.h*. These can be changed to fit your specific application.

### Raspberry Pi 3
Using *simple_example*, which is sequential, single-threaded execution, *libeds2json* processes 2400 EDS files in 301 seconds, or ~125ms per EDS file.

# Installation
Installation is straight forward. In brief, once you have changed directory to the *build* directory, all you need to do is run *cmake* and then decide what target you would like to build against. 

```shell
cd build

# run one of the next two commands
# use this command for a standard build
cmake ../src/

# or, if you would like debug symbols included, run this one:
cmake -DCMAKE_BUILD_TYPE=Debug ../src/

# once cmake sets up the build environment, run make against the target you would like to build

# Option 1 - build just the library
make eds2json

# Option 2 - build the simple_example program
make simple_example

# Option 3 - build the unit tests (this will download GoogleTest to the local directory)
make libeds2json_tests

# Option 4 - build everything!
make

# once your target is built, you can either...

# install the library
sudo make install eds2json

# run the unit tests
./libeds/libeds2json_tests

# or run the simple_example program
./examples/simple_example

```

# Usage
*libeds2json* was designed to be simple to use. The primary calling function is:

```c
ERR_LIBEDS_t convert_eds2json(const char * const eds_file_path, 
					char * const json_array, 
					const size_t json_array_size,
					size_t *json_chars);
```

The function variables are as follows:

1. **eds_file_path**: the full path to the EDS file to be converted to JSON
2. **json_array**: the buffer that will hold the resulting JSON
3. **json_array_size**: the size of json_array
4. **json_chars**: the number of JSON characters in the result.

The function returns an error code of type ERR_LIBEDS_t. A human-readable version of the error can be retrieved using the following function:

```c
int8_t err_string(const ERR_LIBEDS_t err_code, char * const err_string, const size_t err_string_size);
```

***NOTE*** - *err_string* has to be at least 128 bytes (chars) or else the function will throw an error.

*libeds2json* is designed to help the programmer as much as possible. *json_chars* is always set - even if *json_array* is not large enough to hold the JSON generated from the EDS file at *eds_file_path*.

If *json_array* is too small, the function will return *ERR_OBUFF* and the user should check *json_chars* and resize *json_array* as necessary.

# Wish List
1. More thorough parsing of all comma-seperated complex values, similar to the Params and Enum sections b

# Legal
Copyright 2018 3ML LLC

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

All trademarks are copyright their respective owners. Usage of company names and copyrights does not denote a relationship or endorsement of any kind between these companies 3ML LLC.
