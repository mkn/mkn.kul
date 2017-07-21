/**
Copyright (c) 2017, Philip Deegan.
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

TEST(StringOperations, SplitByChar) {
    std::vector<std::string> v;
    kul::String::SPLIT("split - by - char - dash", '-', v);
    EXPECT_EQ((size_t) 4, v.size());
    EXPECT_EQ("split ", v[0]);
    EXPECT_EQ(" by ", v[1]);
    EXPECT_EQ(" char ", v[2]);
    EXPECT_EQ(" dash", v[3]);
}

TEST(StringOperations, SplitByString) {
    std::vector<std::string> v;
    kul::String::SPLIT("split - by - char - dash", "-", v);
    EXPECT_EQ((size_t) 4, v.size());
    EXPECT_EQ("split ", v[0]);
    EXPECT_EQ(" by ", v[1]);
    EXPECT_EQ(" char ", v[2]);
    EXPECT_EQ(" dash", v[3]);
}

TEST(StringOperations, SplitByEscapedChar) {
    std::vector<std::string> v;
    kul::String::ESC_SPLIT("split \\- by - char - dash", '-', v);
    EXPECT_EQ((size_t) 3, v.size());
    EXPECT_EQ("split - by ", v[0]);
    EXPECT_EQ(" char ", v[1]);
    EXPECT_EQ(" dash", v[2]);
}


TEST(StringOperations, String_2_UInt16_t_invalid_tooLarge) {
    tryCatch(
        {
            //toolarge
            [](){
                kul::String::UINT16(std::to_string((std::numeric_limits<uint16_t>::max)() + 10));
            }
        }, true
    );
}
TEST(StringOperations, String_2_UInt16_t_invalid_tooNegative) {
    tryCatch(
        {
            //tooNegative
            [](){
                kul::String::UINT16("-1");
            }
        }, true
    );
}
TEST(StringOperations, String_2_UInt16_t_invalid_hasLetters) {
    tryCatch(
        {
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
        }, true
    );
}
TEST(StringOperations, String_2_UInt16_t_valid) {
    tryCatch(
        {
            //valid
            [](){
                kul::String::UINT16("100");
            }
        }, false
    );
}

TEST(StringOperations, String_2_Int16_t_invalid_tooLarge) {
    tryCatch(
        {
            //toolarge
            [](){
                kul::String::INT16(std::to_string((std::numeric_limits<int16_t>::max)() + 10));
            }
        }, true
    );
}
TEST(StringOperations, String_2_Int16_t_invalid_tooNegative) {
    tryCatch(
        {
            //tooNegative
            [](){
                kul::String::INT16(std::to_string(((std::numeric_limits<int16_t>::max)() + 10) * -1));
            }
        }, true
    );
}
TEST(StringOperations, String_2_Int16_t_invalid_hasLetters) {
    tryCatch(
        {
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
        }, true
    );
}
TEST(StringOperations, String_2_Int16_t_valid) {
    tryCatch(
        {
            //valid
            [](){
                kul::String::INT16("100");
            },
            [](){
                kul::String::INT16("-100");
            }
        }, false
    );
}

TEST(StringOperations, String_2_UInt32_t_invalid_tooLarge) {
    tryCatch(
        {
            //toolarge
            [](){
                uint64_t val = (std::numeric_limits<uint32_t>::max)();
                val += 10;
                kul::String::UINT32(std::to_string(val));
            }
        }, true
    );
}
TEST(StringOperations, String_2_UInt32_t_invalid_tooNegative) {
    tryCatch(
        {
            //tooNegative
            [](){
                kul::String::UINT32(std::to_string(-1));
            }
        }, true
    );
}
TEST(StringOperations, String_2_UInt32_t_invalid_hasLetters) {
    tryCatch(
        {
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
        }, true
    );
}
TEST(StringOperations, String_2_UInt32_t_valid) {
    tryCatch(
        {
            //valid
            [](){
                kul::String::UINT32("100");
            }
        }, false
    );
}

TEST(StringOperations, String_2_Int32_t_invalid_tooLarge) {
    tryCatch(
        {
            //toolarge
            [](){
                int64_t val = (std::numeric_limits<int32_t>::max)();
                val += 10;
                kul::String::INT32(std::to_string(val));
            }
        }, true
    );
}
TEST(StringOperations, String_2_Int32_t_invalid_tooNegative) {
    tryCatch(
        {
            //tooNegative
            [](){
                int64_t val = (std::numeric_limits<int32_t>::max)();
                val += 10;
                kul::String::INT32("-" + std::to_string(val));
            }
        }, true
    );
}
TEST(StringOperations, String_2_Int32_t_invalid_hasLetters) {
    tryCatch(
        {
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
        }, true
    );
}
TEST(StringOperations, String_2_Int32_t_valid) {

    tryCatch(
        {
            //valid
            [](){
                kul::String::INT32("100");
            },
            [](){
                kul::String::INT32("-100");
            }
        }, false
    );
}

TEST(StringOperations, String_2_UInt64_t_invalid_tooLarge) {
    tryCatch(
        {
            //toolarge
            [](){
                uint64_t val = (std::numeric_limits<uint64_t>::max)();
                kul::String::UINT64(std::to_string(val) + "0");
            }
        }, true
    );
}
TEST(StringOperations, String_2_UInt64_t_invalid_tooNegative) {
    tryCatch(
        {
            //tooNegative
            [](){
                uint64_t val = (std::numeric_limits<uint64_t>::max)();
                kul::String::UINT64("-" + std::to_string(val) + "0");
            }
        }, true
    );
}
TEST(StringOperations, String_2_UInt64_t_invalid_hasLetters) {
    tryCatch(
        {
            //contains letters * 3
            [](){
                kul::String::UINT64("a2c");
            },
            [](){
                kul::String::UINT64("1bc");
            },
            [](){
                kul::String::UINT64("ab3");
            }
        }, true
    );
}
TEST(StringOperations, String_2_UInt64_t_valid) {
    tryCatch(
        {
            //valid
            [](){
                kul::String::UINT64(std::to_string((std::numeric_limits<uint64_t>::max)()));
            }
        }, false
    );
}

TEST(StringOperations, String_2_Int64_t_invalid) {
    tryCatch(
        {
            //toolarge
            [](){
                uint64_t val = (std::numeric_limits<uint64_t>::max)();
                kul::String::INT64(std::to_string(val) + "0");
            },
            //tooNegative
            [](){
                kul::String::INT64(std::to_string(-1));
            },
            //contains letters * 3
            [](){
                kul::String::INT64("a2c");
            },
            [](){
                kul::String::INT64("1bc");
            },
            [](){
                kul::String::INT64("ab3");
            }
        }, true
    );
}
TEST(StringOperations, String_2_Int64_t_valid) {
    tryCatch(
        {
            //valid
            [](){
                kul::String::INT64(std::to_string((std::numeric_limits<int64_t>::max)()));
            }
        }, false
    );
}
