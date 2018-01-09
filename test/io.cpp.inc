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

TEST(IO_Test, ReadFileLine) {
    kul::io::Reader r("LICENSE.md");
    const char* c = r.readLine();
    std::string s1 = c, s2;
    while((c = r.readLine())) s2 = c;
    EXPECT_EQ("Copyright (c) 2017, Philip Deegan.", s1);
    EXPECT_EQ("OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.", s2);
}
TEST(IO_Test, ReadFile) {
    char c[20] = {0};
    bzero(c, 20);
    kul::File file("LICENSE.md");
    if(!file) KEXCEPT(kul::Exception, "ReadFile: FileNotFound: ") << file.full();
    kul::io::Reader r("LICENSE.md");
    r.read(c, 20);
    std::string s1 = c;
    std::stringstream ss;
    while((r.read(c, 20))) { ss << c; break; }
    EXPECT_EQ("Copyright (c) 2017, ", s1);
    EXPECT_EQ("Philip Deegan.\nAll r", ss.str());
}

// Travis has a tough time with this one
#ifndef __APPLE__
TEST(IO_Test, ReadBinaryFileLine) {
    kul::io::BinaryReader r("LICENSE.md");
    const char* c = r.readLine();
    std::string s1 = c, s2;
    while((c = r.readLine())) s2 = c;
    EXPECT_EQ("Copyright (c) 2017, Philip Deegan.", s1);
    EXPECT_EQ("OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.", s2);
}
TEST(IO_Test, ReadBinaryFile) {
    char c[20] = {0};
    bzero(c, 20);
    kul::io::BinaryReader r("LICENSE.md");
    r.read(c, 20);
    std::string s1 = c;
    KLOG(INF) << s1.size();
    KLOG(INF) << strlen(c);
    std::stringstream ss;
    while((r.read(c, 20))) { ss << c; break; }
    EXPECT_EQ("Copyright (c) 2017, ", s1);
    EXPECT_EQ("Philip Deegan.\nAll r", ss.str());
}
#endif 
