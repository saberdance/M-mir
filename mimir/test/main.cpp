#include "../utils/mUtil.h"
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem::v1;

int main(int argc, char** argv)
{
	fs::path p1("./bbb/aaa");
	logger.log(p1.filename().string());
	fs::path p2("./bbb/mimirPack");
	logger.log(p2.filename().string());
	fs::copy(p1, fs::path(p2.string()+"/"+p1.filename().string()));
}