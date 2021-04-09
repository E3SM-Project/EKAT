#ifndef EKAT_STD_ANY_HPP
#define EKAT_STD_ANY_HPP

/*
 *  Emulating c++1z features
 *
 *  This file contains an implementation of the 'any' concept, which is implemented
 *  in the std library in c++17.
 *  Since we have limited access to c++1z standard on target platforms,
 *  we can only rely on c++11 features. So we try to emulate std::any here.
 */

#include "ekat/ekat_assert.hpp"
#include "ekat/std_meta/ekat_std_utils.hpp"
#include "ekat/ekat_type_traits.hpp"

#include <memory>
#include <iostream>
#include <typeindex>

namespace ekat {

// ================ std::any ================= //

class any {

  // Implementation detail of the any class
  class holder_base {
  public:
    virtual ~holder_base() = default;

    virtual const std::type_info& type () const = 0;

    virtual void print (std::ostream& os) const = 0;
  };

  template <typename HeldType>
  class holder : public holder_base {
  public:

    template<typename... Args>
    static holder<HeldType>* create(const Args&... args) {
      holder* ptr = new holder<HeldType>();
      ptr->m_value = std::make_shared<HeldType>(args...);
      return ptr;
    }

    template<typename T>
    static
    typename std::enable_if<std::is_base_of<HeldType,T>::value,holder<HeldType>*>::type
    create(std::shared_ptr<T> ptr_in)
    {
      holder* ptr = new holder<HeldType>();
      ptr->m_value = ptr_in;
      return ptr;
    }

    const std::type_info& type () const { return typeid(HeldType); }

    HeldType& value () { return *m_value; }
    std::shared_ptr<HeldType> ptr () const { return m_value; }

    void print (std::ostream& os) const {
      if (static_cast<bool>(m_value)) {
        print_impl<HeldType>(os);
      }
    }
  private:
    holder () = default;

    template<typename T>
    typename std::enable_if<!StreamExists<T>::value>::type
    print_impl (std::ostream& os) const {

      os << "Error! Trying to print object of type '" << type().name() << "',"
         << "       which does not overload operator<< .\n";
    }

    template<typename T>
    typename std::enable_if<StreamExists<T>::value>::type
    print_impl (std::ostream& os) const {
      os << *m_value;
    }

    // Note: we store a shared_ptr rather than a HeldType directly,
    //       since we may store multiple copies of the concrete object.
    //       Since we don't know if the actual object is copiable, we need
    //       to store copiable pointers (hence, cannot use unique_ptr).
    std::shared_ptr<HeldType> m_value;
  };

public:

  any () = default;

  template<typename T, typename... Args>
  void reset (Args... args) {
    m_content.reset( holder<T>::create(args...) );
  }

  holder_base& content () const { 
    EKAT_REQUIRE_MSG (static_cast<bool>(m_content), "Error! Object not yet initialized.\n");
    return *m_content;
  }

  holder_base* content_ptr () const { 
    return m_content.get();
  }

  template<typename ConcreteType>
  bool isType () const {
    return std::type_index(content().type())==std::type_index(typeid(ConcreteType));
  }

  template<typename ConcreteType>
  friend ConcreteType& any_cast (any&);

  template<typename ConcreteType>
  friend const ConcreteType& any_cast (const any&);

  template<typename ConcreteType>
  friend std::shared_ptr<ConcreteType> any_ptr_cast (any&);
  
private:

  std::shared_ptr<holder_base> m_content;
};

template<typename ConcreteType>
ConcreteType& any_cast (any& src) {

  EKAT_REQUIRE_MSG(src.isType<ConcreteType>(),
      "Error! Invalid cast requested.\n"
      "   - actual type:    " + std::string(src.content().type().name()) + "\n"
      "   - requested type: " + std::string(typeid(ConcreteType).name()) + "'.\n");

  any::holder<ConcreteType>* ptr = dynamic_cast<any::holder<ConcreteType>*>(src.content_ptr());

  EKAT_REQUIRE_MSG(ptr!=nullptr,
      "Error! Failed dynamic_cast during any_cast.\n"
      "       This is an internal problem, please, contact developers.\n");

  return ptr->value();
}

template<typename ConcreteType>
const ConcreteType& any_cast (const any& src) {
  EKAT_REQUIRE_MSG(src.isType<ConcreteType>(),
      "Error! Invalid cast requested.\n"
      "   - actual type:    " + std::string(src.content().type().name()) + "\n"
      "   - requested type: " + std::string(typeid(ConcreteType).name()) + "'.\n");

  any::holder<ConcreteType>* ptr = dynamic_cast<any::holder<ConcreteType>*>(src.content_ptr());

  EKAT_REQUIRE_MSG(ptr!=nullptr,
      "Error! Failed dynamic_cast during any_cast.\n"
      "       This is an internal problem, please, contact developers.\n");

  return ptr->value();
}

template<typename ConcreteType>
std::shared_ptr<ConcreteType> any_ptr_cast (any& src) {
  EKAT_REQUIRE_MSG(src.isType<ConcreteType>(),
      "Error! Invalid cast requested.\n"
      "   - actual type:    " + std::string(src.content().type().name()) + "\n"
      "   - requested type: " + std::string(typeid(ConcreteType).name()) + "'.\n");

  any::holder<ConcreteType>* ptr = dynamic_cast<any::holder<ConcreteType>*>(src.content_ptr());

  EKAT_REQUIRE_MSG(ptr!=nullptr,
      "Error! Failed dynamic_cast during any_cast.\n"
      "       This is an internal problem, please, contact developers.\n");

  return ptr->ptr();
}

// Overload stream operator
inline std::ostream& operator<< (std::ostream& out, const any& any) {
  any.content().print(out);

  return out;
}

} // namespace ekat

#endif // EKAT_STD_ANY_HPP
