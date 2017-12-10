/**

	@file libeds_tests.c
	@author John Davis <jd@three-ml.com>
	@date 30 Nov 2017
	@brief libeds unit tests

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

#include <gtest/gtest.h>
#include "libeds.h"

#include <stdint.h>

#define TESTING
#define TEST_EDS_PATH1 /mnt/c/Users/davisjp/OneDrive/Development/eds2json/src/libeds/test/eds_files/SMD23E2_v1_6.eds

namespace 
{
	class libedsTests : public testing::Test
	{
		void SetUp()
		{

		}

		void TearDown()
		{

		}
	};

	TEST(libedsTests, err_string_unknown_error)
	{
		char rs[1300] = {0};

		int32_t r = err_string((ERR_LIBEDS_t)423423432, rs, 128);

		ASSERT_EQ(r, -2);
	}

	TEST(libedsTests, err_string_buffer_too_small)
	{
		char rs[1300] = {0};

		int32_t r = err_string(ERR_OBUFF, rs, 127);

		ASSERT_EQ(r, -1);
	}

	TEST(libedsTests, convert_eds2json_bad_path)
	{
		char fp[128] = "/tmp/foo.foo";
		char json_array[256] = {0};

		ERR_LIBEDS_t r = convert_eds2json(fp, json_array, 256);

		ASSERT_EQ(r, ERR_EDSFILEFAIL);
	}


	/*TEST(libedsTests, convert_section2json)
	{

		char input_buf[256] = {0};
		char output_buf[256] = {0};

		uint32_t retVal = convert_section2json(EDS_FILE, input_buf, output_buf, strlen(output_buf));

		ASSERT_EQ(retVal, 0);
	}*/
}

int main(int argc, char **argv)
{
	testing::InitGoogleTest(&argc, argv);

	return RUN_ALL_TESTS();
}