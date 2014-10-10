#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "Trie.h"

static const int kMaxWords = -1;
extern const char kSowpodsAll[277751];

class WordReceiver : public WordListener
{
public:
  WordReceiver(int words) { wordlist.reserve(words); }
  virtual void word(std::string const& w)
  {
    wordlist.push_back(w);
  }

  std::vector<std::string> wordlist;
};

int main(int argc, char const* agrv[])
{
  // Create a Trie from a flat file word list
 /* {
    std::ifstream ifs("C:\\Users\\clarkson\\Dropbox\\Projects\\Word Lists\\sowpods.txt");
    if (!ifs.good()) {
      return EXIT_FAILURE;
    }

    Trie h;
    std::string word;
    int words = 0;
    while (std::getline(ifs, word)) {
      if (kMaxWords >= 0 && words >= kMaxWords) {
        break;
      }

      h.insert(word);
      ++words;
    }

    std::ofstream ofs("trie.txt");
    ofs << h;
  }*/

  //{
    Trie h;
    std::ifstream ifs("trie.txt");
    if (!ifs.good()) {
      return EXIT_FAILURE;
    }

    //// Convert to a static
    //std::ofstream ofs("static.txt");
    //if (!ofs.good()) {
    //  return EXIT_FAILURE;
    //}
    //char c; int wrap = 16; int written = 0;
    //while ((c = ifs.get()) != EOF) {
    //  ofs << static_cast<int>(c) << ",";
    //  if (!(++written % wrap)) {
    //    ofs << std::endl;
    //  }
    //}

    // Recreate the trie from a file
    ifs >> h;

    // Recreate the trie from a static
    //std::stringstream ss(kSowpodsAll);
    //ss >> h;

    // Generate a flat word list from the trie
    WordReceiver wg(h.words);
    h.get_words(wg);
  //}

  return EXIT_SUCCESS;
}