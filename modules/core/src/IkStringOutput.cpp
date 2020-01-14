#include "IkStringOutput.h"
#include "IkStringEncoding.h"

using namespace iknow::core;

IkStringOutput& IkStringOutput::operator<<(const std::string& s) {
  return *this << iknow::base::IkStringEncoding::UTF8ToBase(s);;
}
