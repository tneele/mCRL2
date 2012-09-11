// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/pbes/detail/symbolic_exploration.h
/// \brief add your file description here.

#ifndef MCRL2_PBES_DETAIL_SYMBOLIC_EXPLORATION_H
#define MCRL2_PBES_DETAIL_SYMBOLIC_EXPLORATION_H

#include "mcrl2/utilities/exception.h"
#include "mcrl2/pbes/find.h"
#include "mcrl2/pbes/io.h"
#include "mcrl2/pbes/pbes_expression.h"
#include "mcrl2/pbes/pbes_functions.h"

namespace mcrl2 {

namespace pbes_system {

namespace detail {

class symbolic_exploration_algorithm
{
  protected:
    /// \brief Concatenates two variable lists
    /// \param x A variable list
    /// \param y A variable list
    /// \return The concatenation of x and y
    data::variable_list concat(const data::variable_list& x, const data::variable_list& y)
    {
      std::vector<data::variable> v(x.begin(), x.end());
      v.insert(v.end(), y.begin(), y.end());
      return data::variable_list(v.begin(), v.end());
    }

    bool is_conjunctive(const pbes_expression& x) const
    {
      return is_forall(x) || is_and(x) || is_propositional_variable_instantiation(x) || is_simple_expression(x);
    }

    pbes_expression expr_or(const pbes_expression& x)
    {
      namespace z = pbes_expr_optimized;

      // N.B. The case statement below is order dependent!
      if (is_forall(x))
      {
        forall y = x;
        return z::forall(y.variables(), expr_or(y.body()));
      }
      else if (is_exists(x))
      {
        exists y = x;
        return z::exists(y.variables(), expr_or(y.body()));
      }
      else if (is_propositional_variable_instantiation(x))
      {
        return false_();
      }
      else if (is_or(x))
      {
        or_ y = x;
        return z::or_(expr_or(y.left()), expr_or(y.right()));
      }
      else if (is_simple_expression(x))
      {
        return x;
      }
      else if (is_and(x))
      {
        return false_();
      }
      throw mcrl2::runtime_error("unknown pbes expression encountered in expr_or: " + pbes_system::pp(x));
      return pbes_expression();
    }

    pbes_expression expr_and(const pbes_expression& x)
    {
      namespace z = pbes_expr_optimized;

      // N.B. The case statement below is order dependent!
      if (is_forall(x))
      {
        forall y = x;
        return z::forall(y.variables(), expr_and(y.body()));
      }
      else if (is_exists(x))
      {
        exists y = x;
        return exists(y.variables(), expr_and(y.body()));
      }
      else if (is_propositional_variable_instantiation(x))
      {
        return true_();
      }
      else if (is_and(x))
      {
        and_ y = x;
        return z::and_(expr_and(y.left()), expr_and(y.right()));
      }
      else if (is_simple_expression(x))
      {
        return x;
      }
      else if (is_or(x))
      {
        return true_();
      }
      throw mcrl2::runtime_error("unknown pbes expression encountered in expr_and: " + pbes_system::pp(x));
      return pbes_expression();
    }

    pbes_expression F_or(const pbes_expression& x)
    {
      namespace z = pbes_expr_optimized;

      // N.B. The case statement below is order dependent!
      if (is_simple_expression(x))
      {
        return false_();
      }
      else if (is_propositional_variable_instantiation(x))
      {
        return x;
      }
      else if (is_and(x))
      {
        return z::and_(expr_and(x), F_and(x));
      }
      else if (is_or(x))
      {
        pbes_expression phi = or_(x).left();
        pbes_expression psi = or_(x).right();
        return z::or_(F_or(phi), F_or(psi));
      }
      else if (is_forall(x))
      {
        return z::and_(expr_and(x), F_and(x));
      }
      else if (is_exists(x))
      {
        data::variable_list d = exists(x).variables();
        pbes_expression phi = exists(x).body();
        return z::and_(z::exists(d, expr_and(phi)), F(phi));
      }
      throw mcrl2::runtime_error("unknown pbes expression encountered in F_or: " + pbes_system::pp(x));
      return pbes_expression();
    }

    pbes_expression F_and(const pbes_expression& x)
    {
      namespace z = pbes_expr_optimized;

      // N.B. The case statement below is order dependent!
      if (is_simple_expression(x))
      {
        return true_();
      }
      else if (is_propositional_variable_instantiation(x))
      {
        return x;
      }
      else if (is_and(x))
      {
        pbes_expression phi = and_(x).left();
        pbes_expression psi = and_(x).right();
        return z::and_(F_and(phi), F_and(psi));
      }
      else if (is_or(x))
      {
        return z::imp(z::not_(expr_or(x)), F_or(x));
      }
      else if (is_exists(x))
      {
        return z::imp(z::not_(expr_or(x)), F_or(x));
      }
      else if (is_forall(x))
      {
        data::variable_list d = exists(x).variables();
        pbes_expression phi = exists(x).body();
        return z::forall(d, z::imp(z::not_(expr_or(phi)), F(phi)));
      }
      throw mcrl2::runtime_error("unknown pbes expression encountered in F_and: " + pbes_system::pp(x));
      return pbes_expression();
    }

    pbes_expression F(const pbes_expression& x)
    {
      if (is_conjunctive(x))
      {
        return F_and(x);
      }
      else
      {
        return F_or(x);
      }
    }

  public:

    void run(pbes<>& p)
    {
      namespace z = pbes_expr_optimized;

      atermpp::vector<pbes_equation>& equations = p.equations();
      for (atermpp::vector<pbes_equation>::iterator i = equations.begin(); i != equations.end(); ++i)
      {
        pbes_expression phi = i->formula();
        if (is_conjunctive(phi))
        {
          i->formula() = z::and_(expr_and(phi), F_and(phi));
        }
        else
        {
          i->formula() = z::imp(z::not_(expr_or(phi)), F_or(phi));
        }
      }
    }
};

inline
void symbolic_exploration(const std::string& input_filename,
                          const std::string& output_filename
                         )
{
  // load the pbes
  pbes<> p;
  load_pbes(p, input_filename);

  // apply the algorithm
  symbolic_exploration_algorithm algorithm;
  algorithm.run(p);

  // save the result
  p.save(output_filename);
}

} // namespace detail

} // namespace pbes_system

} // namespace mcrl2

#endif // MCRL2_PBES_DETAIL_SYMBOLIC_EXPLORATION_H
