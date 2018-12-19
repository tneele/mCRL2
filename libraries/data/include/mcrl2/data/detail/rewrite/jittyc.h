// Author(s): Muck van Weerdenburg
// Copyright: see the accompanying file COPYING or copy at
// https://github.com/mCRL2org/mCRL2/blob/master/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/data/detail/rewrite/jittyc.h

#ifndef __REWR_JITTYC_H
#define __REWR_JITTYC_H

#include "mcrl2/atermpp/indexed_set.h"
#include "mcrl2/data/detail/rewrite.h"
#include "mcrl2/data/data_specification.h"
#include "mcrl2/data/detail/rewrite/jitty.h"
#include "mcrl2/data/detail/rewrite/match_tree.h"
#include "mcrl2/utilities/uncompiledlibrary.h"
#include "mcrl2/utilities/toolset_version.h"
#include "mcrl2/data/detail/rewrite/nfs_array.h"

#ifdef MCRL2_JITTYC_AVAILABLE

#include <utility>
#include <string>

namespace mcrl2
{
namespace data
{
namespace detail
{

typedef std::vector < sort_expression_list> sort_list_vector;

///
/// \brief The normal_form_cache class stores normal forms of data_expressions that
///        are inserted in it. By keeping the cache on the stack, the normal forms
///        in it will not be freed by the ATerm library, and can therefore be used
///        in the generated jittyc code.
///
class normal_form_cache
{
  private:
    RewriterJitty& m_rewriter;
    atermpp::indexed_set<data_expression> m_nf_lookup;
    atermpp::indexed_set<data_expression> m_fs_lookup;

  public:
    normal_form_cache(RewriterJitty& rewriter)
      : m_rewriter(rewriter)
    {
    }

  ///
  /// \brief insert stores the normal form of t in the cache, and returns a string
  ///        that is a C++ representation of the stored normal form. This string can
  ///        be used by the generated rewriter as long as the cache object is alive,
  ///        and its clear() method has not been called.
  /// \param t The term to normalize.
  /// \return A C++ string that evaluates to the cached normal form of t.
  ///
  std::string insert_normal_form(const data_expression& t)
  {
    RewriterJitty::substitution_type sigma;
    std::size_t expression_index = m_nf_lookup.put(m_rewriter(t,sigma)).first;
    std::stringstream ss;
    ss << "*reinterpret_cast<const data_expression*>(normal_form_address[" << expression_index << "])";
    return ss.str();
  }

  std::string insert_function_symbol(const function_symbol& f)
  {
    std::stringstream ss;
    ss << "function_symbol_address[" << m_fs_lookup.put(f).first << "]";
    return ss.str();
  }

  std::vector<uintptr_t> get_normal_form_addresses()
  {
    std::vector<uintptr_t> result;
    result.reserve(normal_form_size());
    for(std::size_t i = 0; i < normal_form_size(); i++)
    {
      // std::cout << "normal_forms[" << i << "] = " << m_nf_lookup.get(i) << " (" << reinterpret_cast<const void*>(&m_nf_lookup.get(i)) << ")" << std::endl;
      result.push_back(reinterpret_cast<uintptr_t>(&m_nf_lookup.get(i)));
    }
    return result;
  }

  std::vector<uintptr_t> get_function_symbol_addresses()
  {
    std::vector<uintptr_t> result;
    result.reserve(function_symbol_size());
    for(std::size_t i = 0; i < function_symbol_size(); i++)
    {
      // std::cout << "function_symbols[" << i << "] = " << m_fs_lookup.get(i) << " (" << reinterpret_cast<const void*>(atermpp::detail::address(m_fs_lookup.get(i))) << ")" << std::endl;
      result.push_back(reinterpret_cast<uintptr_t>(atermpp::detail::address(m_fs_lookup.get(i))));
    }
    return result;
  }

  std::size_t normal_form_size()
  {
    return m_nf_lookup.size();
  }

  std::size_t function_symbol_size()
  {
    return m_fs_lookup.size();
  }

  ///
  /// \brief clear clears the cache. This operation invalidates all the C++ strings
  ///        obtained via the insert() method.
  ///
  void clear()
  {
    m_nf_lookup.clear();
    m_fs_lookup.clear();
  }
};

class RewriterCompilingJitty: public Rewriter
{
  public:
    typedef Rewriter::substitution_type substitution_type;
    typedef data_expression (*rewriter_function)(const application&, RewriterCompilingJitty*);

    RewriterCompilingJitty(const data_specification& DataSpec, const used_data_equation_selector &);
    virtual ~RewriterCompilingJitty();

    rewrite_strategy getStrategy();

    data_expression rewrite(const data_expression& term, substitution_type& sigma);

    // The variable global_sigma is a temporary store to maintain the substitution
    // sigma during rewriting a single term. It is not a variable for public use.
    substitution_type *global_sigma;
    // The data structures below are used to store the variable lists2
    // that are used in the compiling rewriter in forall, where and exists.
    std::vector<variable_list> rewriter_binding_variable_lists;
    std::map <variable_list, std::size_t> variable_list_indices1;
    std::size_t binding_variable_list_index(const variable_list& v);
    inline variable_list binding_variable_list_get(const std::size_t i)
    {
      return (rewriter_binding_variable_lists[i]);
    }

    // The data structures below are used to store single variables
    // that are bound in lambda, forall and exist operators. When required
    // in the compiled required, these variables can be retrieved from
    // the array rewriter_bound_variables. variable_indices0 is used
    // to prevent double occurrences in the vector.
    std::vector<variable> rewriter_bound_variables;
    std::map <variable, std::size_t> variable_indices0;

    // The following values are used to locate rewrite functions in the tables of
    // precompiled functions.
    //   arity_bound -- The maximum occurring arity + 1
    //   index_bound -- The maximum occurring index + 1
    std::size_t arity_bound;
    std::size_t index_bound;

    std::size_t bound_variable_index(const variable& v);
    variable bound_variable_get(const std::size_t i)
    {
      return (rewriter_bound_variables[i]);
    }

    // The two arrays below are intended to contain the precompiled functions used
    // for rewriting. They are used to find the relevant compiled rewriting code quickly.
    std::vector<rewriter_function> functions_when_arguments_are_not_in_normal_form;
    std::vector<rewriter_function> functions_when_arguments_are_in_normal_form;

    // Standard assignment operator.
    RewriterCompilingJitty& operator=(const RewriterCompilingJitty& other)=delete;

  private:
    class ImplementTree;
    friend class ImplementTree;

    RewriterJitty jitty_rewriter;
    std::set < data_equation > rewrite_rules;
    bool made_files;
    std::map<function_symbol, data_equation_list> jittyc_eqns;
    std::set<function_symbol> m_extra_symbols;

    std::shared_ptr<uncompiled_library> rewriter_so;
    normal_form_cache m_nf_cache;

    void (*so_rewr_cleanup)();
    data_expression(*so_rewr)(const data_expression&, RewriterCompilingJitty*);

    void add_base_nfs(nfs_array& a, const function_symbol& opid, std::size_t arity);
    void extend_nfs(nfs_array& a, const function_symbol& opid, std::size_t arity);
    bool opid_is_nf(const function_symbol& opid, std::size_t num_args);
    void calc_nfs_list(nfs_array& a, const application& args, variable_or_number_list nnfvars);
    bool calc_nfs(const data_expression& t, variable_or_number_list nnfvars);
    void CleanupRewriteSystem();
    void BuildRewriteSystem();
    void generate_code(const std::string& filename);
    void generate_rewr_functions(std::ostream& s, const data::function_symbol& func, const data_equation_list& eqs);
    bool lift_rewrite_rule_to_right_arity(data_equation& e, const std::size_t requested_arity);
    sort_list_vector get_residual_sorts(const sort_expression& s, const std::size_t actual_arity, const std::size_t requested_arity);
    match_tree_list create_strategy(const data_equation_list& rules, const std::size_t arity);

};

struct rewriter_interface
{
  std::string caller_toolset_version;
  std::string status;
  RewriterCompilingJitty* rewriter;
  data_expression (*rewrite_external)(const data_expression& t, RewriterCompilingJitty*);
  void (*rewrite_cleanup)();
};

}
}
}

#endif // MCRL2_JITTYC_AVAILABLE

#endif // __REWR_JITTYC_H
