#ifndef POINCARE_FLOAT_H
#define POINCARE_FLOAT_H

#include <poincare/number.h>
#include <poincare/approximation_helper.h>

namespace Poincare {

/* Float reprensents an approximated number. This class is use to avoid turning
 * float/double into Decimal back and forth because performances are
 * dramatically affected when doing so. For instance, when plotting a graph, we
 * need to set a float/double value for a symbol and approximate an expression
 * containing the symbol for each dot displayed).
 * We thus use the Float class that hold a float/double.
 * Float can only be approximated ; Float is an INTERNAL node only. Indeed,
 * they are always turned back into Decimal when beautifying. Thus, the usual
 * methods of expression are not implemented to avoid code duplication with
 * Decimal. */

template<typename T>
class FloatNode : public NumberNode {
public:
  FloatNode() : m_value(0.0) {}

  static FloatNode * FailedAllocationStaticNode();
  FloatNode * failedAllocationStaticNode() override { return FailedAllocationStaticNode(); }

  void setFloat(T a) { m_value = a; }
  T value() const { return m_value; }

  // TreeNode
  size_t size() const override { return sizeof(FloatNode<T>); }
#if TREE_LOG
  const char * description() const override { return "Float";  }
#endif

  // Properties
  Type type() const override { return Type::Float; }

  // Layout
  int serialize(char * buffer, int bufferSize, Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const override {
    assert(false);
    return 0;
  }
  /* Layout */
  LayoutRef createLayout(Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const override {
   assert(false);
   return nullptr;
  }
  /* Evaluation */
  Evaluation<float> approximate(SinglePrecision p, Context& context, Preferences::AngleUnit angleUnit) const override { return templatedApproximate<float>(context, angleUnit); }
  Evaluation<double> approximate(DoublePrecision p, Context& context, Preferences::AngleUnit angleUnit) const override { return templatedApproximate<double>(context, angleUnit); }
private:
  template<typename U> Evaluation<U> templatedApproximate(Context& context, Preferences::AngleUnit angleUnit) const {
    return Complex<U>((U)m_value);
  }
  T m_value;
};

template<typename T>
class Float : public Number {
public:
  Float(T value) : Number(TreePool::sharedPool()->createTreeNode<FloatNode<T>>()) {
    node()->setFloat(value);
  }
private:
  FloatNode<T> * node() const override { return static_cast<FloatNode<T> *>(Number::node()); }
};

}

#endif