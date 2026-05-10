// matrix_debug.cpp — implementations of the inspection helpers.

#include "matrix_debug.hpp"

#include "matrix.hpp"

#include <cstddef>
#include <sstream>
#include <string>

// File-scope using — same rationale as matrix.cpp.
using std::size_t;

std::string dump_raw(const Matrix& m) {
  std::ostringstream os;
  os << "Matrix(rows=" << m.rows() << ", cols=" << m.cols() << ", data=[";
  const auto& d = m.data();
  for (size_t k = 0; k < d.size(); ++k) {
    if (k > 0) os << ", ";
    os << d[k];
  }
  os << "])";
  return os.str();
}

size_t memory_bytes(const Matrix& m) {
  return sizeof(Matrix) + m.data().capacity() * sizeof(double);
}
