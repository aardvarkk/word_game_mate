#include <cassert>
#include <iostream>

#include "tests.h"
#include "word_finder.h"
#include "string_utils.h"

void Tests::run() {
  // Can specify an exact number of letters to search for
  // If we want to search for a range, could just run this search multiple times (once for each value in the range)
  {
	  // auto results = WordFinder::Anagrams(h, "hithere", 6);
	  // for (auto r : results) {
	  //  for (auto w : r) {
	  //    cout << w << " ";
	  //  }
	  //  cout << endl;
	  // }
	  // auto results2 = WordFinder::StartsWith(h, "aba");
  	// auto results3 = WordFinder::StartsWith(h, "");
	}


  // TEST 1
  {
	  Trie h("test");
	  h.insert("hi");
	  h.insert("ma");
	  auto results = WordFinder::Anagrams(h, "ahim");
	  assert(results.size() == 2);
	  cout << results[0][0] << endl;
	  cout << results[1][0] << endl;

	  assert(results[0][0] == "hi");
	  assert(results[1][0] == "ma");
	}

  // TEST 2
  {
	  Trie h("test");
	  h.insert("aa");
	  h.insert("abac");
	  auto results = WordFinder::Anagrams(h, "abac");
	}

  // TEST 3
  {
  	Trie h("test");
	  h.insert("ae");
	  h.insert("nirl");
	  h.insert("st");
	  auto results = WordFinder::Anagrams(h, "starline");
	}

  // TEST 4
  {
  	Trie h("test");
	  h.insert("airlock");
	  h.insert("airlocks");
	  h.insert("nan");
	  h.insert("nans");
	  h.insert("ann");
	  h.insert("anns");
	  auto results = WordFinder::Anagrams(h, "nacakirosln");
		
		// When consuming, can just consume "depth-first", because ordering doesn't matter
  	// It will always return the words in alphabetical order, but that's OK because you can rearrange
  	auto results2 = WordFinder::Anagrams(h, "rankoilcans");
  }

  // TEST 5
  assert(StringUtils::compare_prioritize_length("a", "b") < 0);
  assert(StringUtils::compare_prioritize_length("aa", "b") > 0);
  assert(StringUtils::compare_prioritize_length("aa", "aa") == 0);
  assert(StringUtils::compare_prioritize_length("b", "a") > 0);
  assert(StringUtils::compare_prioritize_length("aa", "ab") < 0);
  assert(StringUtils::compare_prioritize_length("alky", "red") > 0);
  assert(StringUtils::compare_prioritize_length("nib", "red") < 0);

  // TEST 6
  {
	  Trie h("SOWPODS");
	  Strings box;
	  box.push_back("aa");
	  Trie::read_static(kSowpodsAll, h);
	  auto results = WordFinder::Box(
	   h,
	   box,
	   true
	   );
	}

  // TEST 7
  {
  	Trie h("SOWPODS");
	  Strings box;
	  box.push_back("sna");
	  box.push_back("sek");
	  Trie::read_static(kSowpodsAll, h);
	  auto results = WordFinder::Box(
	   h,
	   box,
	   true
	   );
	}
}