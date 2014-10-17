#include <algorithm>
#include <cassert>
#include <cctype>
#include <deque>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#define NOMINMAX
#include "rlutil/rlutil.h"

#include "Trie.h"

static const size_t kMaxResults = 8;
static const size_t kMaxWords = 0;
extern unsigned char const kSowpodsAll[];

typedef std::vector<std::string> Strings;
typedef std::deque<Strings> Results;
static std::vector<Trie*> wordlists_;

class WordReceiver : public WordListener
{
public:
  WordReceiver(int words) { wordlist.reserve(words); }
  virtual void word(std::string const& w)
  {
    wordlist.push_back(w);
  }

  Strings wordlist;
};

class WordFinder
{
public:
  static std::deque<std::string> StartsWith(Trie const& t, std::string const& str)
  {
    return t.search(str);
  }

  // Take the source string and make words from it
  // Will create multiple words if possible
  // Will take into account min and max word lengths for sub-words
  static Results Anagrams(
    Trie const& t,
    std::string const& str,
    bool consume_all = true,
    size_t* min_wordlet = nullptr,
    size_t* max_wordlet = nullptr
    )
  {
    // Adjust ranges for validity
    if (min_wordlet) {
      *min_wordlet = std::max<size_t>(*min_wordlet, 1);
    }
    if (max_wordlet) {
      *max_wordlet = std::min<size_t>(*max_wordlet, str.size());
    }

    return t.anagrams(str, consume_all, min_wordlet, max_wordlet);
  }
};

class StringUtils
{
public:
  static inline int compare_prioritize_length(std::string const& a, std::string const& b)
  {
    if (a.size() > b.size()) {
      return 1;
    }
    else if (a.size() < b.size()) {
      return -1;
    }
    else {
      return a.compare(b);
    }
  }

  static inline int compare_wordsets(Strings const& a, Strings const& b) {
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

void draw_display()
{
  rlutil::setColor(rlutil::YELLOW);
  std::cout << std::endl;
  std::cout << "Word Game Mate" << std::endl;
  std::cout << std::endl;
  rlutil::setColor(rlutil::WHITE);
  std::cout << "Loaded Word Lists:" << std::endl;
  if (wordlists_.empty()) {
    std::cout << "None" << std::endl;
  }
  else {
    for (size_t i = 0; i < wordlists_.size(); ++i) {
      std::cout << i + 1 << ". " << wordlists_[i]->get_name() << std::endl;
    }
  }
  std::cout << std::endl;
  rlutil::setColor(rlutil::LIGHTCYAN);
  std::cout << "Commands:" << std::endl;
  std::cout << "a: anagram (longest word)" << std::endl;
  std::cout << "w: anagram (most words)" << std::endl;
  std::cout << "e: anagram (fewest words)" << std::endl;
  std::cout << "s: load SOWPODS" << std::endl;
  std::cout << "q: quit" << std::endl;
  std::cout << std::endl;
}

Results command_anagram()
{
  if (wordlists_.empty()) {
    rlutil::setColor(rlutil::LIGHTRED);
    std::cout << "No wordlists loaded!" << std::endl;
    return Results();
  }

  rlutil::setColor(rlutil::LIGHTGREEN);

  // Need to choose a word list if we have multiple
  int wordlist = 0;
  if (wordlists_.size() > 1) {
    std::cout << "Choose a wordlist:" << std::endl;
    std::string wordlist_str;
    std::getline(std::cin, wordlist_str);
    wordlist = std::stoi(wordlist_str);
  }
  std::cout << "Enter letters:" << std::endl;
  std::string letters;
  std::getline(std::cin, letters);

  std::cout << "Consume all (y/n, default = y):" << std::endl;
  std::string consume_all_str;
  std::getline(std::cin, consume_all_str);
  bool consume_all = !(!consume_all_str.empty() && std::tolower(consume_all_str[0]) == 'n');

  // Set default wordlet sizes based on whether we want to consume the entire string
  size_t min_wordlet = consume_all ? letters.size() : 2;
  size_t max_wordlet = consume_all ? letters.size() : 2;

  std::cout << "Enter minimum wordlet size (default = " << min_wordlet << "):" << std::endl;
  std::string min_wordlet_str;
  std::getline(std::cin, min_wordlet_str);
  try {
    min_wordlet = std::stoul(min_wordlet_str);
  }
  catch (...) {
  }

  std::cout << "Enter maximum wordlet size (default = " << max_wordlet << "):" << std::endl;
  std::string max_wordlet_str;
  std::getline(std::cin, max_wordlet_str);
  try {
    max_wordlet = std::stoul(max_wordlet_str);
  }
  catch (...) {
  }

  return WordFinder::Anagrams(
    *wordlists_[wordlist],
    letters,
    consume_all,
    min_wordlet > 0 ? &min_wordlet : nullptr,
    max_wordlet > 0 ? &max_wordlet : nullptr
    );
}

void print_results(Results results)
{
  rlutil::setColor(rlutil::LIGHTMAGENTA);

  std::cout << std::endl;

  if (results.empty()) {
    std::cout << "No results" << std::endl;
    return;
  }

  if (results.size() > kMaxResults) {
    std::cout << "Displaying top " << kMaxResults << " of " << results.size() << " results:" << std::endl;
  }

  std::cout << "Results:" << std::endl;
  for (size_t i = 0; i < std::min(results.size(), kMaxResults); ++i) {
    for (auto w : results[i]) {
      std::cout << w << " ";
    }
    std::cout << std::endl;
  }
}

int command_loop()
{
  bool quit = false;

  // Can set background colours by shifting background color by 4
  //rlutil::setColor((rlutil::BLUE << 4) | rlutil::YELLOW);
  rlutil::cls();

  while (!quit) {
    draw_display();
    auto key = rlutil::getkey();
    switch (tolower(key)) {
    case 'a':
    {
      // Sort by longest word
      auto sorted_longest = command_anagram();
      std::sort(sorted_longest.begin(), sorted_longest.end(), [](Strings const& a, Strings const& b) -> bool
      {
        return StringUtils::compare_wordsets(a, b) > 0;
      });
      print_results(sorted_longest);
    }
    break;
    case 'w':
    {
      // Sort by most words
      auto sorted_most = command_anagram();
      std::sort(sorted_most.begin(), sorted_most.end(), [](Strings const& a, Strings const& b) -> bool
      {
        if (a.size() != b.size()) {
          return a.size() > b.size();
        }
        // Same number of words, so must use sorting from longest word above to find lexicographically higher one
        else {
          return StringUtils::compare_wordsets(a, b) < 0;
        }
      });
      print_results(sorted_most);
    }
    break;
    case 'e':
    {
      // Sort by fewest words
      auto sorted_fewest = command_anagram();
      std::sort(sorted_fewest.begin(), sorted_fewest.end(), [](Strings const& a, Strings const& b) -> bool
      {
        if (a.size() != b.size()) {
          return a.size() < b.size();
        }
        // Same number of words, so must use sorting from longest word above to find lexicographically higher one
        else {
          return StringUtils::compare_wordsets(a, b) > 0;
        }
      });
      print_results(sorted_fewest);
    }
    break;
    case 's':
    {
      auto s = new Trie("SOWPODS");
      Trie::read_static(kSowpodsAll, *s);
      wordlists_.push_back(s);
    }
    break;
    case 'q':
    {
      quit = true;
    }
    break;
    }
  }

  for (auto wl : wordlists_) {
    delete wl;
  }

  return EXIT_SUCCESS;
}

int main(int argc, char const* agrv[])
{
  command_loop();

  //// Create a Trie from a flat file word list
  //std::ifstream ifs_flat("C:\\Users\\clarkson\\Dropbox\\Projects\\Word Lists\\sowpods.txt");
  //if (!ifs_flat.good()) {
  //  return EXIT_FAILURE;
  //}
  //Trie h_flat("SOWPODS");
  //std::string word;
  //int words = 0;
  //while (std::getline(ifs_flat, word)) {
  //  if (kMaxWords && words >= kMaxWords) {
  //    break;
  //  }

  //  h_flat.insert(word);
  //  ++words;
  //}

  //// Write a tree to a file
  //std::ofstream ofs("trie.bin", std::ios::binary);
  //if (!ofs.good()) {
  //  return EXIT_FAILURE;
  //}
  //ofs << h_flat;
  //ofs.close();

  // Recreate the trie from a file
  //Trie h("SOWPODS");
  //std::ifstream ifs("trie.bin", std::ios::binary);
  //if (!ifs.good()) {
  //  return EXIT_FAILURE;
  //}
  //ifs >> h;

  // Retrieve all words in a flat list
  //WordReceiver wr(h.words);
  //h.get_words(wr);

  //// Convert to a static
  //std::ofstream ofs_static("static.txt");
  //if (!ofs_static.good()) {
  //  return EXIT_FAILURE;
  //}
  //int wrap = 8; int written = 0;
  //while (!ifs.eof()) {
  //  unsigned char c;
  //  ifs.read(reinterpret_cast<char*>(&c), sizeof(c));
  //  std::stringstream ss;
  //  ss << "0x" << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(c) << ",";
  //  ofs_static << ss.str();
  //  if (!(++written % wrap)) {
  //    ofs_static << std::endl;
  //  }
  //}
  //ofs_static.close();

  // Recreate the trie from a static
  //Trie h;
  //Trie::read_static(kSowpodsAll, h);

  // Can specify an exact number of letters to search for
  // If we want to search for a range, could just run this search multiple times (once for each value in the range)
  //auto results = WordFinder::Anagrams(h, "hithere", 6);
  //for (auto r : results) {
  //  for (auto w : r) {
  //    std::cout << w << " ";
  //  }
  //  std::cout << std::endl;
  //}

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

  return EXIT_SUCCESS;
}