#include <fstream>
#include <string>
#include <vector>

#include "Trie.h"

extern const int kSowpodsAllCount;
extern char const* kSowpodsAll[];

int main(int argc, char const* agrv[])
{
  /*std::ifstream ifs("C:\\Users\\clarkson\\Dropbox\\Projects\\Word Lists\\sowpods.txt");
  if (!ifs.good()) {
    return EXIT_FAILURE;
  }*/

  Trie h;
  for (int i = 0; i < kSowpodsAllCount; ++i) {
    h.insert(kSowpodsAll[i]);
  }

  std::ofstream ofs("trie.txt");
  ofs << h;

  return EXIT_SUCCESS;
}