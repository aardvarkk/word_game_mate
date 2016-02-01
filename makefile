word_game_mate: main.cpp tests.cpp sowpods_all.cpp twl_all.cpp cpptrie/Trie.cpp cpptrie/TrieNode.cpp
	g++ -std=c++11 -Wall $(CFLAGS) -I./cpptrie -I./ -o word_game_mate main.cpp tests.cpp sowpods_all.cpp twl_all.cpp cpptrie/Trie.cpp cpptrie/TrieNode.cpp

.PHONY: clean
clean:
	rm -f word_game_mate
