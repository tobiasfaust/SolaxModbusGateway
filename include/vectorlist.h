#ifndef VECTORLIST_H
#define VECTORLIST_H

/*
  vectorlist.h
  Extended template container that stores pairs of (value, identifier) for arbitrary types.

  API (T = value type, I = identifier type):
    - setOffset(uint8_t off)               // sets a runtime offset that is applied to all numeric values in getArray* calls
    - uint8_t getOffset() const            // returns the currently set offset
    - bool addValue(const T& value, const I& id)
    - bool deleteValue(const T& value)                  // first match across all identifiers
    - bool deleteValue(const T& value, const I& id)     // match with specific identifier
    - bool isInList(const T& value) const
    - bool isInList(const T& value, const I& id) const
    - bool deleteAll()                // clears all entries
    - bool deleteAll(const I& id)     // clears only entries having this identifier
    - int  size() const               // total entries
    - int  size(const I& id) const    // entries with specific identifier
    - String getArray() const                     // all values
    - String getArrayForIdentifier(const I& id) const            // only values with matching identifier
    - String getArrayExcludeIdentifier(const I& id) const        // all except those with identifier
    - String getArray(const std::vector<T>& excludeValues) const // exclude certain values (global)
    - String getArray(std::initializer_list<T> excludeValues) const

  Zusätzlich gewünschte Semantik:
    * Wird bei Prüf-/Löschfunktionen kein Identifier angegeben wird global gesucht.
    * Duplicate Policy: (value,id) Kombination muss eindeutig sein. Gleiches value mit anderem id ist erlaubt.
    * bei Angabe eines Offsets wird dieser bei allen getArray* Aufrufen zu allen numerischen Werten addiert

  String Handling:
    If T is Arduino String the element is quoted and internal double quotes are escaped so JS can parse it.
*/

#include <Arduino.h>
#include <vector>
#include <type_traits>
#include <algorithm>

// Helper formatter: generic version
template<typename U, typename Enable = void>
struct VectorListValueFormatter {
  static String toString(const U& v) { return String(v); }
};

// Specialization for Arduino String
template<typename U>
struct VectorListValueFormatter<U, typename std::enable_if<std::is_same<U, String>::value>::type> {
  static String toString(const String& v) {
    String esc = v; // escape double quotes
    esc.replace("\"", "\\\"");
    return String("\"") + esc + String("\"");
  }
};

template<typename T, typename I>
class vectorlist {
public:
  struct Entry { T value; I id; };

  // Set a runtime offset that is applied to values for all getArray* calls.
  // The offset is only added for numeric types (integral except bool, or floating point types).
  void setOffset(uint8_t off) { _offset = off; }
  uint8_t getOffset() const { return _offset; }

  // Insert (value,id) if unique; duplicates allowed across different id values.
  bool addValue(const T& value, const I& id) {
    if (isInList(value, id)) { return false; }
    _data.push_back({value, id});
    return true;
  }

  // Bulk insert: add multiple values with the same identifier.
  // Returns true if at least one new (value,id) pair was inserted.
  bool addValues(const std::vector<T>& values, const I& id) {
    bool anyInserted = false;
    if (!values.empty()) { _data.reserve(_data.size() + values.size()); }
    for (const auto& v : values) {
      if (!isInList(v, id)) { _data.push_back({v, id}); anyInserted = true; }
    }
    return anyInserted;
  }

  // Convenience overload for initializer_list
  bool addValues(std::initializer_list<T> values, const I& id) {
    return addValues(std::vector<T>(values), id);
  }

  // Delete first occurrence of value (across all identifiers)
  bool deleteValue(const T& value) {
    for (auto it = _data.begin(); it != _data.end(); ++it) {
      if (it->value == value) { _data.erase(it); return true; }
    }
    return false;
  }

  // Delete specific (value,id)
  bool deleteValue(const T& value, const I& id) {
    for (auto it = _data.begin(); it != _data.end(); ++it) {
      if (it->value == value && it->id == id) { _data.erase(it); return true; }
    }
    return false;
  }

  // Clear everything
  bool deleteAll() { _data.clear(); return true; }

  // Clear only entries with identifier
  bool deleteAll(const I& id) {
    bool removed = false;
    for (auto it = _data.begin(); it != _data.end();) {
      if (it->id == id) { it = _data.erase(it); removed = true; } else { ++it; }
    }
    return removed;
  }

  bool isInList(const T& value) const {
    for (const auto& e : _data) { if (e.value == value) return true; }
    return false;
  }
  bool isInList(const T& value, const I& id) const {
    for (const auto& e : _data) { if (e.value == value && e.id == id) return true; }
    return false;
  }

  int size() const { return static_cast<int>(_data.size()); }
  int size(const I& id) const {
    int c=0; for (const auto& e : _data) if (e.id == id) c++; return c; }

  // Build JS array from ALL values
  String getArray() const { return buildArray(nullptr, nullptr, nullptr); }

  // Only values with given identifier
  String getArrayForIdentifier(const I& id) const { return buildArray(&id, nullptr, nullptr); }

  // All values except those with given identifier
  String getArrayExcludeIdentifier(const I& id) const { return buildArray(nullptr, &id, nullptr); }

  // Exclude given value list globally
  String getArray(const std::vector<T>& excludeValues) const { return buildArray(nullptr, nullptr, &excludeValues); }

  String getArray(std::initializer_list<T> excludeValues) const {
    std::vector<T> tmp(excludeValues);
    return getArray(tmp);
  }

  // Raw value vectors (without identifiers)
  std::vector<T> raw() const { std::vector<T> v; v.reserve(_data.size()); for (auto& e : _data) v.push_back(e.value); return v; }
  std::vector<T> raw(const I& id) const { std::vector<T> v; for (auto& e : _data) if (e.id==id) v.push_back(e.value); return v; }

  // Direct access to entries if really needed
  const std::vector<Entry>& rawEntries() const { return _data; }

private:
  std::vector<Entry> _data;
  uint8_t _offset = 0; // runtime offset applied to numeric values in array output

  // Helper: determine at compile time if T supports addition with uint8_t safely (numeric, not bool)
  template<typename U>
  struct CanAddOffset : std::integral_constant<bool,
    ( (std::is_integral<U>::value && !std::is_same<U,bool>::value) || std::is_floating_point<U>::value )> {};

  // Apply offset only if allowed
  template<typename U>
  static typename std::enable_if<CanAddOffset<U>::value, U>::type applyOffset(const U& v, uint8_t off) {
    return static_cast<U>(v + static_cast<U>(off));
  }
  template<typename U>
  static typename std::enable_if<!CanAddOffset<U>::value, U>::type applyOffset(const U& v, uint8_t) {
    return v; // no change
  }

  // Internal builder with optional filters:
  //   onlyId != nullptr           -> include only entries with *onlyId
  //   excludeId != nullptr        -> exclude entries with *excludeId
  //   excludeValues != nullptr    -> exclude entries whose value is in *excludeValues
  String buildArray(const I* onlyId, const I* excludeId, const std::vector<T>* excludeValues) const {
    String out("[");
    bool first = true;
    for (const auto& e : _data) {
      if (onlyId && e.id != *onlyId) continue;
      if (excludeId && e.id == *excludeId) continue;
      if (excludeValues && std::find(excludeValues->begin(), excludeValues->end(), e.value) != excludeValues->end()) continue;
      if (!first) out += ","; else first = false;
  // Apply offset for eligible numeric types before formatting
  auto adjusted = applyOffset<T>(e.value, _offset);
  out += VectorListValueFormatter<T>::toString(adjusted);
    }
    out += "]";
    return out;
  }

};

#endif // VECTORLIST_H
