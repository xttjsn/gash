/*
 * cmpl_basic.cc -- Basic testing for the compilation functionality
 *
 * Author: Xiaoting Tang <tang_xiaoting@brown.edu>
 * Copyright: Xiaoting Tang (2018)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "../include/common.hh"

TEST_F(CMPLTest, ADD_1)
{

    m_circ_stream = ofstream("add1.circ", std::ios::out | std::ios::trunc);
    m_data_stream = ofstream("add1.dat", std::ios::out | std::ios::trunc);
    extern FILE* yyin;
    const char* src = "func add(int1 a, int1 b) {      "
                      "    return a + b;               "
                      "}                               "
                      "#definput     a    0            "
                      "#definput     b    1            ";

    yyin = std::tmpfile();
    std::fputs(src, yyin);
    std::rewind(yyin);
    gashlang::set_ofstream(m_circ_stream, m_data_stream);

    int parse_result = yyparse();

    EXPECT_EQ(0, parse_result);
}

TEST_F(CMPLTest, ADD_8)
{

    m_circ_stream = ofstream("add8.circ", std::ios::out | std::ios::trunc);
    m_data_stream = ofstream("add8.dat", std::ios::out | std::ios::trunc);
    extern FILE* yyin;
    const char* src = "func add(int8 a, int8 b) {      "
                      "    return a + b;               "
                      "}                               "
                      "#definput     a    0            "
                      "#definput     b    1            ";

    yyin = std::tmpfile();
    std::fputs(src, yyin);
    std::rewind(yyin);
    gashlang::set_ofstream(m_circ_stream, m_data_stream);

    int parse_result = yyparse();

    EXPECT_EQ(0, parse_result);
}

TEST_F(CMPLTest, ADD_16)
{

    m_circ_stream = ofstream("add16.circ", std::ios::out | std::ios::trunc);
    m_data_stream = ofstream("add16.dat", std::ios::out | std::ios::trunc);
    extern FILE* yyin;
    const char* src = "func add(int16 a, int16 b) {    "
                      "    return a + b;               "
                      "}                               "
                      "#definput     a    0            "
                      "#definput     b    1            ";

    yyin = std::tmpfile();
    std::fputs(src, yyin);
    std::rewind(yyin);
    gashlang::set_ofstream(m_circ_stream, m_data_stream);

    int parse_result = yyparse();

    EXPECT_EQ(0, parse_result);
}

TEST_F(CMPLTest, ADD_32)
{

    m_circ_stream = ofstream("add32.circ", std::ios::out | std::ios::trunc);
    m_data_stream = ofstream("add32.dat", std::ios::out | std::ios::trunc);
    extern FILE* yyin;
    const char* src = "func add(int32 a, int32 b) {    "
                      "    return a + b;               "
                      "}                               "
                      "#definput     a    0            "
                      "#definput     b    1            ";

    yyin = std::tmpfile();
    std::fputs(src, yyin);
    std::rewind(yyin);
    gashlang::set_ofstream(m_circ_stream, m_data_stream);

    int parse_result = yyparse();

    EXPECT_EQ(0, parse_result);
}

TEST_F(CMPLTest, ADD_64)
{

    m_circ_stream = ofstream("add64.circ", std::ios::out | std::ios::trunc);
    m_data_stream = ofstream("add64.dat", std::ios::out | std::ios::trunc);
    extern FILE* yyin;
    const char* src = "func add(int64 a, int64 b) {    "
                      "    return a + b;               "
                      "}                               "
                      "#definput     a    0            "
                      "#definput     b    1            ";

    yyin = std::tmpfile();
    std::fputs(src, yyin);
    std::rewind(yyin);
    gashlang::set_ofstream(m_circ_stream, m_data_stream);

    int parse_result = yyparse();

    EXPECT_EQ(0, parse_result);
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
