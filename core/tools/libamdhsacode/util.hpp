/******************************************************************************
* University of Illinois / NCSA
* Open Source License
*
* Copyright(c) 2011 - 2015  Advanced Micro Devices, Inc.
* All rights reserved.
*
* Developed by:
* Advanced Micro Devices, Inc.
* www.amd.com
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files(the "Software"), to deal
* with the Software without restriction, including without limitation the
* rights to use, copy, modify, merge, publish, distribute, sublicense, and /
* or sell copies of the Software, and to permit persons to whom the Software
* is furnished to do so, subject to the following conditions:
*
*     Redistributions of source code must retain the above copyright notice,
* this list of conditions and the following disclaimers.
*
*     Redistributions in binary form must reproduce the above copyright notice,
* this list of conditions and the following disclaimers in the documentation
* and / or other materials provided with the distribution.
*
*     Neither the names of Advanced Micro Devices, Inc, nor the
mes of its
* contributors may be used to endorse or promote products derived from this
* Software without specific prior written permission.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
* CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS WITH
* THE SOFTWARE.
******************************************************************************/

#ifndef EXT_COMMON_UTIL_HPP
#define EXT_COMMON_UTIL_HPP

#include <cstdlib>
#include <iostream>
#include <list>
#include <vector>
#include <cstdint>

#include <cassert>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>

//===----------------------------------------------------------------------===//
// Error Handling.                                                            //
//===----------------------------------------------------------------------===//

#define hsa_error(e) static_cast<hsa_status_t>(e)

#define release_assert(e)                                                      \
  if (!(e)) {                                                                  \
    std::cerr << __FILE__ << ":";                                              \
    std::cerr << __LINE__ << ":";                                              \
    std::cerr << " Assertion `" << #e << "' failed." << std::endl;             \
    std::abort();                                                              \
  }                                                                            \

//===----------------------------------------------------------------------===//
// Memory Management.                                                         //
//===----------------------------------------------------------------------===//

#if defined(_WIN32) || defined(_WIN64)
#  define x_aligned_alloc(l, a) _mm_malloc(l, a)
#  define x_aligned_free(p) _mm_free(p)
#else
inline void *x_aligned_alloc(size_t l, size_t a) {
  void *ptr = 0;
  if (posix_memalign(&ptr, a, l) != 0) { return 0; }
  return ptr;
}
#  define x_aligned_free(p) free(p)
#endif

namespace ext {
namespace common {

//===----------------------------------------------------------------------===//
// ElfMemoryImage.                                                            //
//===----------------------------------------------------------------------===//

class ElfMemoryImage final {
public:
  static bool Is32(const void *emi);

  static bool Is64(const void *emi);

  static bool IsBigEndian(const void *emi);

  static bool IsLittleEndian(const void *emi);

  static bool IsValid(const void *emi);

  static uint64_t Size(const void *emi);

private:
  template<typename ElfN_Ehdr, typename ElfN_Shdr>
  static uint64_t Size(const void *emi);
};

//===----------------------------------------------------------------------===//
// StringFactory.                                                             //
//===----------------------------------------------------------------------===//

class StringFactory final {
public:
  static std::string Flatten(const char **cstrs,
                             const uint32_t &cstrs_count,
                             const char &spacer = '\0');

  static std::string Format(const char *cstr, va_list arg);

  static std::list<std::string> Tokenize(const char *cstr, const char &delim);

  static std::string ToLower(const std::string& str);
  static std::string ToUpper(const std::string& str);
};

//===----------------------------------------------------------------------===//
// HelpPrinter, HelpStreambuf.                                                //
//===----------------------------------------------------------------------===//

class HelpStreambuf : public std::streambuf {
public:
  explicit HelpStreambuf(std::ostream& stream);

  virtual ~HelpStreambuf() {
    basicStream_->rdbuf(basicBuf_);
  }

  void IndentSize(unsigned indent) {
    assert(wrapWidth_ == 0 || indentSize_ < wrapWidth_);
    indentSize_ = indent;
  }

  void WrapWidth(unsigned wrap) {
    assert(wrapWidth_ == 0 || indentSize_ < wrapWidth_);
    wrapWidth_ = wrap;
  }

protected:
  virtual int_type overflow(int_type ch) override;

private:
  std::ostream* basicStream_;
  std::streambuf* basicBuf_;

  unsigned wrapWidth_;
  unsigned indentSize_;

  bool atLineStart_;
  unsigned lineWidth_;
};


class HelpPrinter {
private:
  static const unsigned USAGE_WIDTH = 30;
  static const unsigned PADDING_WIDTH = 2;
  static const unsigned DESCRIPTION_WIDTH = 50;

public:
  HelpPrinter& PrintUsage(const std::string& usage);
  HelpPrinter& PrintDescription(const std::string& description);

  std::ostream& Stream() { return *out_; }

private:
  explicit HelpPrinter(std::ostream& out = std::cout) : out_(&out), sbuf_(*out_) {}

  /// @brief Not copy-constructible.
  HelpPrinter(const HelpPrinter&);
  /// @brief Not copy-assignable.
  HelpPrinter& operator =(const HelpPrinter&);

  friend class OptionParser;

  std::ostream *out_;
  HelpStreambuf sbuf_;
};

//===----------------------------------------------------------------------===//
// OptionBase.                                                                //
//===----------------------------------------------------------------------===//

class OptionBase {
public:
  virtual ~OptionBase() {}

  const std::string& name() const {
    return name_;
  }
  const bool& is_set() const {
    return is_set_;
  }

  bool IsValid() const {
    return 0 < name_.size();
  }

protected:
  OptionBase(const std::string& name,
             const std::string& help = "",
             std::ostream &error = std::cerr)
    : name_(name),
      help_(help),
      is_set_(false),
      error_(&error) {}

  virtual void PrintHelp(HelpPrinter& printer) const = 0;

  const std::string name_;
  const std::string help_;
  bool is_set_;

  std::ostream &error() const { return *error_; }

private:
  /// @brief Not copy-constructible.
  OptionBase(const OptionBase &ob);
  /// @brief Not copy-assignable.
  OptionBase& operator=(const OptionBase &ob);

  void Reset() {
    is_set_ = false;
  }

  virtual bool ProcessTokens(std::list<std::string> &tokens) = 0;

  friend class OptionParser;

  mutable std::ostream *error_;
};


//===----------------------------------------------------------------------===//
// Option<T>.                                                                 //
//===----------------------------------------------------------------------===//

template<typename T>
class Option final: public OptionBase {
public:
  Option(const std::string& name,
         const std::string& help = "",
         std::ostream& error = std::cerr):
    OptionBase(name, help, error) {}

  ~Option() {}

  const std::list<T>& values() const {
    return values_;
  }

protected:
  virtual void PrintHelp(HelpPrinter& printer) const override;

private:
  /// @brief Not copy-constructible.
  Option(const Option &o);
  /// @brief Not copy-assignable.
  Option& operator=(const Option &o);

  bool ProcessTokens(std::list<std::string> &tokens);

  std::list<T> values_;
};

template<typename T>
bool Option<T>::ProcessTokens(std::list<std::string> &tokens) {
  assert(0 == name_.compare(tokens.front()) && "option name is mismatched");
  if (2 > tokens.size()) {
    error() << "error: invalid option: \'" << name_ << '\'' << std::endl;
    return false;
  }

  is_set_ = true;
  tokens.pop_front();

  while (!tokens.empty()) {
    std::istringstream token_stream(tokens.front());
    if (!token_stream.good()) {
      error() << "error: invalid option: \'" << name_ << '\'' << std::endl;
      return false;
    }

    T value;
    token_stream >> value;

    values_.push_back(value);
    tokens.pop_front();
  }
  return true;
}

template<typename T>
void Option<T>::PrintHelp(HelpPrinter& printer) const {
  printer.PrintUsage("-" + name_ + " [" + StringFactory::ToUpper(name_) + "s]")
         .PrintDescription(help_);
}

//===----------------------------------------------------------------------===//
// ValueOption<T>.                                                            //
//===----------------------------------------------------------------------===//

template<typename T>
class ValueOption final: public OptionBase {
public:
  ValueOption(const std::string& name,
               const std::string& help = "",
               std::ostream& error = std::cerr):
    OptionBase(name, help, error) {}

  ~ValueOption() {}

  const T& value() const {
    return value_;
  }

protected:
  void PrintHelp(HelpPrinter& printer) const override;

private:
  /// @brief Not copy-constructible.
  ValueOption(const ValueOption &o);
  /// @brief Not copy-assignable.
  ValueOption& operator=(const ValueOption &o);

  bool ProcessTokens(std::list<std::string> &tokens);

  T value_;
};

template<typename T>
bool ValueOption<T>::ProcessTokens(std::list<std::string> &tokens) {
  assert(0 == name_.compare(tokens.front()) && "option name is mismatched");
  if (2 != tokens.size()) {
    error() << "error: invalid option: \'" << name_ << '\'' << std::endl;
    return false;
  }

  is_set_ = true;
  tokens.pop_front();

  std::istringstream token_stream(tokens.front());
  if (!token_stream.good()) {
    error() << "error: invalid option: \'" << name_ << '\'' << std::endl;
    return false;
  }
  token_stream >> value_;
  tokens.pop_front();
  return true;
}

template<typename T>
void ValueOption<T>::PrintHelp(HelpPrinter& printer) const {
  printer.PrintUsage("-" + name_ + "=[VAL]")
         .PrintDescription(help_);
}

//===----------------------------------------------------------------------===//
// ChoiceOptioin.                                                             //
//===----------------------------------------------------------------------===//
class ChoiceOption final: public OptionBase {
public:
  ChoiceOption(const std::string& name,
               const std::vector<std::string>& choices,
               const std::string& help = "",
               std::ostream& error = std::cerr);

  ~ChoiceOption() {}

  const std::string& value() const {
    return value_;
  }

protected:
  void PrintHelp(HelpPrinter& printer) const override;

private:
  /// @brief Not copy-constructible.
  ChoiceOption(const ChoiceOption&);
  /// @brief Not copy-assignable.
  ChoiceOption& operator =(const ChoiceOption&);

  bool ProcessTokens(std::list<std::string> &tokens);

  std::unordered_set<std::string> choices_;
  std::string value_;
};

//===----------------------------------------------------------------------===//
// Option<void>.                                                              //
//===----------------------------------------------------------------------===//

class NoArgOption final: public OptionBase {
public:
  NoArgOption(const std::string& name,
              const std::string& help = "",
              std::ostream& error = std::cerr):
    OptionBase(name, help, error) {}

  ~NoArgOption() {}

protected:
  void PrintHelp(HelpPrinter& printer) const override {
    printer.PrintUsage("-" + name_).PrintDescription(help_);
  }

private:
  /// @brief Not copy-constructible.
  NoArgOption(const NoArgOption &o);
  /// @brief Not copy-assignable.
  NoArgOption& operator=(const NoArgOption &o);

  bool ProcessTokens(std::list<std::string> &tokens) {
    assert(0 == name_.compare(tokens.front()) && "option name is mismatched");
    if (1 != tokens.size()) {
      error() << "error: invalid option: \'" << name_ << '\'' << std::endl;
      return false;
    }

    is_set_ = true;
    tokens.pop_front();
    return true;
  }
};

//===----------------------------------------------------------------------===//
// OptionParser.                                                              //
//===----------------------------------------------------------------------===//

class OptionParser final {
public:
  OptionParser(bool collectUnknown = false, std::ostream& error = std::cerr)
    : collectUnknown_(collectUnknown),
      error_(&error) {}

  ~OptionParser() {}

  bool AddOption(OptionBase *option);

  bool ParseOptions(const char *options);

  const std::string& Unknown() const;
  void CollectUnknown(bool b) { collectUnknown_ = b; }

  void PrintHelp(std::ostream& out, const std::string& addition = "") const;

private:
  /// @brief Not copy-constructible.
  OptionParser(const OptionParser &op);
  /// @brief Not copy-assignable.
  OptionParser& operator=(const OptionParser &op);

  void Reset();

  std::ostream& error() { return *error_; }

  std::vector<OptionBase*>::iterator FindOption(const std::string& name);

  std::vector<OptionBase*> options_;

  std::string unknownOptions_;
  bool collectUnknown_;

  std::ostream *error_;
};

inline uint32_t alignUp(uint32_t num, uint32_t align)
{
  assert(align);
  assert((align & (align - 1)) == 0);
  return (num + align - 1) & ~(align - 1);
}

} // namespace common
} // namespace ext

#endif // EXT_COMMON_UTIL_HPP
