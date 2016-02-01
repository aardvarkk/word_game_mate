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

#include "word_finder.h"
#include "string_utils.h"
#include "tests.h"

using namespace rlutil;

static size_t const kMaxWords = 0; // Used only when loading flat word lists for debug
static char const*  kTempFile = "tmp.txt";

static vector<Trie*> wordlists_;

enum DisplayMethod
{
	kCommaSeparated = 0,
	kLine,
	kNumDisplayMethods
};

static DisplayMethod display_method_ = kCommaSeparated;

static size_t max_results_ = 100;

static SortMethod sort_method_ = kLongestWord;

string SortMethodString(SortMethod sort_method)
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

string DisplayMethodString(DisplayMethod display_method)
{
	switch (display_method) {
	case kLine:
		return "One Result Per Line";
	case kCommaSeparated:
		return "Comma Separated";
	default:
		return "Unknown";
	}
}

void draw_display()
{
	setColor(GREY);
	cout << "Loaded Word Lists: ";
	setColor(CYAN);
	if (wordlists_.empty()) {
		cout << "None";
	}
	else {
		for (size_t i = 0; i < wordlists_.size(); ++i) {
			if (i > 0) {
				setColor(GREY);
				cout << ", ";
				setColor(CYAN);
			}
			cout << wordlists_[i]->get_name();
		}
	}
	cout << endl;

	setColor(GREY);
	cout << "Sort Method: ";
	for (int i = 0; i < kNumSortMethods; ++i) {
		setColor(static_cast<int>(sort_method_) == i ? CYAN : GREY);
		cout << SortMethodString(static_cast<SortMethod>(i));
		if (i < kNumSortMethods - 1) {
			setColor(GREY);
			cout << " | ";
		}
	}
	cout << endl;
	
	setColor(GREY);
	cout << "Display Method: ";
	for (int i = 0; i < kNumDisplayMethods; ++i) {
		setColor(static_cast<int>(display_method_) == i ? CYAN : GREY);
		cout << DisplayMethodString(static_cast<DisplayMethod>(i));
		if (i < kNumDisplayMethods - 1) {
			setColor(GREY);
			cout << " | ";
		}
	}
	cout << endl;

	cout << endl;
	setColor(BLUE);
	cout << "a: anagram" << endl;
	cout << "b: word box" << endl;
	cout << "c: change sort" << endl;
	cout << "d: change display" << endl;
	cout << "i: is ___ a word?" << endl;
	cout << "r: crossword fill-in" << endl;
	cout << "s: load SOWPODS" << endl;
	cout << "t: load TWL" << endl;
	cout << "u: unload wordlist" << endl;
	cout << "q: quit";
	hidecursor();
}

int get_wordlist()
{
	if (wordlists_.empty()) {
		setColor(LIGHTMAGENTA);
		cout << "No wordlists loaded!" << endl;
		return -1;
	}

	setColor(LIGHTCYAN);

	// Need to choose a word list if we have multiple
	int idx = 0;
	if (wordlists_.size() > 1) {
		cout << "Choose a wordlist ";
		size_t i = 1;
		cout << "(";
		for (auto w : wordlists_) {
			if (i > 1) {
				cout << ", ";
			}
			cout << i++ << " = " << w->get_name();
		}
		cout << "):" << endl;
		string wordlist_str;
		getline(cin, wordlist_str);

		try {
			idx = stoi(wordlist_str) - 1;
		}
		catch (...)
		{
			cout << "Unable to parse numeric input" << endl;
			return -1;
		}

		if (idx < 0 || idx >= static_cast<int>(wordlists_.size())) {
			cout << "Numeric input out of range" << endl;
			return -1;
		}
	}

	return idx;
}

// Collects info on whether or not to consume all input and wordlet sizes
void command_common(size_t input_size, bool& consume_all, size_t& min_wordlet, size_t& max_wordlet)
{
	cout << "Consume all (y/n, default = y):" << endl;
	string consume_all_str;
	getline(cin, consume_all_str);
	consume_all = !(!consume_all_str.empty() && tolower(consume_all_str[0]) == 'n');

	// Set default wordlet sizes
	min_wordlet = 2;
	max_wordlet = input_size;

	cout << "Enter minimum wordlet size (default = " << min_wordlet << ", input length = " << input_size << "):" << endl;
	string min_wordlet_str;
	getline(cin, min_wordlet_str);
	try {
		min_wordlet = stoul(min_wordlet_str);
	}
	catch (...) {
	}

	cout << "Enter maximum wordlet size (default = " << max_wordlet << ", input length = " << input_size << "):" << endl;
	string max_wordlet_str;
	getline(cin, max_wordlet_str);
	try {
		max_wordlet = stoul(max_wordlet_str);
	}
	catch (...) {
	}
}

Results command_anagram(Trie const& wordlist)
{
	cout << "Enter letters:" << endl;
	string letters;
	getline(cin, letters);

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

Results command_crossword(Trie const& wordlist) {
	cout << "Enter letters in order using ? for unknown letters" << endl;
	string letters;
	getline(cin, letters);

	return WordFinder::Crossword(
		wordlist,
		letters
		);
}

Results command_box(Trie const& wordlist)
{
	cout << "Enter box (finish lines with ';'):" << endl;
	Strings box;
	for (;;) {
		string line;
		getline(cin, line);

		// The letters to add
		string letter_str = line;

		// We've added a new line
		bool add_more = !line.empty() && line.back() == ';';
		if (add_more) {
			// Trim the letters, since we don't want the semicolon
			letter_str = line;
			letter_str.pop_back();
		}

		// Check that the letters we're adding match length of existing
		if (box.size() > 0 && letter_str.length() != box.back().length()) {
			setColor(LIGHTMAGENTA);
			cout << "Box line length doesn't match previous" << endl << endl;
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
	cout << "Enter a word to check:" << endl;
	string line;
	getline(cin, line);

	bool isword = WordFinder::IsWord(wordlist, line);
	if (isword) {
		cout << "Yes, '" << line << "' is a word";
	}
	else {
		cout << "Sorry, '" << line << "' is not a word";
	}
	cout << " in the " << wordlist.get_name() << " wordlist" << endl;
}

void print_results(Results const& results)
{
	if (results.empty()) {
		setColor(LIGHTMAGENTA);
		cout << "No results" << endl << endl;
		return;
	}

	setColor(GREY);
	if (results.size() > max_results_) {
		cout << "Displaying top " << max_results_ << " of " << results.size() << " results:" << endl;
	}
	else {
		cout << "Results:" << endl;
	}

	setColor(LIGHTMAGENTA);
	switch (display_method_) {
	
	case kLine:
		for (size_t i = 0; i < min(results.size(), max_results_); ++i) {
			for (auto w : results[i]) {
				cout << w << " ";
			}
			cout << endl;
		}
		break;

	case kCommaSeparated:
	{
		auto cols = tcols();
		size_t pos = 0;
		auto num_write = min(results.size(), max_results_);
		for (size_t i = 0; i < num_write; ++i) {
			
			// Generate string to write
			stringstream to_write;
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
				cout << endl;
				pos = 0;
			}

			// If we have enough room, write it
			cout << to_write.str();
			pos += to_write.str().length();
		}
		cout << endl;
	}
		break;

	}
	cout << endl;
}

int command_loop()
{
	bool quit = false;

	while (!quit) {
		draw_display();
		
		// Wait for a keypress, and clear once we've received it
		auto key = getkey();
		cls();

		switch (tolower(key)) {

			// Do a word anagram
		case 'a':
		{
			auto wl_idx = get_wordlist();
			if (wl_idx < 0) {
				break;
			}
			auto results = command_anagram(*wordlists_[wl_idx]);
			StringUtils::sort_results(results, sort_method_);
			print_results(results);
		}
			break;

			// Accept letters in "box" format
		case 'b':
		{
			auto wl_idx = get_wordlist();
			if (wl_idx < 0) {
				break;
			}
			auto results = command_box(*wordlists_[wl_idx]);
			StringUtils::sort_results(results, sort_method_);
			print_results(results);
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

		case 'r':
		{
			auto wl_idx = get_wordlist();
			if (wl_idx < 0) {
				break;
			}
			auto results = command_crossword(*wordlists_[wl_idx]);
			StringUtils::sort_results(results, sort_method_);
			print_results(results);
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

	resetColor();

	return EXIT_SUCCESS;
}

void wordlist_to_binary(
	string const& name,
	string const& in_path, 
	string const& out_path,
	size_t max_words = 0
	)
{
	// Create a Trie from a flat file word list
	ifstream ifs(in_path);
	if (!ifs.good()) {
		return;
	}

	Trie t(name);
	string word;
	size_t words = 0;
	while (getline(ifs, word)) {
		if (max_words && words >= max_words) {
			break;
		}

		t.insert(word);
		++words;
	}

	// Write a tree to a file
	ofstream ofs(out_path, ios::binary);
	if (!ofs.good()) {
		return;
	}
	ofs << t;
	ofs.close();
}

void binary_to_static(string const& in_path, string const& out_path)
{
	ofstream ofs(out_path);
	if (!ofs.good()) {
		return;
	}
	ifstream ifs(in_path, ios::binary);
	int wrap = 8; int written = 0;
	while (!ifs.eof()) {
		unsigned char c;
		ifs.read(reinterpret_cast<char*>(&c), sizeof(c));
		stringstream ss;
		ss << "0x" << setfill('0') << setw(2) << hex << static_cast<int>(c) << ",";
		ofs << ss.str();
		if (!(++written % wrap)) {
			ofs << endl;
		}
	}
	ofs.close();
}

void wordlist_to_static(string const& name, string const& in_path, string const& out_path)
{
	wordlist_to_binary(name, in_path, kTempFile);
	binary_to_static(kTempFile, out_path);
	remove(kTempFile);
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
	//ifstream ifs("trie.bin", ios::binary);
	//if (!ifs.good()) {
	//  return EXIT_FAILURE;
	//}
	//ifs >> h;

	// Recreate the trie from a static
	//Trie h;
	//Trie::read_static(kSowpodsAll, h);

	Tests::run();
//	return EXIT_SUCCESS;
	
	// Can set background colours by shifting background color by 4
	//setColor((BLUE << 4) | YELLOW);
	cls();

	setColor(BLUE);
	cout << "Word Game Mate" << endl;
	cout << endl;

	command_loop();

	return EXIT_SUCCESS;
}