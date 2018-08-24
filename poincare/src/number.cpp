#include <poincare/number.h>
#include <poincare/decimal.h>
#include <poincare/integer.h>
#include <poincare/rational.h>
#include <poincare/float.h>
#include <poincare/undefined.h>
#include <poincare/infinity.h>

extern "C" {
#include <stdlib.h>
#include <assert.h>
}
#include <cmath>

namespace Poincare {

double NumberNode::doubleApproximation() const {
  switch (type()) {
    case Type::Undefined:
      return NAN;
    case Type::Infinity:
      return sign() == Sign::Negative ? -INFINITY : INFINITY;
    case Type::Float:
      if (sizeof(*this) == sizeof(FloatNode<float>)) {
        return static_cast<const FloatNode<float> *>(this)->value();
      } else {
        assert(sizeof(*this) == sizeof(FloatNode<double>));
        return static_cast<const FloatNode<double> *>(this)->value();
      }
    case Type::Rational:
      return static_cast<const RationalNode *>(this)->templatedApproximate<double>();
    case Type::Integer:
      return static_cast<const IntegerNode *>(this)->templatedApproximate<double>();
    default:
      assert(false);
      return 0.0;
  }
}

Number Number::ParseDigits(const char * digits, size_t length) {
  const char * integral = digits;
  size_t integralLength = length;
  const char * fractional = strchr(digits, '.');
  size_t fractionalLength = 0;
  if (fractional) {
    integralLength = fractional - integral;
    fractional++;
    fractionalLength = length - integralLength - 1;
  }
  const char * exponent = strchr(digits, Ion::Charset::Exponent);
  size_t exponentLength = 0;
  if (exponent) {
    integralLength = fractional ? integralLength : exponent - integral;
    fractionalLength = fractional ? exponent - fractional : 0;
    exponent++;
    exponentLength = length - integralLength - fractionalLength - (fractional ? 2 : 1);
  }
  // Integer
  if (exponentLength == 0 && fractionalLength == 0) {
    Integer i(digits, length, false);
    if (!i.isInfinity()) {
      return i;
    }
  }
  // Avoid overflowing int
  if (exponentLength > Decimal::k_maxExponentLength) {
    assert(exponent);
    if (exponent[0] == '-') {
      return Decimal(0.0);
    } else {
      return Infinity(false);
    }
  }
  // Avoid Decimal with exponent > k_maxExponentLength
  int exp = Decimal::Exponent(integral, integralLength, fractional, fractionalLength, exponent, exponentLength);
  if (exp > Decimal::k_maxExponent) {
    return Infinity(false);
  } else if (exp < -Decimal::k_maxExponent) {
    return Decimal(0.0);
  }
  return Decimal(integral, integralLength, fractional, fractionalLength, exp);
}

template <typename T>
Number Number::DecimalNumber(T f) {
  if (std::isnan(f)) {
    return Undefined();
  }
  if (std::isinf(f)) {
    return Infinity(f < 0.0);
  }
  return Decimal(f);
}

Number Number::FloatNumber(double d) {
  if (std::isnan(d)) {
    return Undefined();
  } else if (std::isinf(d)) {
    return Infinity(d < 0.0);
  } else {
    return Float<double>(d);
  }
}

Number Number::BinaryOperation(const Number & i, const Number & j, IntegerBinaryOperation integerOp, RationalBinaryOperation rationalOp, DoubleBinaryOperation doubleOp) {
  if (i.node()->type() == ExpressionNode::Type::Integer && j.node()->type() == ExpressionNode::Type::Integer) {
  // Integer + Integer
    Integer k = integerOp(static_cast<const Integer&>(i), static_cast<const Integer&>(j));
    if (!k.isInfinity()) {
      return k;
    }
  } else if (i.node()->type() == ExpressionNode::Type::Integer && j.node()->type() == ExpressionNode::Type::Rational) {
  // Integer + Rational
    Rational r = rationalOp(Rational(static_cast<const Integer&>(i)), static_cast<const Rational&>(j));
    if (!r.numeratorOrDenominatorIsInfinity()) {
      return r;
    }
  } else if (i.node()->type() == ExpressionNode::Type::Rational && j.node()->type() == ExpressionNode::Type::Integer) {
  // Rational + Integer
    return Number::BinaryOperation(j, i, integerOp, rationalOp, doubleOp);
  } else if (i.node()->type() == ExpressionNode::Type::Rational && j.node()->type() == ExpressionNode::Type::Rational) {
  // Rational + Rational
    Rational a = rationalOp(Rational(static_cast<const Rational&>(i)), Rational(static_cast<const Rational&>(j)));
    if (!a.numeratorOrDenominatorIsInfinity()) {
      return a;
    }
  }
  // one of the operand is Undefined/Infinity/Float or the Integer/Rational addition overflowed
  double a = doubleOp(i.node()->doubleApproximation(), j.node()->doubleApproximation());
  return FloatNumber(a);
}

Number Number::Addition(const Number & i, const Number & j) {
  return BinaryOperation(i, j, Integer::Addition, Rational::Addition, [](double a, double b) { return a+b; });
}

Number Number::Multiplication(const Number & i, const Number & j) {
  return BinaryOperation(i, j, Integer::Multiplication, Rational::Multiplication, [](double a, double b) { return a*b; });
}

Number Number::Power(const Number & i, const Number & j) {
  return BinaryOperation(i, j, Integer::Power,
      // Special case for Rational^Rational: we escape to Float if the index is not an Integer
      [](const Rational & i, const Rational & j) {
        if (!j.integerDenominator().isOne()) {
          // We return an overflown result to reach the escape case Float+Float
          return Rational(Integer::Overflow());
        }
        return Rational::IntegerPower(i, j.signedIntegerNumerator());
      },
      [](double a, double b) {
        return std::pow(a, b);
      }
    );
}

int Number::NaturalOrder(const Number & i, const Number & j) {
  if (i.node()->type() == ExpressionNode::Type::Integer && j.node()->type() == ExpressionNode::Type::Integer) {
  // Integer + Integer
    return Integer::NaturalOrder(static_cast<const Integer&>(i), static_cast<const Integer&>(j));
  } else if (i.node()->type() == ExpressionNode::Type::Integer && j.node()->type() == ExpressionNode::Type::Rational) {
  // Integer + Rational
    return Rational::NaturalOrder(Rational(static_cast<const Integer&>(i)), static_cast<const Rational&>(j));
  } else if (i.node()->type() == ExpressionNode::Type::Rational && j.node()->type() == ExpressionNode::Type::Integer) {
  // Rational + Integer
    return -Number::NaturalOrder(j, i);
  } else if (i.node()->type() == ExpressionNode::Type::Rational && j.node()->type() == ExpressionNode::Type::Rational) {
  // Rational + Rational
    return Rational::NaturalOrder(static_cast<const Rational&>(i), static_cast<const Rational&>(j));
  }
  // one of the operand is Undefined/Infinity/Float or the Integer/Rational addition overflowed
  if (i.node()->doubleApproximation() < j.node()->doubleApproximation()) {
    return -1;
  } else if (i.node()->doubleApproximation() == j.node()->doubleApproximation()) {
    return 0;
  } else {
    assert(i.node()->doubleApproximation() > j.node()->doubleApproximation());
    return 1;
  }
}

template Number Number::DecimalNumber<float>(float);
template Number Number::DecimalNumber<double>(double);
}