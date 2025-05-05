#ifndef EKAT_MAP_KEY_ITERATOR_HPP
#define EKAT_MAP_KEY_ITERATOR_HPP

#include <iterator>
#include <map>

namespace ekat {

/*
 * An iterator that allows to iterate over the keys of a map only.
 *
 * The reason to have this class is that we can expose a way to
 * access the keys of a map without exposing the actual content
 * of the pair. This is very handy for map<K,std::shared_ptr<V>>,
 * where even if the map is const, iterating over it allows one
 * to modify the pointees.
 * By offering a key-iterator, one can iterate over the keys of
 * the map, without exposing the values in any way.
 * Note: this is similar to python's dictionary 'keys()' method.
 */

template<typename MapType>
struct map_key_iterator;
template<typename MapType>
struct map_key_const_iterator;

template<typename Key, typename Value>
struct map_key_iterator<std::map<Key,Value>> final
{
public:
  using iterator          = map_key_iterator<std::map<Key,Value>>;
  using iterator_category = std::bidirectional_iterator_tag;
  using difference_type   = std::ptrdiff_t;
  using value_type        = Key;
  using pointer           = value_type*;
  using reference         = value_type&;

  using map_iterator = typename std::map<Key,Value>::iterator;

  map_key_iterator ( map_iterator it )
    : m_iter (it)
  {
    // nothing to do here
  }

  pointer    operator -> ( ) { return & (m_iter->first); }
  reference  operator *  ( ) { return    m_iter->first;  }

  iterator& operator++()   { m_iter++; return *this;}
  iterator operator++(int) { auto retval = *this; m_iter++; return retval; }
  iterator& operator--()   { m_iter--; return *this;}
  iterator operator--(int) { auto retval = *this; m_iter--; return retval; }

  bool operator==(iterator other) const {return m_iter == other.m_iter;}
  bool operator!=(iterator other) const {return m_iter != other.m_iter;}
private:
  map_iterator m_iter;
};

template<typename Key, typename Value>
struct map_key_const_iterator<std::map<Key,Value>> final
{
public:
  using iterator          = map_key_const_iterator<std::map<Key,Value>>;
  using iterator_category = std::bidirectional_iterator_tag;
  using difference_type   = std::ptrdiff_t;
  using value_type        = Key;
  using pointer           = const value_type*;
  using reference         = const value_type&;

  using map_iterator = typename std::map<Key,Value>::const_iterator;

  map_key_const_iterator ( map_iterator it )
    : m_iter (it)
  {
    // nothing to do here
  }

  pointer    operator -> ( ) { return & (m_iter->first); }
  reference  operator *  ( ) { return    m_iter->first;  }

  iterator& operator++()   { m_iter++; return *this;}
  iterator operator++(int) { auto retval = *this; m_iter++; return retval; }
  iterator& operator--()   { m_iter--; return *this;}
  iterator operator--(int) { auto retval = *this; m_iter--; return retval; }

  bool operator==(iterator other) const {return m_iter == other.m_iter;}
  bool operator!=(iterator other) const {return m_iter != other.m_iter;}
private:
  map_iterator m_iter;
};

} // namespace ekat

#endif // EKAT_MAP_KEY_ITERATOR_HPP
