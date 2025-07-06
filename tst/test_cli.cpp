
#include "mkn/kul/assert.hpp"
#include "mkn/kul/cli.hpp"

int main(int argc, char** argv) {
  std::vector<mkn::kul::cli::Arg> argV{
      mkn::kul::cli::Arg('a', "args", mkn::kul::cli::ArgType::STRING)};
  std::vector<mkn::kul::cli::Cmd> cmdV{mkn::kul::cli::Cmd("cmd")};
  mkn::kul::cli::Args args(cmdV, argV);
  args.process(argc, argv);
  KOUT(NON) << args.rest();
  return 0;
}
