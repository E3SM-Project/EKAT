#ifndef EKAT_PARAMETER_LIST_HPP
#define EKAT_PARAMETER_LIST_HPP

#include "ekat/std_meta/ekat_std_any.hpp"
#include "ekat/std_meta/ekat_std_map_key_iterator.hpp"
#include "ekat_assert.hpp"

#include <map>

namespace ekat {

/*
 * A class to store list of arbitrary parameters (possibly recursively)
 *
 * A parameter list store two things: parameters, and sublists.
 * Each of these is stored in a map, which uses a string as key.
 *
 * Parameters are stored using ekat::any, which allows to store pretty
 * much anything you want in the list. However, this means that when
 * you try to retrieve a parameter, you must already know what the
 * type is, or else the any_cast will throw. You can specify the type
 * via a template argument to the 'get' method. Otherwise, when dealing
 * with a non-const ParameterList, you can simply pass a default value
 * to the 'get' method, which will be used to add the parameter to the
 * list if the given name is not found. If the name is found, the type
 * of the default value will be used to perform the any_cast. Again,
 * if the type does not match the type of what is already stored,
 * an exception will be thrown.
 */

class ParameterList {
public:

  // Constructor(s) & Destructor
  ParameterList () = default;
  explicit ParameterList (const std::string& name) : m_name(name) {}
  ~ParameterList () = default;

  ParameterList& operator= (const ParameterList&) = default;

  // The name of the list
  const std::string& name () const { return m_name; }

  // Parameters getters and setters
  template<typename T>
  T& get (const std::string& name);

  template<typename T>
  const T& get (const std::string& name) const;

  template<typename T>
  T& get (const std::string& name, const T& def_value);

  template<typename T>
  void set (const std::string& name, const T& value);

  // Sublist getters
  ParameterList& sublist (const std::string& name);

  const ParameterList& sublist (const std::string& name) const { return m_sublists.at(name); }

  // Check methods, to verify a parameter/sublist is present
  bool isParameter (const std::string& name) const { return m_params.find(name)!=m_params.end(); }
  bool isSublist   (const std::string& name) const { return m_sublists.find(name)!=m_sublists.end(); }

  // Check methods, to determine the type of a node
  template<typename T>
  bool isType (const std::string& name) const;

  // Display the sublist.
  // NOTE: this *requires* op<< to be overloaded for all the stored parameters.
  //       The code won't crash otherwise, but instead of the parameter, you
  //       will get a message informing you of the lack of op<< overload.
  void print (std::ostream& out = std::cout, const int indent = 0) const;

  // Add content of src into *this. Existing items will be overwritten.
  void import (const ParameterList& src);

  // Access const iterators to stored data
  using params_names_const_iter   = map_key_const_iterator<std::map<std::string,any>>;
  using sublists_names_const_iter = map_key_const_iterator<std::map<std::string,ParameterList>>;

  params_names_const_iter   params_names_cbegin ()   const { return params_names_const_iter(m_params.cbegin()); }
  params_names_const_iter   params_names_cend   ()   const { return params_names_const_iter(m_params.cend());   }

  sublists_names_const_iter sublists_names_cbegin () const { return sublists_names_const_iter(m_sublists.cbegin()); }
  sublists_names_const_iter sublists_names_cend   () const { return sublists_names_const_iter(m_sublists.cend());   }

private:

  std::string                           m_name;
  std::map<std::string,any>             m_params;
  std::map<std::string,ParameterList>   m_sublists;
};

// ====================== IMPLEMENTATION ===================== //

template<typename T>
inline T& ParameterList::get (const std::string& name) {
  // Check entry exists
  EKAT_REQUIRE_MSG ( isParameter(name),
      "Error! Key '" + name + "' not found in parameter list '" + m_name + "'.\n");

  return any_cast<T>(m_params[name]);
}

template<typename T>
inline const T& ParameterList::get (const std::string& name) const {
  EKAT_REQUIRE_MSG ( isParameter(name),
      "Error! Key '" + name + "' not found in parameter list '" + m_name + "'.\n");

  return any_cast<T>(m_params.at(name));
}

template<typename T>
inline T& ParameterList::get (const std::string& name, const T& def_value) {
  if ( !isParameter(name) ) {
    m_params[name].template reset<T>(def_value);
  }
  return get<T>(name);
}

template<typename T>
inline void ParameterList::set (const std::string& name, const T& value) {
  if ( !isParameter(name) ) {
    m_params[name].template reset<T>(value);
  } else {
    get<T>(name) = value;
  }
}

template<typename T>
inline bool ParameterList::isType (const std::string& name) const {
  // Check entry exists
  EKAT_REQUIRE_MSG ( isParameter(name),
      "Error! Key '" + name + "' not found in parameter list '" + m_name + "'.\n");

  return m_params.at(name).isType<T>();
}

} // namespace ekat

#endif // EKAT_PARAMETER_LIST_HPP
