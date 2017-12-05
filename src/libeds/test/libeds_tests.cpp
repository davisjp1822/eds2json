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

uint32_t _parse_file(const PARSABLE_EDS_SECTIONS_t s_type, const char * const input_buf, char * const output_buf, const size_t output_buf_size);

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

	TEST(libedsTests, err_obuff)
	{
		ASSERT_EQ(1,1);
	}

	TEST(libedsTests, err_parsefail)
	{
		ASSERT_EQ(1,1);
	}
}

int main(int argc, char **argv)
{
	testing::InitGoogleTest(&argc, argv);

	return RUN_ALL_TESTS();
}