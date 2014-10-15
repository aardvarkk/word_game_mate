#include <algorithm>
#include <cassert>
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

  // Anagrams finds words that can be made from the source string
  static std::deque<std::string> Anagrams(Trie const& t, std::string const& str, size_t letters = 0)
  {
    return t.anagrams(str, letters);
  }

  // "Consume" allows multiple words to be created to use the source string
  static std::deque<std::vector<std::string>> Consume(Trie const& t, std::string const& str, size_t min_letters = 2)
  {
    return t.consume(str, min_letters);
  }
};

class StringUtils
{
public:
  static inline int compare_prioritize_length(std::string const& a, std::string const& b)
  {
    if (a.size() > b.size()) {
      return 1;
    } else if (a.size() < b.size()) {
      return -1;
    } else {
      return a.compare(b);
    }
  }

  static inline int compare_wordsets(std::vector<std::string> const& a, std::vector<std::string> const& b) {
    // Sort each wordset
    auto sorted_a = a;
    std::sort(sorted_a.begin(), sorted_a.end(), [](std::string const& a, std::string const& b) -> bool { return StringUtils::compare_prioritize_length(a, b) > 0; });
    auto sorted_b = b;
    std::sort(sorted_b.begin(), sorted_b.end(), [](std::string const& a, std::string const& b) -> bool { return StringUtils::compare_prioritize_length(a, b) > 0; });

    // Go through each wordset, starting at the top
    // Keep descending until we find a difference...
    auto a_it = sorted_a.begin();
    auto b_it = sorted_b.begin();
    while (StringUtils::compare_prioritize_length(*a_it, *b_it) == 0) {
      ++a_it;
      ++b_it;
    }
    return StringUtils::compare_prioritize_length(*a_it, *b_it) > 0;
  }
};

int main(int argc, char const* agrv[])
{
  //// Create a Trie from a flat file word list
  //std::ifstream ifs("C:\\Users\\clarkson\\Dropbox\\Projects\\Word Lists\\sowpods.txt");
  //if (!ifs.good()) {
  //  return EXIT_FAILURE;
  //}

  //Trie h;
  //std::string word;
  //int words = 0;
  //while (std::getline(ifs, word)) {
  //  if (kMaxWords && words >= kMaxWords) {
  //    break;
  //  }

  //  h.insert(word);
  //  ++words;
  //}

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
  Trie h;
  Trie::read_static(kSowpodsAll, h);

  // TEST
  //auto results = WordFinder::Anagrams(h, "hi?", 4);
  auto results = WordFinder::Consume(h, "batca?", 3);

  // Can specify an exact number of letters to search for
  // If we want to search for a range, could just run this search multiple times (once for each value in the range)
  //auto results = WordFinder::Anagrams(h, "hithere", 6);
  for (auto r : results) {
    for (auto w : r) {
      std::cout << w << " ";
    }
    std::cout << std::endl;
  }

  // Generate a flat word list from the trie
  //WordReceiver wr_static(r.words);
  //r.get_words(wr_static);
  //}

  //auto results = WordFinder::StartsWith(h, "aba");
  //auto results = WordFinder::StartsWith(h, "");

  // TEST 1
  //Trie h;
  //h.insert("hi");
  //h.insert("ma");
  //auto results = WordFinder::Consume(h, "ahim");

  // TEST 2
  //Trie h;
  //h.insert("aa");
  //h.insert("abac");
  //auto results = WordFinder::Consume(h, "abac");

  //// TEST 3
  //Trie h;
  //h.insert("ae");
  //h.insert("nirl");
  //h.insert("st");
  //auto results = WordFinder::Consume(h, "starline");

  //// TEST 4
  //Trie h;
  //h.insert("airlock");
  //h.insert("airlocks");
  //h.insert("nan");
  //h.insert("nans");
  //h.insert("ann");
  //h.insert("anns");
  //auto results = WordFinder::Consume(h, "nacakirosln");

  // TEST 5
  //assert(StringUtils::compare_prioritize_length("a", "b") < 0);
  //assert(StringUtils::compare_prioritize_length("aa", "b") > 0);
  //assert(StringUtils::compare_prioritize_length("aa", "aa") == 0);
  //assert(StringUtils::compare_prioritize_length("b", "a") > 0);
  //assert(StringUtils::compare_prioritize_length("aa", "ab") < 0);
  //assert(StringUtils::compare_prioritize_length("alky", "red") > 0);
  //assert(StringUtils::compare_prioritize_length("nib", "red") < 0);

  // When consuming, can just consume "depth-first", because ordering doesn't matter
  // It will always return the words in alphabetical order, but that's OK because you can rearrange
  //auto results = WordFinder::Consume(h, "rankoilcans");

  // Sort by longest word
  //auto sorted_longest = results;
  //std::sort(sorted_longest.begin(), sorted_longest.end(), [](std::vector<std::string> const& a, std::vector<std::string> const& b) -> bool
  //{
  //  return StringUtils::compare_wordsets(a, b) > 0;
  //});

  // Sort by fewest words
  //auto sorted_fewest = results;
  //std::sort(sorted_fewest.begin(), sorted_fewest.end(), [](std::vector<std::string> const& a, std::vector<std::string> const& b) -> bool
  //{
  //  if (a.size() != b.size()) {
  //    return a.size() < b.size();
  //  }
  //  // Same number of words, so must use sorting from longest word above to find lexicographically higher one
  //  else {
  //    return StringUtils::compare_wordsets(a, b) > 0;
  //  }
  //});

  //// Sort by most words
  //auto sorted_most = results;
  //std::sort(sorted_most.begin(), sorted_most.end(), [](std::vector<std::string> const& a, std::vector<std::string> const& b) -> bool
  //{
  //  if (a.size() != b.size()) {
  //    return a.size() > b.size();
  //  }
  //  // Same number of words, so must use sorting from longest word above to find lexicographically higher one
  //  else {
  //    return StringUtils::compare_wordsets(a, b) < 0;
  //  }
  //});

  //for (size_t i = 0; i < std::min<size_t>(sorted_fewest.size(), 500); ++i) {
  //  for (auto w : sorted_fewest[i]) {
  //    std::cout << w << " ";
  //  }
  //  std::cout << std::endl;
  //}

  return EXIT_SUCCESS;
}