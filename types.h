#ifndef Types_h
#define Types_h

#include <deque>
#include <map>
#include <vector>

typedef std::vector<std::string> Strings;
typedef std::deque<Strings> Results;
typedef std::map<char, int> LetterCounts;

extern unsigned char const kSowpodsAll[];
extern unsigned char const kTwlAll[];

#endif