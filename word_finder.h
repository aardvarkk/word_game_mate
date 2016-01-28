#ifndef WordFinder_h
#define WordFinder_h

#include "types.h"

#include "Trie.h"

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
      *min_wordlet = max<size_t>(*min_wordlet, 1);
    }
    if (max_wordlet) {
      *max_wordlet = min<size_t>(*max_wordlet, str.size());
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
      *min_wordlet = max<size_t>(*min_wordlet, 1);
    }
    if (max_wordlet) {
      *max_wordlet = min<size_t>(*max_wordlet, box.size() * box.front().size());
    }

    return t.box(box, consume_all, min_wordlet, max_wordlet);
  }

  static bool IsWord(
    Trie const& t,
    std::string const& word
    )
  {
    auto results = t.search(word, word.length());
    return results.size() > 0 && results.front() == word;
  }
};

#endif