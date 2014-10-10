#pragma once

#include <vector>

struct RadixTreeEdge;

struct RadixTreeNode
{
  RadixTreeNode(RadixTreeEdge* incoming);
  ~RadixTreeNode();
  bool isLeaf();
  RadixTreeEdge* incoming;
  std::vector<RadixTreeEdge*> outgoing;
};

struct RadixTreeEdge
{
  RadixTreeEdge(std::string const& x);
  ~RadixTreeEdge();
  std::string label;
  RadixTreeNode* targetNode;
};

struct RadixTree
{
  RadixTree();
  ~RadixTree();
  RadixTreeNode* closest(std::string const& x, size_t& elementsFound);
  bool lookup(std::string const& x);
  int common_len(std::string const& a, std::string const& b);
  bool common_match(std::string const& a, std::string const& b);
  void insert(std::string const& x);
  RadixTreeNode* root;
};
