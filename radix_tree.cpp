#include <algorithm>

#include "radix_tree.h"

RadixTreeNode::RadixTreeNode(RadixTreeEdge* incoming) :
  incoming(incoming)
{
  outgoing.clear();
}

RadixTreeNode::~RadixTreeNode()
{
  for (auto e : outgoing) {
    delete e;
  }
}

bool RadixTreeNode::isLeaf() 
{ 
  return outgoing.empty(); 
}

RadixTreeEdge::RadixTreeEdge(std::string const& x) :
  label(x)
{
  targetNode = new RadixTreeNode(this);
}

RadixTreeEdge::~RadixTreeEdge() 
{
  delete targetNode;
}

RadixTree::RadixTree() : root(nullptr)
{
}

RadixTree::~RadixTree()
{
  if (root) {
    delete root;
    root = nullptr;
  }
}

RadixTreeNode* RadixTree::closest(std::string const& x, size_t& elementsFound)
{
  // Begin at the root with no elements found
  RadixTreeNode* traverseNode = root;
  elementsFound = 0;
  
  // Traverse until a leaf is found or it is not possible to continue
  while (traverseNode != nullptr && !traverseNode->isLeaf() && elementsFound < x.length())
  {
    // Get the next edge to explore based on the elements not yet found in x
    RadixTreeEdge* nextEdge = nullptr; // = select edge from traverseNode->outgoing where RadixTreeEdge.label is a prefix of x.suffix(elementsFound)
      // x.suffix(elementsFound) returns the last (x.length - elementsFound) elements of x
    for (auto e : traverseNode->outgoing) {
      auto prefix = x.substr(elementsFound, elementsFound + e->label.length());
      if (!prefix.compare(e->label)) {
        nextEdge = e;
      }
    }
  
    // Was an edge found?
    if (nextEdge != nullptr)
    {
      // Set the next node to explore
      traverseNode = nextEdge->targetNode;
    
      // Increment elements found based on the label stored at the edge
      elementsFound += nextEdge->label.length();
    }
    else
    {
      // Terminate loop
      break;
      //traverseNode = nullptr;
    }
  }

  return traverseNode;
}

bool RadixTree::lookup(std::string const& x)
{
  size_t elementsFound;
  auto traverseNode = closest(x, elementsFound);
  
  // A match is found if we arrive at a leaf RadixTreeNode and have used up exactly x.length elements
  return (traverseNode != nullptr && traverseNode->isLeaf() && elementsFound == x.length());
}

int RadixTree::common_len(std::string const& a, std::string const& b)
{
  return std::min(a.length(), b.length());
}

bool RadixTree::common_match(std::string const& a, std::string const& b)
{
  auto len = common_len(a, b);
  return !a.substr(0, len).compare(b.substr(0, len));
}

void RadixTree::insert(std::string const& x)
{
  size_t elementsFound;
  auto traverseNode = closest(x, elementsFound);

  // Found a closest node...
  if (traverseNode) {
    
    // Do we share a common prefix?
    RadixTreeEdge* match = nullptr;
    if (traverseNode->incoming && common_match(x, traverseNode->incoming->label)) {
      match = traverseNode->incoming;
    }
    
    // We don't match any edges up to the common length, so add a new outgoing edge
    if (!match) {
      traverseNode->outgoing.push_back(new RadixTreeEdge(x));
    }
    // We match up to a common prefix
    else
    {
      // Shorter becomes the incoming and longer becomes a new child
      auto len = common_len(x, match->label);
      int n = 0;
    }
  } 
  // Just starting the tree...
  else {
    root = new RadixTreeNode(nullptr);
    root->outgoing.push_back(new RadixTreeEdge(x));
  }
}