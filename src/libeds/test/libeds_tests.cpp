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
#include <fstream>

#define TESTING

extern "C" 
{
	ERR_LIBEDS_t _parse_comma_delimited_val(const SPECIAL_DATA_TYPES_t type, 
											const char * const val_buf, 
											char * const output_buf,
											const size_t output_buf_size,
											size_t *json_chars);

	PARSABLE_EDS_SECTIONS_t _section_enum_from_section_name(const char * const section_name);
}

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

	TEST(libedsTests, err_string_test_ret_string)
	{
		char rs[1300] = {0};
		int32_t r = err_string(ERR_OBUFF, rs, 128);

		ASSERT_EQ(r, 0);
		ASSERT_STREQ("Error (libeds): insufficient capacity in receiving buffer", rs);
	}

	TEST(libedsTests, convert_eds2json_bad_path)
	{
		char fp[128] = "/tmp/foo.foo";
		char json_array[256] = {0};
		size_t json_chars = 0;

		ERR_LIBEDS_t r = convert_eds2json(fp, json_array, 256, &json_chars);

		ASSERT_EQ(r, ERR_EDSFILENOTFOUND);
	}

/*	TEST(libedsTests, read_eds_file)
	{
		const char *path = TEST_EDS_PATH1;
		char output_buf[2000000] = {0};
		size_t json_chars = 0;

		ERR_LIBEDS_t err = convert_eds2json(path, output_buf, 2000000, &json_chars);

		std::ofstream myfile;
		myfile.open ("example.txt");
		myfile << output_buf;
		myfile.close();

		ASSERT_EQ(err, 0);
		ASSERT_STREQ("foo", output_buf);
	}*/

	/*
	 * 
	 * Various section2json function checks - checking each section, essentially.
	 *
	 */

	TEST(libedsTests, convert_section2json_output_buf_too_small)
	{
		const char *input = "DescText=\"SMD23E2\";\nCreateDate=03-29-2012;\nCreateTime=14:01:47;\n"
							"ModDate=05-24-2016;\nModTime=13:57:27;\nRevision=1.6;\n"
        					"HomeURL=\"http://www.amci.com/driver files/SMD23E2_v1_5.eds\";\n"
        					"1_IOC_Details_License=0xDFE8039A;\n";

       	size_t good_json_chars = 239;
        char output_buf[7] = {0};
        size_t output_json_chars = 0;

        ERR_LIBEDS_t err = convert_section2json(EDS_FILE, input, output_buf, 7, &output_json_chars);

        ASSERT_EQ(1, err);
        ASSERT_EQ(good_json_chars, output_json_chars);
	}

	TEST(libedsTests, convert_section2json_unknown_section)
	{
		const char *input = "DescText=\"SMD23E2\";CreateDate=03-29-2012;CreateTime=14:01:47;"
							"ModDate=05-24-2016;ModTime=13:57:27;Revision=1.6;"
        					"HomeURL=\"http://www.amci.com/driver files/SMD23E2_v1_5.eds\";"
        					"1_IOC_Details_License=0xDFE8039A;";

        const char *good_json = "\"File\":{\"DescText\":\"\\\"SMD23E2\\\"\",\"CreateDate\":\"03-29-2012\","
        							"\"CreateTime\":\"14:01:47\",\"ModDate\":\"05-24-2016\","
        							"\"ModTime\":\"13:57:27\",\"Revision\":\"1.6\","
        							"\"HomeURL\":\"\\\"http://www.amci.com/driver files/SMD23E2_v1_5.eds\\\"\","
        							"\"1_IOC_Details_License\":\"0xDFE8039A\"}";

        //std::cout << good_json << std::endl;

        size_t good_json_chars = 247;
        char output_buf[1024] = {0};
        size_t output_json_chars = 0;

        ERR_LIBEDS_t err = convert_section2json((PARSABLE_EDS_SECTIONS_t)42424242, input, output_buf, 1024, &output_json_chars);

        //std::cout << output_buf << std::endl;

        ASSERT_EQ(2, err);
	}

	TEST(libedsTests, convert_section2json_value_embedded_semicolon)
	{
		const char *input = "DescText=\"SMD23;E2\";\nCreateDate=03-29-2012;\nCreateTime=14:01:47;\n"
							"ModDate=05-24-2016;\nModTime=13:57:27;\nRevision=1.6;\n"
        					"HomeURL=\"http://www.amci.com/driver files/SMD23E2_v1_5.eds\";\n"
        					"1_IOC_Details_License=0x;DFE8039A;\n";

        const char *good_json = "\"File\":{\"DescText\":\"SMD23;E2\",\"CreateDate\":\"03-29-2012\","
        							"\"CreateTime\":\"14:01:47\",\"ModDate\":\"05-24-2016\","
        							"\"ModTime\":\"13:57:27\",\"Revision\":\"1.6\","
        							"\"HomeURL\":\"http://www.amci.com/driver files/SMD23E2_v1_5.eds\","
        							"\"1_IOC_Details_License\":\"0x;DFE8039A\"}";

        //std::cout << good_json << std::endl;

        size_t good_json_chars = 241;
        char output_buf[1024] = {0};
        size_t output_json_chars = 0;

        ERR_LIBEDS_t err = convert_section2json(EDS_FILE, input, output_buf, 1024, &output_json_chars);

        //std::cout << output_buf << std::endl;

        ASSERT_EQ(0, err);
        ASSERT_STREQ(good_json, output_buf);
        ASSERT_EQ(good_json_chars, output_json_chars);
	}

	TEST(libedsTests, convert_section2json_file_section_toJSON)
	{
		const char *input = "DescText=\"SMD23E2\";\nCreateDate=03-29-2012;\nCreateTime=14:01:47;\n"
							"ModDate=05-24-2016;\nModTime=13:57:27;\nRevision=1.6;\n"
        					"HomeURL=\"http://www.amci.com/driver files/SMD23E2_v1_5.eds\";\n"
        					"1_IOC_Details_License=0xDFE8039A;\n";

        const char *good_json = "\"File\":{\"DescText\":\"SMD23E2\",\"CreateDate\":\"03-29-2012\","
        							"\"CreateTime\":\"14:01:47\",\"ModDate\":\"05-24-2016\","
        							"\"ModTime\":\"13:57:27\",\"Revision\":\"1.6\","
        							"\"HomeURL\":\"http://www.amci.com/driver files/SMD23E2_v1_5.eds\","
        							"\"1_IOC_Details_License\":\"0xDFE8039A\"}";

        //std::cout << good_json << std::endl;

        size_t good_json_chars = 239;
        char output_buf[1024] = {0};
        size_t output_json_chars = 0;

        ERR_LIBEDS_t err = convert_section2json(EDS_FILE, input, output_buf, 1024, &output_json_chars);

        //std::cout << output_buf << std::endl;

        ASSERT_EQ(0, err);
        ASSERT_STREQ(good_json, output_buf);
        ASSERT_EQ(good_json_chars, output_json_chars);
	}

	TEST(libedsTests, convert_section2json_device_section_toJSON)
	{
		const char *input = "VendCode=589;\nVendName=\"LinMot\";\nProdType=43;\nProdTypeStr=\"GenericDevice\";\nProdCode=2354;\nMajRev=1;\nMinRev=1;\n"
							"ProdName=\"E1450IPQN1S\";\nCatalog=\"LMDrive\";\nIcon=\"E1250_SC_xx.ico\";\n"
							"IconContents="
							"\"AAABAAEAICAQAAEABADoAgAAFgAAACgAAAAgAAAAQAAAAAEABAAAAAAAAAAA\"\n"
							"\"AAAAAAAAAAAAAAAAAAAAAAAAAAAAKrwqALKysgD///8AAAAAAAAAAAAAAAAA\"\n"
							"\"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAMzMzMzMzIiIi\"\n"
							"\"IjMzMzMzMzMzMzMzMyACIAIzMzMzMzMzMzMzMzMgAiACMzMzMzMzMzMzMzMz\"\n"
							"\"IiIiIjMzMzMzMzMzMzMzMyACIAIzMzMzMzMzMzMzMzMgAiACMzMzMzMzMzMz\"\n"
							"\"MzMzIiIiIjMzMzMzMzMzMzMzMyIiIgIzMzMzMzMzMzMzMzMiIiIiMzMzMzMz\"\n"
							"\"MzMzMzMzIiIiAjMzMzMzMzMzMzMzMyIiIiIzMzMzMzMzMzMzMzMiIiESMzMz\"\n"
							"\"MzMzMzMzMzMzIiIhEjMzMzMzMzMzMzMzMyIiIRIzMzMzMzMzMzMzMzMiIiES\"\n"
							"\"MzMzMzMzMzMzMzMzIiIhEjMzMzMzMzMzMzMzMyIiIRIzMzMzMzMzMzMzMzMi\"\n"
							"\"IiIiMzMzMzMzMzMzMzMzIiIgAjMzMzMzMzMzMzMzMyIAIAIzMzMzMzMzMzMz\"\n"
							"\"MzMiACACMzMzMzMzMzMzMzMzIgAgAjMzMzMzMzMzMzMzMyIiIAIzMzMzMzMz\"\n"
							"\"MzMzMzMiIiIiMzMzMzMzMzMzMzMzIREREjMzMzMzMzMzMzMzMyEBABIzMzMz\"\n"
							"\"MzMzMzMzMzMhERESMzMzMzMzMzMzMzMzIQEAEjMzMzMzMzMzMzMzMyERERIz\"\n"
							"\"MzMzMzMzMzMzMzMhAQASMzMzMzMzMzMzMzMzIREREjMzMzMzMzMzMzMzMyIi\"\n"
							"\"IiIzMzMzMzMAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\"\n"
							"\"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\"\n"
							"\"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\"\n"
							"\"AA==\";\n";

        const char *good_json = "\"Device\":{\"VendCode\":\"589\",\"VendName\":\"LinMot\",\"ProdType\":\"43\",\"ProdTypeStr\":\"GenericDevice\","
        						"\"ProdCode\":\"2354\",\"MajRev\":\"1\",\"MinRev\":\"1\",\"ProdName\":\"E1450IPQN1S\",\"Catalog\":\"LMDrive\","
        						"\"Icon\":\"E1250_SC_xx.ico\",\"IconContents\":\"AAABAAEAICAQAAEABADoAgAAFgAAACgAAAAgAAAAQAAAAAEABAAAAAAAAAAAAAA"
        						"AAAAAAAAAAAAAAAAAAAAAAAAAKrwqALKysgD///8AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAMzMzMz"
        						"MzIiIiIjMzMzMzMzMzMzMzMyACIAIzMzMzMzMzMzMzMzMgAiACMzMzMzMzMzMzMzMzIiIiIjMzMzMzMzMzMzMzMyACIAIzMzMzMzMzMzMzMzMgA"
        						"iACMzMzMzMzMzMzMzMzIiIiIjMzMzMzMzMzMzMzMyIiIgIzMzMzMzMzMzMzMzMiIiIiMzMzMzMzMzMzMzMzIiIiAjMzMzMzMzMzMzMzMyIiIiIz"
        						"MzMzMzMzMzMzMzMiIiESMzMzMzMzMzMzMzMzIiIhEjMzMzMzMzMzMzMzMyIiIRIzMzMzMzMzMzMzMzMiIiESMzMzMzMzMzMzMzMzIiIhEjMzMzM"
        						"zMzMzMzMzMyIiIRIzMzMzMzMzMzMzMzMiIiIiMzMzMzMzMzMzMzMzIiIgAjMzMzMzMzMzMzMzMyIAIAIzMzMzMzMzMzMzMzMiACACMzMzMzMzMz"
        						"MzMzMzIgAgAjMzMzMzMzMzMzMzMyIiIAIzMzMzMzMzMzMzMzMiIiIiMzMzMzMzMzMzMzMzIREREjMzMzMzMzMzMzMzMyEBABIzMzMzMzMzMzMzM"
        						"zMhERESMzMzMzMzMzMzMzMzIQEAEjMzMzMzMzMzMzMzMyERERIzMzMzMzMzMzMzMzMhAQASMzMzMzMzMzMzMzMzIREREjMzMzMzMzMzMzMzMyIi"
        						"IiIzMzMzMzMAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
        						"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA==\"}";

        //std::cout << good_json << std::endl;

        size_t good_json_chars = 1249;
        char output_buf[2048] = {0};
        size_t output_json_chars = 0;

        ERR_LIBEDS_t err = convert_section2json(EDS_DEVICE, input, output_buf, 2048, &output_json_chars);

        //std::cout << output_buf << std::endl;

        ASSERT_EQ(0, err);
        ASSERT_STREQ(good_json, output_buf);
        ASSERT_EQ(good_json_chars, output_json_chars);
	}

	/*
	 * 
	 * These tests are used with the Params section.
	 * Various items are tested, including parsing the Param comma string as well
	 * as the Enum comma string.
	 *
	 */

	TEST(libedsTests, convert_section2json_comma_value_parsing_param)
	{
		const char *input = "0,,,0x0000,0xD2,2,\"CONFIGURATION WORD 0\",\"individual bit-fields\",\"Configuration Word 0\",,,1024,,,,,,,,,";

		const char *good_json = "\"Reserved\":\"0\",\"PathSize\":\"null\",\"Path\":\"null\",\"Descriptor\":\"0x0000\",\"DataType\":\"0xD2\",\"DataSizeInBytes\":\"2\","
								"\"Name\":\"CONFIGURATION WORD 0\",\"Units\":\"individual bit-fields\",\"HelpString\":\"Configuration Word 0\",\"DataValues\":{\"min\":\"null\","
								"\"max\":\"null\",\"default\":\"1024\"},\"Scaling\":{\"mult\":\"null\",\"div\":\"null\",\"base\":\"null\",\"offset\":\"null\"},"
								"\"Links\":{\"mult\":\"null\",\"div\":\"null\",\"base\":\"null\",\"offset\":\"null\"},\"DecimalPlaces\":\"0\"";

		size_t good_json_chars = 420;
        size_t output_json_chars = 0;
		size_t output_buf_size = 4096;
		char output_buf[output_buf_size] = {0};

       	ERR_LIBEDS_t err = _parse_comma_delimited_val(DATATYPE_SPEC_PARAM, input, output_buf, output_buf_size, &output_json_chars);

       	ASSERT_STREQ(good_json, output_buf);
       	ASSERT_EQ(good_json_chars, output_json_chars);
       	ASSERT_EQ(0, err);
	}

	TEST(libedsTests, convert_section2json_comma_value_parsing_param_buf_overrun)
	{
		const char *input = "0,,,0x0000,0xD2,2,\"CONFIGURATION WORD 0\",\"individual bit-fields\",\"Configuration Word 0\",,,1024,,,,,,,,,";

		const char *good_json = "\"Reserved\":\"0\",\"PathSize\":\"null\",\"Path\":\"null\",\"Descriptor\":\"0x0000\",\"DataType\":\"0xD2\",\"DataSizeInBytes\":\"2\","
								"\"Name\":\"CONFIGURATION WORD 0\",\"Units\":\"individual bit-fields\",\"HelpString\":\"Configuration Word 0\",\"DataValues\":{\"min\":\"null\","
								"\"max\":\"null\",\"default\":\"1024\"},\"Scaling\":{\"mult\":\"null\",\"div\":\"null\",\"base\":\"null\",\"offset\":\"null\"},"
								"\"Links\":{\"mult\":\"null\",\"div\":\"null\",\"base\":\"null\",\"offset\":\"null\"},\"DecimalPlaces\":\"0\"";

		size_t good_json_chars = 420;
        size_t output_json_chars = 0;
		size_t output_buf_size = 5;
		char output_buf[output_buf_size] = {0};

       	ERR_LIBEDS_t err = _parse_comma_delimited_val(DATATYPE_SPEC_PARAM, input, output_buf, output_buf_size, &output_json_chars);

       	ASSERT_EQ(good_json_chars, output_json_chars);
       	ASSERT_EQ(1, err);
	}

	TEST(libedsTests, convert_section2json_comma_value_parsing_enum)
	{
		const char *input = "0,IN1 function bit 0,1,IN1 function bit 1,2,IN1 function bit 2,3,IN2 function bit 0,4,IN2 function bit 1,5,IN2 function bit 2,10,"
							"Use Encoder,11,Use Backplane Proximity,13,Enable Stall Detection,14,Disable Antiresonance";

		const char *good_json = "\"0\":\"IN1 function bit 0\",\"1\":\"IN1 function bit 1\",\"2\":\"IN1 function bit 2\","
								"\"3\":\"IN2 function bit 0\",\"4\":\"IN2 function bit 1\",\"5\":\"IN2 function bit 2\","
								"\"10\":\"Use Encoder\",\"11\":\"Use Backplane Proximity\",\"13\":\"Enable Stall Detection\","
								"\"14\":\"Disable Antiresonance\"";

		size_t good_json_chars = 258;
		size_t output_json_chars = 0;
		size_t output_buf_size = 4096;
		char output_buf[output_buf_size] = {0};

		ERR_LIBEDS_t err = _parse_comma_delimited_val(DATATYPE_SPEC_ENUM, input, output_buf, output_buf_size, &output_json_chars);

       	ASSERT_STREQ(good_json, output_buf);
       	ASSERT_EQ(good_json_chars, output_json_chars);
       	ASSERT_EQ(0, err);
	}

	TEST(libedsTests, convert_section2json_comma_value_parsing_enum_buf_overrun)
	{
		const char *input = "0,IN1 function bit 0,1,IN1 function bit 1,2,IN1 function bit 2,3,IN2 function bit 0,4,IN2 function bit 1,5,IN2 function bit 2,10,"
							"Use Encoder,11,Use Backplane Proximity,13,Enable Stall Detection,14,Disable Antiresonance";

		const char *good_json = "\"0\":\"IN1 function bit 0\",\"1\":\"IN1 function bit 1\",\"2\":\"IN1 function bit 2\","
								"\"3\":\"IN2 function bit 0\",\"4\":\"IN2 function bit 1\",\"5\":\"IN2 function bit 2\","
								"\"10\":\"Use Encoder\",\"11\":\"Use Backplane Proximity\",\"13\":\"Enable Stall Detection\","
								"\"14\":\"Disable Antiresonance\"";

		size_t good_json_chars = 257;
		size_t output_json_chars = 0;
		size_t output_buf_size = 7;
		char output_buf[output_buf_size] = {0};

		ERR_LIBEDS_t err = _parse_comma_delimited_val(DATATYPE_SPEC_ENUM, input, output_buf, output_buf_size, &output_json_chars);

       	ASSERT_EQ(good_json_chars, output_json_chars);
       	ASSERT_EQ(1, err);
	}

	TEST(libedsTests, convert_section2json_comma_value_parsing_bad_input)
	{
		const char *input = "foobar";
		size_t output_json_chars = 0;
		size_t output_buf_size = 4096;
		char output_buf[output_buf_size] = {0};

		// this should catch an invalid string going in
		// this is fine as we just return a properly formatted JSON object for the param with no values
		ERR_LIBEDS_t err = _parse_comma_delimited_val(DATATYPE_SPEC_PARAM, input, output_buf, output_buf_size, &output_json_chars);

		ASSERT_EQ(output_json_chars, 300);

		//this should test a weird type
		err = _parse_comma_delimited_val((SPECIAL_DATA_TYPES_t)56546151, input, output_buf, output_buf_size, &output_json_chars);
		ASSERT_EQ(output_json_chars, 0);
		ASSERT_EQ(err, 2);
	}

	TEST(libedsTests, convert_section2json_params_section_toJSON_buf_overflow)
	{
		const char *input = "\nParam1=0,,,0x0000,0xD2,2,\"CONFIGURATION WORD 0\",\"individual bit-fields\",\"Configuration Word 0\",,,1024,,,,,,,,,;\n"
							"Enum1=0,\"IN1 function bit 0\",1,"
							"\"IN1 function bit 1\",2,\"IN1 function bit 2\",3,\"IN2 function bit 0\",4,\"IN2 function bit 1\","
							"5,\"IN2 function bit 2\",10,\"Use Encoder\",11,\"Use Backplane Proximity\",13,"
							"\"Enable Stall Detection\",14,\"Disable Antiresonance\";\nParam2=0,,,0x0000,0xD2,"
							"2,\"CONFIGURATION WORD 1\",\"individual bit-fields\",\"Configuration Word 1\",,,771,"
							",,,,,,,,;\n";

		const char *good_json = "\"Params\":{\"Param1\":{\"Reserved\":\"0\",\"PathSize\":\"null\",\"Path\":\"null\",\"Descriptor\":\"0x0000\","
									"\"DataType\":\"0xD2\",\"DataSizeInBytes\":\"2\",\"Name\":\"CONFIGURATION WORD 0\",\"Units\":\"individual bit-fields\","
									"\"HelpString\":\"Configuration Word 0\",\"DataValues\":{\"min\":\"null\",\"max\":\"null\",\"default\":\"1024\"},"
									"\"Scaling\":{\"mult\":\"null\",\"div\":\"null\",\"base\":\"null\",\"offset\":\"null\"},\"Links\":{\"mult\":\"null\","
									"\"div\":\"null\",\"base\":\"null\",\"offset\":\"null\"},\"DecimalPlaces\":\"0\",\"Enum1\":{\"0\":\"IN1 function bit 0\","
									"\"1\":\"IN1 function bit 1\",\"2\":\"IN1 function bit 2\",\"3\":\"IN2 function bit 0\",\"4\":\"IN2 function bit 1\",\"5\":"
									"\"IN2 function bit 2\",\"10\":\"Use Encoder\",\"11\":\"Use Backplane Proximity\",\"13\":\"Enable Stall Detection\",\"14\":"
									"\"Disable Antiresonance\"}},\"Param2\":{\"Reserved\":\"0\",\"PathSize\":\"null\",\"Path\":\"null\",\"Descriptor\":\"0x0000\","
									"\"DataType\":\"0xD2\",\"DataSizeInBytes\":\"2\",\"Name\":\"CONFIGURATION WORD 1\",\"Units\":\"individual bit-fields\","
									"\"HelpString\":\"Configuration Word 1\",\"DataValues\":{\"min\":\"null\",\"max\":\"null\",\"default\":\"771\"},"
									"\"Scaling\":{\"mult\":\"null\",\"div\":\"null\",\"base\":\"null\",\"offset\":\"null\"},\"Links\":{\"mult\":\"null\","
									"\"div\":\"null\",\"base\":\"null\",\"offset\":\"null\"},\"DecimalPlaces\":\"0\"}}";

		size_t good_json_chars = 1142;
		size_t output_json_chars = 0;
		size_t output_buf_size = 1142;
		char output_buf[output_buf_size] = {0};

		ERR_LIBEDS_t err = convert_section2json(EDS_PARAMS, input, output_buf, output_buf_size, &output_json_chars);

		ASSERT_EQ(good_json_chars, output_json_chars);
       	ASSERT_EQ(1, err);
	}

	TEST(libedsTests, convert_section2json_params_section_toJSON)
	{
		const char *input = "\nParam1=0,,,0x0000,0xD2,2,\"CONFIGURATION WORD 0\",\"individual bit-fields\",\"Configuration Word 0\",,,1024,,,,,,,,,;\n"
							"Enum1=0,\"IN1 function bit 0\",1,"
							"\"IN1 function bit 1\",2,\"IN1 function bit 2\",3,\"IN2 function bit 0\",4,\"IN2 function bit 1\","
							"5,\"IN2 function bit 2\",10,\"Use Encoder\",11,\"Use Backplane Proximity\",13,"
							"\"Enable Stall Detection\",14,\"Disable Antiresonance\";\nParam2=0,,,0x0000,0xD2,"
							"2,\"CONFIGURATION WORD 1\",\"individual bit-fields\",\"Configuration Word 1\",,,771,"
							",,,,,,,,;\n";

		const char *good_json = "\"Params\":{\"Param1\":{\"Reserved\":\"0\",\"PathSize\":\"null\",\"Path\":\"null\",\"Descriptor\":\"0x0000\","
									"\"DataType\":\"0xD2\",\"DataSizeInBytes\":\"2\",\"Name\":\"CONFIGURATION WORD 0\",\"Units\":\"individual bit-fields\","
									"\"HelpString\":\"Configuration Word 0\",\"DataValues\":{\"min\":\"null\",\"max\":\"null\",\"default\":\"1024\"},"
									"\"Scaling\":{\"mult\":\"null\",\"div\":\"null\",\"base\":\"null\",\"offset\":\"null\"},\"Links\":{\"mult\":\"null\","
									"\"div\":\"null\",\"base\":\"null\",\"offset\":\"null\"},\"DecimalPlaces\":\"0\",\"Enum1\":{\"0\":\"IN1 function bit 0\","
									"\"1\":\"IN1 function bit 1\",\"2\":\"IN1 function bit 2\",\"3\":\"IN2 function bit 0\",\"4\":\"IN2 function bit 1\",\"5\":"
									"\"IN2 function bit 2\",\"10\":\"Use Encoder\",\"11\":\"Use Backplane Proximity\",\"13\":\"Enable Stall Detection\",\"14\":"
									"\"Disable Antiresonance\"}},\"Param2\":{\"Reserved\":\"0\",\"PathSize\":\"null\",\"Path\":\"null\",\"Descriptor\":\"0x0000\","
									"\"DataType\":\"0xD2\",\"DataSizeInBytes\":\"2\",\"Name\":\"CONFIGURATION WORD 1\",\"Units\":\"individual bit-fields\","
									"\"HelpString\":\"Configuration Word 1\",\"DataValues\":{\"min\":\"null\",\"max\":\"null\",\"default\":\"771\"},"
									"\"Scaling\":{\"mult\":\"null\",\"div\":\"null\",\"base\":\"null\",\"offset\":\"null\"},\"Links\":{\"mult\":\"null\","
									"\"div\":\"null\",\"base\":\"null\",\"offset\":\"null\"},\"DecimalPlaces\":\"0\"}}";

		size_t good_json_chars = 1142;
		size_t output_json_chars = 0;
		size_t output_buf_size = 2048;
		char output_buf[output_buf_size] = {0};

		ERR_LIBEDS_t err = convert_section2json(EDS_PARAMS, input, output_buf, output_buf_size, &output_json_chars);

		ASSERT_STREQ(output_buf, good_json);
		ASSERT_EQ(good_json_chars, output_json_chars);
       	ASSERT_EQ(0, err);
	}

	TEST(libedsTests, test_section_enum_from_section_name)
	{
		const char *section_name = "Connection Manager";
		PARSABLE_EDS_SECTIONS_t s = _section_enum_from_section_name(section_name);

		ASSERT_EQ(s, EDS_CONNECTION_MANAGER);
	}
}

int main(int argc, char **argv)
{
	testing::InitGoogleTest(&argc, argv);

	return RUN_ALL_TESTS();
}