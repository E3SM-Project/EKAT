#include "ekat_parameter_list.hpp"
#include "ekat_std_utils.hpp"

#include <ios>

namespace ekat {

ParameterList::
ParameterList (const ParameterList& src)
{
  m_name = src.m_name;
  for (const auto& [name,p] : src.m_params) {
    m_params[name] = p;
  }

  for (const auto& [name,sl] : src.m_sublists) {
    m_sublists[name] = sl;
  }
}

ParameterList& ParameterList::operator= (const ParameterList& src)
{
  if (this!=&src) {
    m_name = src.m_name;
    for (const auto& [name,p] : src.m_params) {
      m_params[name] = p;
    }

    for (const auto& [name,sl] : src.m_sublists) {
      m_sublists[name] = sl;
    }
  }
  return *this;
}

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
  for (const auto& [name,p] : m_params) {
    out << tab << name << ": ";

    // Try a few common types
         if (std::any_cast<int>(&p))          out << std::any_cast<int>(p);
    else if (std::any_cast<float>(&p))        out << std::showpoint << std::any_cast<float>(p);
    else if (std::any_cast<double>(&p))       out << std::showpoint << std::any_cast<double>(p);
    else if (std::any_cast<bool>(&p))         out << std::boolalpha << std::any_cast<bool>(p);
    else if (std::any_cast<std::string>(&p))  out << std::any_cast<std::string>(p);
    else if (std::any_cast<std::vector<int>>(&p))         out << std::any_cast<std::vector<int>>(p);
    else if (std::any_cast<std::vector<float>>(&p))       out << std::showpoint << std::any_cast<std::vector<float>>(p);
    else if (std::any_cast<std::vector<double>>(&p))      out << std::showpoint << std::any_cast<std::vector<double>>(p);
    else if (std::any_cast<std::vector<bool>>(&p))        out << std::boolalpha << std::any_cast<std::vector<bool>>(p);
    else if (std::any_cast<std::vector<std::string>>(&p)) out << std::any_cast<std::vector<std::string>>(p);
    else out << " (Cannot print type '" << p.type().name() << "')";

    out << std::endl;
  }

  for (const auto& [name,sl] : m_sublists) {
    sl.print(out,indent+indent_inc,indent_inc);
  }
}

void ParameterList::import (const ParameterList& src)
{
  for (const auto& [name,sl] : src.m_sublists) {
    m_sublists[name] = sl;
  }
  for (const auto& [name,p] : src.m_params) {
    m_params[name] = p;
  }
}

std::vector<std::string>
ParameterList::param_names () const
{
  std::vector<std::string> names;
  for (const auto& it : m_params) {
    names.push_back(it.first);
  }
  return names;
}

std::vector<std::string>
ParameterList::sublist_names () const
{
  std::vector<std::string> names;
  for (const auto& it : m_sublists) {
    names.push_back(it.first);
  }
  return names;
}

} // namespace ekat
