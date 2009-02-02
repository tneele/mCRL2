// Author(s): Jeroen Keiren
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/new_data/lambda.h
/// \brief The class lambda.

#ifndef MCRL2_NEW_DATA_LAMBDA_H
#define MCRL2_NEW_DATA_LAMBDA_H

#include "mcrl2/atermpp/aterm_appl.h"
#include "mcrl2/atermpp/aterm_list.h"
#include "mcrl2/atermpp/aterm_traits.h"
#include "mcrl2/atermpp/vector.h"
#include "mcrl2/core/detail/constructors.h"
#include "mcrl2/new_data/abstraction.h"
#include "mcrl2/new_data/variable.h"
#include "mcrl2/new_data/application.h"

namespace mcrl2 {
  
  namespace new_data {

    /// \brief function symbol.
    ///
    class lambda: public abstraction
    {
      protected:
       variable_list m_variables; ///< The list of variables that is abstracted.

      public:

        /// Constructor.
        ///
        /// \param[in] d A new_data expression.
        /// \pre d is a lambda abstraction.
        lambda(const data_expression& d)
          : abstraction(d),
            m_variables(atermpp::term_list<data_expression>(atermpp::list_arg2(d)).begin(), atermpp::term_list<data_expression>(atermpp::list_arg2(d)).end())
        {
          assert(d.is_abstraction());
          assert(static_cast<abstraction>(d).binding_operator() == "lambda");
        }

        /// Constructor.
        ///
        /// \param[in] variables A nonempty list of binding variables.
        /// \param[in] body The body of the lambda abstraction.
        /// \pre variables is not empty.
        lambda(const boost::iterator_range<variable_list::const_iterator>& variables,
               const data_expression& body)
          : abstraction("lambda", variables, body),
            m_variables(variables.begin(), variables.end())
        {
          assert(!variables.empty());
        }

        /// \brief Returns the application of this lambda abstraction to an argument.
        /// \pre this->sort() is a function sort.
        /// \param[in] e The new_data expression to which the function symbol is applied
        application operator()(const data_expression& e) const
        {
          assert(this->sort().is_function_sort());
          return application(*this, e);
        }

        /*  Should be enabled when the implementation in data_expression is
         * removed
        /// \overload
        inline
        sort_expression sort() const
        {
          return function_sort(sorts_of_data_expressions(boost::make_iterator_range(m_variables.begin(), m_variables.end())), body().sort());
        }
        */

    }; // class lambda

    /// \brief list of lambdas
    ///
    typedef atermpp::vector<lambda> lambda_list;

  } // namespace new_data

} // namespace mcrl2

/// \cond INTERNAL_DOCS
MCRL2_ATERM_TRAITS_SPECIALIZATION(mcrl2::new_data::lambda)
/// \endcond

#endif // MCRL2_NEW_DATA_LAMBDA_H

