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

typedef std::vector<std::string> Strings;
typedef std::deque<Strings> Results;

static size_t const kMaxWords = 0; // Used only when loading flat word lists for debug
static char const*  kTempFile = "tmp.txt";

extern unsigned char const kSowpodsAll[];
extern unsigned char const kTwlAll[];

static std::vector<Trie*> wordlists_;

enum DisplayMethod
{
  kCommaSeparated = 0,
  kLine,
  kNumDisplayMethods
};

static DisplayMethod display_method_ = kCommaSeparated;

static size_t max_results_ = 50;

enum SortMethod
{
  kLongestWord = 0,
  kFewestWords,
  kMostWords,
  kNumSortMethods
};

static SortMethod sort_method_ = kLongestWord;

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

  static Results Box(
    Trie const& t,
    Strings const& box,
    bool consume_all,
    size_t* min_wordlet = nullptr,
    size_t* max_wordlet = nullptr
    )
  {
    if (box.empty() || box.front().empty()) {
      return Results();
    }

    // Adjust ranges for validity
    if (min_wordlet) {
      *min_wordlet = std::max<size_t>(*min_wordlet, 1);
    }
    if (max_wordlet) {
      *max_wordlet = std::min<size_t>(*max_wordlet, box.size() * box.front().size());
    }

    return t.box(box, consume_all, min_wordlet, max_wordlet);
  }

  static bool IsWord(
    Trie const& t,
    std::string const& word
    )
  {
    return t.search(word, word.length()).size() > 0;
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

std::string SortMethodString(SortMethod sort_method)
{
  switch (sort_method) {
  case kLongestWord:
    return "Longest Word";
  case kFewestWords:
    return "Fewest Words";
  case kMostWords:
    return "Most Words";
  default:
    return "Unknown";
  }
}

std::string DisplayMethodString(DisplayMethod display_method)
{
  switch (display_method) {
  case kLine:
    return "One Word Per Line";
  case kCommaSeparated:
    return "Comma Separated";
  default:
    return "Unknown";
  }
}

void draw_display()
{
  rlutil::setColor(rlutil::GREY);
  std::cout << "Loaded Word Lists: ";
  rlutil::setColor(rlutil::CYAN);
  if (wordlists_.empty()) {
    std::cout << "None";
  }
  else {
    for (size_t i = 0; i < wordlists_.size(); ++i) {
      if (i > 0) {
        rlutil::setColor(rlutil::GREY);
        std::cout << ", ";
        rlutil::setColor(rlutil::CYAN);
      }
      std::cout << wordlists_[i]->get_name();
    }
  }
  std::cout << std::endl;

  rlutil::setColor(rlutil::GREY);
  std::cout << "Sort Method: ";
  for (int i = 0; i < kNumSortMethods; ++i) {
    rlutil::setColor(static_cast<int>(sort_method_) == i ? rlutil::CYAN : rlutil::GREY);
    std::cout << SortMethodString(static_cast<SortMethod>(i));
    if (i < kNumSortMethods - 1) {
      rlutil::setColor(rlutil::GREY);
      std::cout << " | ";
    }
  }
  std::cout << std::endl;
  
  rlutil::setColor(rlutil::GREY);
  std::cout << "Display Method: ";
  for (int i = 0; i < kNumDisplayMethods; ++i) {
    rlutil::setColor(static_cast<int>(display_method_) == i ? rlutil::CYAN : rlutil::GREY);
    std::cout << DisplayMethodString(static_cast<DisplayMethod>(i));
    if (i < kNumDisplayMethods - 1) {
      rlutil::setColor(rlutil::GREY);
      std::cout << " | ";
    }
  }
  std::cout << std::endl;

  std::cout << std::endl;
  rlutil::setColor(rlutil::LIGHTCYAN);
  std::cout << "a: anagram" << std::endl;
  std::cout << "b: word box" << std::endl;
  std::cout << "c: change sort" << std::endl;
  std::cout << "d: change display" << std::endl;
  std::cout << "i: is ___ a word?" << std::endl;
  std::cout << "s: load SOWPODS" << std::endl;
  std::cout << "t: load TWL" << std::endl;
  std::cout << "u: unload wordlist" << std::endl;
  std::cout << "q: quit";
  rlutil::hidecursor();
}

int get_wordlist()
{
  if (wordlists_.empty()) {
    rlutil::setColor(rlutil::LIGHTMAGENTA);
    std::cout << "No wordlists loaded!" << std::endl;
    return -1;
  }

  rlutil::setColor(rlutil::LIGHTCYAN);

  // Need to choose a word list if we have multiple
  int idx = 0;
  if (wordlists_.size() > 1) {
    std::cout << "Choose a wordlist ";
    size_t i = 1;
    std::cout << "(";
    for (auto w : wordlists_) {
      if (i > 1) {
        std::cout << ", ";
      }
      std::cout << i++ << " = " << w->get_name();
    }
    std::cout << "):" << std::endl;
    std::string wordlist_str;
    std::getline(std::cin, wordlist_str);

    try {
      idx = std::stoi(wordlist_str) - 1;
    }
    catch (...)
    {
      std::cout << "Unable to parse numeric input" << std::endl;
      return -1;
    }

    if (idx < 0 || idx >= static_cast<int>(wordlists_.size())) {
      std::cout << "Numeric input out of range" << std::endl;
      return -1;
    }
  }

  return idx;
}

// Collects info on whether or not to consume all input and wordlet sizes
void command_common(size_t input_size, bool& consume_all, size_t& min_wordlet, size_t& max_wordlet)
{
  std::cout << "Consume all (y/n, default = y):" << std::endl;
  std::string consume_all_str;
  std::getline(std::cin, consume_all_str);
  consume_all = !(!consume_all_str.empty() && std::tolower(consume_all_str[0]) == 'n');

  // Set default wordlet sizes
  min_wordlet = 2;
  max_wordlet = input_size;

  std::cout << "Enter minimum wordlet size (default = " << min_wordlet << ", input length = " << input_size << "):" << std::endl;
  std::string min_wordlet_str;
  std::getline(std::cin, min_wordlet_str);
  try {
    min_wordlet = std::stoul(min_wordlet_str);
  }
  catch (...) {
  }

  std::cout << "Enter maximum wordlet size (default = " << max_wordlet << ", input length = " << input_size << "):" << std::endl;
  std::string max_wordlet_str;
  std::getline(std::cin, max_wordlet_str);
  try {
    max_wordlet = std::stoul(max_wordlet_str);
  }
  catch (...) {
  }
}

Results command_anagram(Trie const& wordlist)
{
  std::cout << "Enter letters:" << std::endl;
  std::string letters;
  std::getline(std::cin, letters);

  bool consume_all;
  size_t min_wordlet, max_wordlet;
  command_common(letters.size(), consume_all, min_wordlet, max_wordlet);

  return WordFinder::Anagrams(
    wordlist,
    letters,
    consume_all,
    min_wordlet > 0 ? &min_wordlet : nullptr,
    max_wordlet > 0 ? &max_wordlet : nullptr
    );
}

Results command_box(Trie const& wordlist)
{
  std::cout << "Enter box (finish lines with ';'):" << std::endl;
  Strings box;
  for (;;) {
    std::string line;
    std::getline(std::cin, line);

    // The letters to add
    std::string letter_str = line;

    // We've added a new line
    bool add_more = !line.empty() && line.back() == ';';
    if (add_more) {
      // Trim the letters, since we don't want the semicolon
      letter_str = line;
      letter_str.pop_back();
    }

    // Check that the letters we're adding match length of existing
    if (box.size() > 0 && letter_str.length() != box.back().length()) {
      rlutil::setColor(rlutil::LIGHTMAGENTA);
      std::cout << "Box line length doesn't match previous" << std::endl << std::endl;
      return Results();
    }

    // Add our letters to the box
    box.push_back(letter_str);

    // We're done if we didn't end with a semicolon
    if (!add_more) {
      break;
    }
  }

  if (box.empty() || box.front().empty()) {
    return Results();
  }

  bool consume_all;
  size_t min_wordlet, max_wordlet;
  command_common(box.size() * box.front().size(), consume_all, min_wordlet, max_wordlet);

  // We now have a box to process, so run a Trie algorithm on it
  return WordFinder::Box(wordlist, box, consume_all, &min_wordlet, &max_wordlet);
}

void command_is_word(Trie const& wordlist)
{
  std::cout << "Enter a word to check:" << std::endl;
  std::string line;
  std::getline(std::cin, line);

  bool isword = WordFinder::IsWord(wordlist, line);
  if (isword) {
    std::cout << "Yes, '" << line << "' is a word";
  }
  else {
    std::cout << "Sorry, '" << line << "' is not a word";
  }
  std::cout << " in the " << wordlist.get_name() << " wordlist" << std::endl;
}

Results sort_results(Results const& unsorted)
{
  auto sorted = unsorted;

  switch (sort_method_) {

    // Sort by longest word
  case kLongestWord:
  {
    std::sort(sorted.begin(), sorted.end(), [](Strings const& a, Strings const& b) -> bool
    {
      return StringUtils::compare_wordsets(a, b) > 0;
    });
  }
    break;

    // Sort by most words
  case kMostWords:
  {
    std::sort(sorted.begin(), sorted.end(), [](Strings const& a, Strings const& b) -> bool
    {
      if (a.size() != b.size()) {
        return a.size() > b.size();
      }
      // Same number of words, so must use sorting from longest word above to find lexicographically higher one
      else {
        return StringUtils::compare_wordsets(a, b) < 0;
      }
    });
  }
    break;

    // Sort by fewest words
  case kFewestWords:
  {
    std::sort(sorted.begin(), sorted.end(), [](Strings const& a, Strings const& b) -> bool
    {
      if (a.size() != b.size()) {
        return a.size() < b.size();
      }
      // Same number of words, so must use sorting from longest word above to find lexicographically higher one
      else {
        return StringUtils::compare_wordsets(a, b) > 0;
      }
    });
  }
    break;

  }

  return sorted;
}

void print_results(Results const& results)
{
  if (results.empty()) {
    rlutil::setColor(rlutil::LIGHTMAGENTA);
    std::cout << "No results" << std::endl << std::endl;
    return;
  }

  rlutil::setColor(rlutil::GREY);
  if (results.size() > max_results_) {
    std::cout << "Displaying top " << max_results_ << " of " << results.size() << " results:" << std::endl;
  }
  else {
    std::cout << "Results:" << std::endl;
  }

  rlutil::setColor(rlutil::LIGHTMAGENTA);
  switch (display_method_) {
  
  case kLine:
    for (size_t i = 0; i < std::min(results.size(), max_results_); ++i) {
      for (auto w : results[i]) {
        std::cout << w << " ";
      }
      std::cout << std::endl;
    }
    break;

  case kCommaSeparated:
  {
    auto cols = rlutil::tcols();
    size_t pos = 0;
    auto num_write = std::min(results.size(), max_results_);
    for (size_t i = 0; i < num_write; ++i) {
      
      // Generate string to write
      std::stringstream to_write;
      for (size_t j = 0; j < results[i].size(); ++j) {
        if (j > 0) {
          to_write << " ";
        }
        to_write << results[i][j];
      }

      // Add a comma if we have more results to write
      if (i < num_write-1) {
        to_write << ", ";
      }

      // New line if required
      if (pos + to_write.str().length() >= static_cast<size_t>(cols)) {
        std::cout << std::endl;
        pos = 0;
      }

      // If we have enough room, write it
      std::cout << to_write.str();
      pos += to_write.str().length();
    }
    std::cout << std::endl;
  }
    break;

  }
  std::cout << std::endl;
}

int command_loop()
{
  bool quit = false;

  while (!quit) {
    draw_display();
    
    // Wait for a keypress, and clear once we've received it
    auto key = rlutil::getkey();
    rlutil::cls();

    switch (tolower(key)) {

      // Do a word anagram
    case 'a':
    {
      auto wl_idx = get_wordlist();
      if (wl_idx < 0) {
        break;
      }
      auto sorted = sort_results(command_anagram(*wordlists_[wl_idx]));
      print_results(sorted);
    }
      break;

      // Accept letters in "box" format
    case 'b':
    {
      auto wl_idx = get_wordlist();
      if (wl_idx < 0) {
        break;
      }
      auto sorted = sort_results(command_box(*wordlists_[wl_idx]));
      print_results(sorted);
    }
      break;

    case 'c':
      sort_method_ = static_cast<SortMethod>((static_cast<int>(sort_method_)+1)%kNumSortMethods);
      break;

    case 'd':
      display_method_ = static_cast<DisplayMethod>(!static_cast<int>(display_method_));
      break;

    case 'i':
    {
      auto wl_idx = get_wordlist();
      if (wl_idx < 0) {
        break;
      }
      command_is_word(*wordlists_[wl_idx]);
    }
      break;

    case 's':
    {
      auto s = new Trie("SOWPODS");
      Trie::read_static(kSowpodsAll, *s);
      wordlists_.push_back(s);
    }
      break;

    case 't':
    {
      auto t = new Trie("TWL");
      Trie::read_static(kTwlAll, *t);
      wordlists_.push_back(t);
    }
      break;

    case 'u':
    {
      int idx = get_wordlist();
      if (idx >= 0) {
        wordlists_.erase(wordlists_.begin()+idx);
      }
    }
      break;

    case 'q':
      quit = true;
      break;

    }
  }

  for (auto wl : wordlists_) {
    delete wl;
  }

  return EXIT_SUCCESS;
}

void wordlist_to_binary(
  std::string const& name,
  std::string const& in_path, 
  std::string const& out_path,
  size_t max_words = 0
  )
{
  // Create a Trie from a flat file word list
  std::ifstream ifs(in_path);
  if (!ifs.good()) {
    return;
  }

  Trie t(name);
  std::string word;
  size_t words = 0;
  while (std::getline(ifs, word)) {
    if (max_words && words >= max_words) {
      break;
    }

    t.insert(word);
    ++words;
  }

  // Write a tree to a file
  std::ofstream ofs(out_path, std::ios::binary);
  if (!ofs.good()) {
    return;
  }
  ofs << t;
  ofs.close();
}

void binary_to_static(std::string const& in_path, std::string const& out_path)
{
  std::ofstream ofs(out_path);
  if (!ofs.good()) {
    return;
  }
  std::ifstream ifs(in_path, std::ios::binary);
  int wrap = 8; int written = 0;
  while (!ifs.eof()) {
    unsigned char c;
    ifs.read(reinterpret_cast<char*>(&c), sizeof(c));
    std::stringstream ss;
    ss << "0x" << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(c) << ",";
    ofs << ss.str();
    if (!(++written % wrap)) {
      ofs << std::endl;
    }
  }
  ofs.close();
}

void wordlist_to_static(std::string const& name, std::string const& in_path, std::string const& out_path)
{
  wordlist_to_binary(name, in_path, kTempFile);
  binary_to_static(kTempFile, out_path);
  std::remove(kTempFile);
}

int main(int argc, char const* agrv[])
{
  //wordlist_to_static(
  //  "TWL",
  //  "TWL06.txt",
  //  "static.txt"
  //  );
  //return EXIT_SUCCESS;

  // Recreate the trie from a file
  //Trie h("SOWPODS");
  //std::ifstream ifs("trie.bin", std::ios::binary);
  //if (!ifs.good()) {
  //  return EXIT_FAILURE;
  //}
  //ifs >> h;

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

  //// TEST 6
  //Trie h("SOWPODS");
  //Strings box;
  //box.push_back("aa");
  //Trie::read_static(kSowpodsAll, h);
  //auto results = WordFinder::Box(
  //  h,
  //  box,
  //  true
  //  );
  //print_results(results);

  //// TEST 7
  //Trie h("SOWPODS");
  //Strings box;
  //box.push_back("sna");
  //box.push_back("sek");
  //Trie::read_static(kSowpodsAll, h);
  //auto results = WordFinder::Box(
  //  h,
  //  box,
  //  true
  //  );
  //print_results(results);

  // Can set background colours by shifting background color by 4
  //rlutil::setColor((rlutil::BLUE << 4) | rlutil::YELLOW);
  rlutil::cls();

  rlutil::setColor(rlutil::YELLOW);
  std::cout << "Word Game Mate" << std::endl;
  std::cout << std::endl;

  command_loop();

  return EXIT_SUCCESS;
}