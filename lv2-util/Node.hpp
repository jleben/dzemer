#pragma once

#include <lilv/lilv.h>

#include <string>

namespace LV2 {

using std::string;

class Node
{
  LilvNode * own_node = nullptr;
  const LilvNode * node = nullptr;

public:
  struct InvalidConversion {};

  Node(LilvNode * node): own_node(node), node(node) {}

  Node(const LilvNode * node): node(node) {}

  ~Node() { lilv_node_free(own_node); }

  Node(const Node & other):
    Node(lilv_node_duplicate(other.node))
  {}

  string to_uri()
  {
    if (!lilv_node_is_uri(node))
      throw InvalidConversion();
    return lilv_node_as_uri(node);
  }

  string to_string()
  {
    if (!lilv_node_is_string(node))
      throw InvalidConversion();
    return lilv_node_as_string(node);
  }

  int to_int()
  {
    if (!lilv_node_is_int(node))
      throw InvalidConversion();
    return lilv_node_as_int(node);
  }

  float to_float()
  {
    if (!lilv_node_is_float(node))
      throw InvalidConversion();
    return lilv_node_as_float(node);
  }

  bool to_bool()
  {
    if (!lilv_node_is_bool(node))
      throw InvalidConversion();
    return lilv_node_as_bool(node);
  }

  const LilvNode * get() const { return node; }
};

}
