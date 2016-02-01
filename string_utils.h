#ifndef StringUtils_h
#define StringUtils_h

#include "types.h"

#include <set>

using namespace std;

enum SortMethod
{
	kLongestWord = 0,
	kFewestWords,
	kMostWords,
	kNumSortMethods
};

class StringUtils
{
public:
  static inline int compare_prioritize_length(std::string const& a, std::string const& b) {
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
	
	static inline bool in_search_order(std::string const& a, std::string const& b) {
		if (a.empty()) return false;
		if (b.empty()) return true;
		if (b.front() > a.front()) return true;
		else if (b.front() == a.front()) {
			return in_search_order(a.substr(1, a.length()-1), b.substr(1, b.length()-1));
		} else return false;
	}

  static inline int compare_wordsets(Strings const& a, Strings const& b) {
    // Sort each wordset
    auto sorted_a = a;
    sort(sorted_a.begin(), sorted_a.end(), [](string const& a, string const& b) -> bool { return StringUtils::compare_prioritize_length(a, b) > 0; });
    auto sorted_b = b;
    sort(sorted_b.begin(), sorted_b.end(), [](string const& a, string const& b) -> bool { return StringUtils::compare_prioritize_length(a, b) > 0; });

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
	
	static void sort_results(Results& unsorted, SortMethod sort_method)
	{
		switch (sort_method) {

			// Sort by longest word
		case kLongestWord:
		{
			sort(unsorted.begin(), unsorted.end(), [](Strings const& a, Strings const& b) -> bool
			{
				if (a.empty()) return false;
				if (b.empty()) return true;
				
				multiset<size_t> lengthsA;
				multiset<size_t> lengthsB;
				for (auto& s : a) { lengthsA.insert(s.length()); }
				for (auto& s : b) { lengthsB.insert(s.length()); }
				if (*lengthsA.rbegin() > *lengthsB.rbegin()) return true;
				return false;
			});
		}
			break;

			// Sort by most words
		case kMostWords:
		{
			sort(unsorted.begin(), unsorted.end(), [](Strings const& a, Strings const& b) -> bool
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
			sort(unsorted.begin(), unsorted.end(), [](Strings const& a, Strings const& b) -> bool
			{
				return a.size() < b.size();
			});
		}
			break;

		default:
			break;
		}
	}
};

#endif