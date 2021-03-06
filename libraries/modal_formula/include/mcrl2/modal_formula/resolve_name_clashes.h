// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://github.com/mCRL2org/mCRL2/blob/master/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/modal_formula/detail/state_formula_name_clash_resolver.h
/// \brief add your file description here.

#ifndef MCRL2_MODAL_FORMULA_RESOLVE_NAME_CLASHES_H
#define MCRL2_MODAL_FORMULA_RESOLVE_NAME_CLASHES_H

#include "mcrl2/core/detail/print_utility.h"
#include "mcrl2/modal_formula/builder.h"
#include "mcrl2/modal_formula/state_formula.h"
#include "mcrl2/utilities/number_postfix_generator.h"
#include <set>

namespace mcrl2
{

namespace state_formulas
{

namespace detail
{

template <typename Derived>
class state_formula_name_clash_resolver: public state_formulas::state_formula_builder<Derived>
{
  public:
    typedef state_formulas::state_formula_builder<Derived> super;

    using super::enter;
    using super::leave;
    using super::apply;

    typedef std::map<core::identifier_string, std::vector<core::identifier_string> > name_map;

    /// \brief The stack of names.
    name_map m_names;

    /// \brief Generator for fresh variable names.
    utilities::number_postfix_generator m_generator;

    /// \brief Pops the name of the stack
    void pop(const core::identifier_string& name)
    {
      m_names[name].pop_back();
    }

    /// \brief Pushes name on the stack.
    void push(const core::identifier_string& name)
    {
      std::vector<core::identifier_string>& names = m_names[name];
      if (names.empty())
      {
        names.push_back(name);
      }
      else
      {
        names.push_back(m_generator(std::string(name) + "_"));
      }
    }

    void enter(const mu& x)
    {
      push(x.name());
    }

    void leave(const mu& x)
    {
      pop(x.name());
    }

    void enter(const nu& x)
    {
      push(x.name());
    }

    void leave(const nu& x)
    {
      pop(x.name());
    }

    // Rename variable
    state_formula apply(const mu& x)
    {
      enter(x);
      // N.B. If the two lines below are replace by
      //   state_formula result = mu(m_names[x.name()].back(), x.assignments(), (*this)(x.operand()));
      // a memory error occurs with the clang and intel compilers!
      core::identifier_string name = m_names[x.name()].back();
      state_formula result = mu(name, x.assignments(), apply(x.operand()));
      leave(x);
      return result;
    }

    // Rename variable
    state_formula apply(const nu& x)
    {
      enter(x);
      // N.B. If the two lines below are replace by
      //   state_formula result = nu(m_names[x.name()].back(), x.assignments(), (*this)(x.operand()));
      // a memory error occurs with the clang and intel compilers!
      core::identifier_string name = m_names[x.name()].back();
      state_formula result = nu(name, x.assignments(), apply(x.operand()));
      leave(x);
      return result;
    }

    // Rename variable
    state_formula operator()(const variable& x)
    {
      return variable(m_names[x.name()].back(), x.arguments());
    }
};

} // namespace detail

/// \brief Resolves name clashes in state formula f
inline
state_formula resolve_name_clashes(const state_formula& x)
{
  return core::make_apply_builder<detail::state_formula_name_clash_resolver>().apply(x);
}

} // namespace state_formulas

} // namespace mcrl2

#endif // MCRL2_MODAL_FORMULA_RESOLVE_NAME_CLASHES_H
