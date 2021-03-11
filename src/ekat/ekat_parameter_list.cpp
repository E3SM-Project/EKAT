#include "ekat/ekat_parameter_list.hpp"

namespace ekat {

ParameterList& ParameterList::sublist (const std::string& name) {
  if (m_sublists.find(name)==m_sublists.end()) {
    ParameterList p(name);
    m_sublists[name] = p;
  }
  return m_sublists[name];
}

void ParameterList::print(std::ostream& out, const int indent) const {
  std::string tab;
  for (int i=0; i<indent; ++i) {
    tab += " ";
  }

  out << tab << name() << ":\n";
  tab += " ";
  for (auto it : m_params) {
    out << tab << it.first << ": " << it.second << "\n";
  }
  for (auto it : m_sublists) {
    it.second.print(out,indent+1);
  }
}

void ParameterList::import (const ParameterList& src) {
  for (const auto& it : src.m_sublists) {
    m_sublists[it.first] = it.second;
  }
  for (const auto& it : src.m_params) {
    m_params[it.first] = it.second;
  }
}

} // namespace ekat
