#libeds2json
libeds2json is a cross platform file processing program written in POSIX C. The goal of the library is to provide a fast, cheap, and accurate API for converting EDS (Electronic Data Sheets) to JSON objects.

With each release, libeds2json is tested against the (roughly) 2400 files in [Rockwell Automation's EDS repository](https://www.rockwellautomation.com/global/support/networks/eds.page) and tested for JSON validity using [jsonlint](https://www.npmjs.com/package/jsonlint).

libeds2json is designed to work as both a shared library for as well as a static include - the intent of the latter to be used in embedded applications.

Currently, libeds2json supports more in-depth parsing of the Params and Enum sections, parsing these sections into more complete JSON objects. Future releases of libeds2json will incorporate the same level of parsing for other comma-delimited data structures such as the Assembly section. See the **Wish List** section for more information.



#Requirements

#Installation

cmake ../src/| 2085  cmake -DCMAKE_BUILD_TYPE=Debug ../src/| 2137  cmake -DCMAKE_BUILD_TYPE=Debug ../src/

#Usage

#Wish List

#Legal
