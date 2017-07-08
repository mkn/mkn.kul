/**
Copyright (c) 2016, Philip Deegan.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

    * Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
copyright notice, this list of conditions and the following disclaimer
in the documentation and/or other materials provided with the
distribution.
    * Neither the name of Philip Deegan nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "kul/io.hpp"
#include "kul/os.hpp"
#include "kul/cli.hpp"
#include "kul/log.hpp"
#include "kul/proc.hpp"

TEST(StringTest, ParseCommandLineArguments) {
    std::vector<std::string> v;
    kul::cli::asArgs("/path/to \"words in quotes\" words\\ not\\ in\\ quotes end", v);
    EXPECT_EQ((size_t) 4, v.size());
    EXPECT_EQ("/path/to", v[0]);
    EXPECT_EQ("words in quotes", v[1]);
    EXPECT_EQ("words not in quotes", v[2]);
    EXPECT_EQ("end", v[3]);
}

TEST(StringTest, SplitByChar) {
    std::vector<std::string> v;
    kul::String::SPLIT("split - by - char - dash", '-', v);
    EXPECT_EQ((size_t) 4, v.size());
    EXPECT_EQ("split ", v[0]);
    EXPECT_EQ(" by ", v[1]);
    EXPECT_EQ(" char ", v[2]);
    EXPECT_EQ(" dash", v[3]);
}

TEST(StringTest, SplitByString) {
    std::vector<std::string> v;
    kul::String::SPLIT("split - by - char - dash", "-", v);
    EXPECT_EQ((size_t) 4, v.size());
    EXPECT_EQ("split ", v[0]);
    EXPECT_EQ(" by ", v[1]);
    EXPECT_EQ(" char ", v[2]);
    EXPECT_EQ(" dash", v[3]);
}

TEST(StringTest, SplitByEscapedChar) {
    std::vector<std::string> v;
    kul::String::ESC_SPLIT("split \\- by - char - dash", '-', v);
    EXPECT_EQ((size_t) 3, v.size());
    EXPECT_EQ("split - by ", v[0]);
    EXPECT_EQ(" char ", v[1]);
    EXPECT_EQ(" dash", v[2]);
}

TEST(IOTest, ReadFileLine) {
    kul::io::Reader r("LICENSE.md");
    const char* c = r.readLine();
    std::string s1 = c, s2;
    while((c = r.readLine())) s2 = c;
    EXPECT_EQ("Copyright (c) 2013, Philip Deegan.", s1);
    EXPECT_EQ("OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.", s2);
}
TEST(IOTest, ReadFile) {
    kul::io::Reader r("LICENSE.md");
    const char* c = r.read(20);
    std::string s1 = c;
    std::stringstream ss;
    while((c = r.read(20))) { ss << c; break; }
    EXPECT_EQ("Copyright (c) 2013, ", s1);
    EXPECT_EQ("Philip Deegan.\nAll r", ss.str());
}

TEST(IOTest, ReadBinaryFileLine) {
    kul::io::BinaryReader r("LICENSE.md");
    const char* c = r.readLine();
    std::string s1 = c, s2;
    while((c = r.readLine())) s2 = c;
    EXPECT_EQ("Copyright (c) 2013, Philip Deegan.", s1);
    EXPECT_EQ("OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.", s2);
}
TEST(IOTest, ReadBinaryFile) {
    kul::io::Reader r("LICENSE.md");
    const char* c = r.read(20);
    std::string s1 = c;
    std::stringstream ss;
    while((c = r.read(20))) { ss << c; break; }
    EXPECT_EQ("Copyright (c) 2013, ", s1);
    EXPECT_EQ("Philip Deegan.\nAll r", ss.str());
}

TEST(OperatingSystemTests, HasRAMUsageSupport) {
    ASSERT_TRUE(kul::this_proc::physicalMemory());
    ASSERT_TRUE(kul::this_proc::virtualMemory());
}

auto tryCatch = [](std::vector<std::function<void()>> funcs){
    for(const auto& func : funcs)
        try{
            func();
            ASSERT_TRUE(0);
        }catch(const kul::StringException& e){
            ASSERT_TRUE(1);
            KOUT(DBG) << e.debug();
        }
};

TEST(StringOperations, String_2_UInt16_t) {
    tryCatch(
        {
            //toolarge
            [](){
                kul::String::UINT16(std::to_string((std::numeric_limits<uint16_t>::max)() + 10));
            },
            //tooNegative
            [](){
                kul::String::UINT16("-1");
            },
            //contains letters * 3
            [](){
                kul::String::UINT16("a2c");
            },
            [](){
                kul::String::UINT16("1bc");
            },
            [](){
                kul::String::UINT16("ab3");
            }
        }
    );

    std::string positive = "100";
    kul::String::UINT16(positive);
    ASSERT_TRUE(1);
}


TEST(StringOperations, String_2_Int16_t) {
    tryCatch(
        {
            //toolarge
            [](){
                kul::String::INT16(std::to_string((std::numeric_limits<int16_t>::max)() + 10));
            },
            //tooNegative
            [](){
                kul::String::INT16(std::to_string(((std::numeric_limits<int16_t>::max)() + 10) * -1));
            },
            //contains letters * 3
            [](){
                kul::String::INT16("a2c");
            },
            [](){
                kul::String::INT16("1bc");
            },
            [](){
                kul::String::INT16("ab3");
            }
        }
    );

    kul::String::INT16("100");
    ASSERT_TRUE(1);
    kul::String::INT16("-100");
    ASSERT_TRUE(1);
}

TEST(StringOperations, String_2_UInt32_t) {
    tryCatch(
        {
            //toolarge
            [](){
                uint64_t val = (std::numeric_limits<uint32_t>::max)();
                val += 10;
                kul::String::UINT32(std::to_string(val));
            },
            //tooNegative
            [](){
                kul::String::UINT32(std::to_string(-1));
            },
            //contains letters * 3
            [](){
                kul::String::UINT32("a2c");
            },
            [](){
                kul::String::UINT32("1bc");
            },
            [](){
                kul::String::UINT32("ab3");
            }
        }
    );

    kul::String::UINT32("100");
    ASSERT_TRUE(1);
}

TEST(StringOperations, String_2_Int32_t) {
    tryCatch(
        {
            //toolarge
            [](){
                int64_t val = (std::numeric_limits<int32_t>::max)();
                val += 10;
                kul::String::INT32(std::to_string(val));
            },
            //tooNegative
            [](){
                int64_t val = (std::numeric_limits<int32_t>::max)();
                val += 10;
                kul::String::INT32("-" + std::to_string(val));
            },
            //contains letters * 3
            [](){
                kul::String::INT32("a2c");
            },
            [](){
                kul::String::INT32("1bc");
            },
            [](){
                kul::String::INT32("ab3");
            }
        }
    );

    kul::String::INT32("100");
    ASSERT_TRUE(1);
    kul::String::INT32("-100");
    ASSERT_TRUE(1);
}


class TimeStampHandler{
    private:
        kul::File f;
        kul::fs::TimeStamps fts;
        TimeStampHandler() : f("mkn.yaml"), fts(f.timeStamps()){}
    public:
        static TimeStampHandler&  INSTANCE(){
            static TimeStampHandler i;
            return i;
        }
        bool is(){ return f.is(); }
        const kul::fs::TimeStamps& timeStamps() { return fts; }
};

TEST(OperatingSystemTests, HasFileAccessedTimeStampSupport) {
    ASSERT_TRUE(TimeStampHandler::INSTANCE().is());
    ASSERT_TRUE(TimeStampHandler::INSTANCE().timeStamps().accessed());
}
#ifdef _WIN32
TEST(OperatingSystemTests, HasFileCreatedTimeStampSupport) {
    ASSERT_TRUE(TimeStampHandler::INSTANCE().is());
    ASSERT_TRUE(TimeStampHandler::INSTANCE().timeStamps().created());
}
#endif
TEST(OperatingSystemTests, HasFileModifiedTimeStampSupport) {
    ASSERT_TRUE(TimeStampHandler::INSTANCE().is());
    ASSERT_TRUE(TimeStampHandler::INSTANCE().timeStamps().modified());
}


int main(int argc, char* argv[]) {
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}