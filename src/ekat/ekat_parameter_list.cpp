#include "ekat/ekat_parameter_list.hpp"

#include <ios>

namespace ekat {

ParameterList& ParameterList::sublist (const std::string& name) {
  if (m_sublists.find(name)==m_sublists.end()) {
    ParameterList p(name);
    m_sublists[name] = p;
  }
  return m_sublists[name];
}

void ParameterList::print(std::ostream& out, const int indent, const int indent_inc) const {
  std::string tab(indent,' ');

  out << tab << name() << ":\n";
  tab.append(indent_inc,' ');
  for (auto it : m_params) {
    out << std::showpoint << tab << it.first << ": " << it.second << "\n";
  }
  for (auto it : m_sublists) {
    it.second.print(out,indent+indent_inc,indent_inc);
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
