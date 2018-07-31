#ifndef POINCARE_SERIALIZABLE_REFERENCE_H
#define POINCARE_SERIALIZABLE_REFERENCE_H

#include <poincare/preferences.h>
#include <poincare/tree_reference.h>
#include <poincare/serializable_node.h>

namespace Poincare {

class SerializableReference : public TreeReference {
public:
  using TreeReference::TreeReference;

  SerializableNode * node() const override { return static_cast<SerializableNode *>(TreeReference::node()); }
  // Serialization
  bool needsParenthesisWithParent(SerializableReference parentRef) {
    assert(isDefined());
    return node()->needsParenthesisWithParent(parentRef.node());
  }
  int writeTextInBuffer(char * buffer, int bufferSize, Preferences::PrintFloatMode floatDisplayMode = Preferences::PrintFloatMode::Decimal, int numberOfSignificantDigits = 0) const {
    assert(isDefined());
    return node()->writeTextInBuffer(buffer, bufferSize, floatDisplayMode, numberOfSignificantDigits);
  }

  // Tree
  SerializableReference serializableChildAtIndex(int i) {
    TreeReference treeRefChild = TreeReference::treeChildAtIndex(i);
    return SerializableReference(treeRefChild.node());
  }
};

typedef SerializableReference SerializableRef;

}

#endif