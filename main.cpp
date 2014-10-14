#include <deque>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "Trie.h"

static const size_t kMaxWords = 10000;
extern unsigned char const kSowpodsAll[];

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

class WordFinder
{
public:
  static std::deque<std::string> StartsWith(Trie const& t, std::string const& str)
  {
    return t.search(str);
  }

  static std::deque<std::string> Anagrams(Trie const& t, std::string const& str, size_t letters = 0)
  {
    return t.anagrams(str, letters);
  }

  static std::deque<std::vector<std::string>> Consume(Trie const& t, std::string const& str, size_t min_letters = 2)
  {
    return t.consume(str, min_letters);
  }
};

int main(int argc, char const* agrv[])
{
  // Create a Trie from a flat file word list
  std::ifstream ifs("C:\\Users\\clarkson\\Dropbox\\Projects\\Word Lists\\sowpods.txt");
  if (!ifs.good()) {
    return EXIT_FAILURE;
  }

  Trie h;
  std::string word;
  int words = 0;
  while (std::getline(ifs, word)) {
    if (kMaxWords && words >= kMaxWords) {
      break;
    }

    h.insert(word);
    ++words;
  }

  //std::ofstream ofs("trie.bin", std::ios::binary);
  //ofs << h;

  //{
  /* Trie h;
  std::ifstream ifs("trie.bin", std::ios::binary);
  if (!ifs.good()) {
  return EXIT_FAILURE;
  }*/

  // Recreate the trie from a file
  //ifs >> h;

  // Retrieve all words in a flat list
  //WordReceiver wr(h.words);
  //h.get_words(wr);

  //// Convert to a static
  //std::ofstream ofs("static.txt");
  //if (!ofs.good()) {
  //  return EXIT_FAILURE;
  //}
  //int wrap = 8; int written = 0;
  //while (!ifs.eof()) {
  //  unsigned char c;
  //  ifs.read(reinterpret_cast<char*>(&c), sizeof(c));

  //  std::stringstream ss;
  //  ss << "0x" << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(c) << ",";
  //  ofs << ss.str();
  //  if (!(++written % wrap)) {
  //    ofs << std::endl;
  //  }
  //}
  //ofs.close();

  // Recreate the trie from a static
  //Trie h;
  //Trie::read_static(kSowpodsAll, h);

  // Generate a flat word list from the trie
  //WordReceiver wr_static(r.words);
  //r.get_words(wr_static);
  //}

  //auto results = WordFinder::StartsWith(h, "aba");
  //auto results = WordFinder::StartsWith(h, "");

  //auto results = WordFinder::Anagrams(h, "sandra");

  // When consuming, can just consume "depth-first", because ordering doesn't matter
  // It will always return the words in alphabetical order, but that's OK because you can rearrange
  auto results = WordFinder::Consume(h, "ianclarkson");

  return EXIT_SUCCESS;
}