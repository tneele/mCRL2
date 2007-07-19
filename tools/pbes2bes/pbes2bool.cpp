// Author(s): Jan Friso Groote
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file pbes2bool.cpp
/// \brief Add your file description here.

// ======================================================================
//
// file          : pbes2bool
// date          : 15-04-2007
// version       : 0.1.3
//
// author(s)     : Alexander van Dam <avandam@damdonk.nl>
//                 Jan Friso Groote <J.F.Groote@tue.nl>
//
// ======================================================================


#define NAME "pbes2bool"
#define VERSION "0.1.3"
#define AUTHOR "Alexander van Dam and Jan Friso Groote"


//C++
#include <ostream>
#include <exception>
#include <iostream>
#include <fstream>
#include <string>
#include <utility>

#include <sstream>

//Boost
#include <boost/program_options.hpp>

//MCRL-specific
#include "print/messaging.h"

//LPS-Framework
#include "mcrl2/pbes/pbes.h"
#include "mcrl2/pbes/utility.h"
#include "mcrl2/data/data_operators.h"
#include "mcrl2/data/sort.h"

//ATERM-specific
#include "atermpp/substitute.h"
#include "atermpp/utility.h"
#include "atermpp/indexed_set.h"
#include "atermpp/table.h"

//Tool-specific
#include "pbes_rewrite_jfg.h"
#include "sort_functions.h"
#include "bes.h"

using namespace std;
using namespace lps;
using namespace mcrl2::utilities;
using bes::bes_expression;

using atermpp::make_substitution;

namespace po = boost::program_options;

//Type definitions
//----------------

//typedef struct {
  //data_variable_list finite_var;    // List of all finite variables
  //data_variable_list infinite_var;  // List of all infinite variables
  //data_expression_list finite_exp;  // List of all finite expressions
  //data_expression_list infinite_exp;  // List of all infinite expressions
//
  //void protect()
  //{
    //finite_var.protect();
    //infinite_var.protect();
    //finite_exp.protect();
    //infinite_exp.protect();
  //}
  //
  //void unprotect()
  //{
    //finite_var.unprotect();
    //infinite_var.unprotect();
    //finite_exp.unprotect();
    //infinite_exp.unprotect();
  //}
  //void mark()
  //{
    //finite_var.mark();
    //infinite_var.mark();
    //finite_exp.mark();
    //infinite_exp.mark();
  //}
//} t_instantiations;


// Specify how the ATerms in t_instantiations need to be protected using a traits class
//namespace atermpp
//{
  //template<>
  //struct aterm_traits<t_instantiations>
  //{
    //static void protect(t_instantiations t) { t.protect(); }
    //static void unprotect(t_instantiations t) { t.unprotect(); }
    //static void mark(t_instantiations t) { t.mark(); }
  //};
// } // namespace atermpp

//Function declarations used by main program
//------------------------------------------
static t_tool_options parse_command_line(int argc, char** argv);
//Post: The command line options are parsed.
//      The program has aborted with a suitable error code, if:
//    - errors were encounterd
//    - non-standard behaviour was requested (help, version)
//Ret:  The parsed command line options

static void calculate_bes(pbes pbes_spec, t_tool_options tool_options);
//Post: tool_options.infilename contains a PBES ("-" indicates stdin)
//Ret:  The BES generated from the PBES

pbes load_pbes(t_tool_options tool_options);
//Post: tool_options.infilename contains a PBES ("-" indicates stdin)
//Ret: The pbes loaded from infile

static void save_bes_in_cwi_format(string outfilename);
static void save_bes_in_vasy_format(string outfilename);
//Post: pbes_spec is saved in cwi-format
//Ret: -

static void save_rhs_in_cwi_form(ostream &outputfile, bes_expression p);
static void save_rhs_in_vasy_form(ostream &outputfile, 
                                     bes_expression p,
                                     vector<unsigned long> &variable_index,
                                     const unsigned long current_rank);
// Function used to convert a pbes_expression to the variant used by the cwi-output

static void do_lazy_algorithm(pbes pbes_spec, t_tool_options tool_options);

static bool solve_bes();

// Create a propositional variable instantiation with the checks needed in the naive algorithm

static bool process(t_tool_options const& tool_options) 
{
  //Load PBES
  pbes pbes_spec = load_pbes(tool_options);

  //Process the pbes
  calculate_bes(pbes_spec, tool_options);

  if (tool_options.opt_outputformat == "cwi")
  { // in CWI format only if the result is a BES, otherwise Binary
    save_bes_in_cwi_format(tool_options.outfilename);
  }
  else if (tool_options.opt_outputformat == "vasy")
  { //Save resulting bes if necessary.
    save_bes_in_vasy_format(tool_options.outfilename);
  }
  else 
  { 
    gsMessage("The pbes is %s valid\n", solve_bes() ? "" : "not");
  }

  return true;
}

// SQuADT protocol interface
#ifdef ENABLE_SQUADT_CONNECTIVITY
#include <mcrl2/utilities/squadt_interface.h>

class squadt_interactor : public mcrl2::utilities::squadt::tool_interface {

  private:

    static const char*  pbes_file_for_input;  ///< file containing an LPS
    static const char*  bes_file_for_output; ///< file used to write the output to

    enum bes_output_format {
      none,
      vasy,
      cwi
    };

    enum transformation_strategy {
      lazy
    };

    static const char* option_transformation_strategy;
    static const char* option_selected_output_format;

  private:

    boost::shared_ptr < tipi::datatype::enumeration > transformation_method_enumeration;
    boost::shared_ptr < tipi::datatype::enumeration > output_format_enumeration;

  public:

    /** \brief constructor */
    squadt_interactor();

    /** \brief configures tool capabilities */
    void set_capabilities(tipi::tool::capabilities&) const;

    /** \brief queries the user via SQuADT if needed to obtain configuration information */
    void user_interactive_configuration(tipi::configuration&);

    /** \brief check an existing configuration object to see if it is usable */
    bool check_configuration(tipi::configuration const&) const;

    /** \brief performs the task specified by a configuration */
    bool perform_task(tipi::configuration&);
};

const char* squadt_interactor::pbes_file_for_input  = "pbes_in";
const char* squadt_interactor::bes_file_for_output = "bes_out";

const char* squadt_interactor::option_transformation_strategy = "transformation_strategy";
const char* squadt_interactor::option_selected_output_format  = "selected_output_format";

squadt_interactor::squadt_interactor() {
  transformation_method_enumeration.reset(new tipi::datatype::enumeration("lazy"));

  output_format_enumeration.reset(new tipi::datatype::enumeration("none"));

  output_format_enumeration->add_value("vasy");
  output_format_enumeration->add_value("cwi");
}

void squadt_interactor::set_capabilities(tipi::tool::capabilities& c) const {
  c.add_input_combination(pbes_file_for_input, tipi::mime_type("pbes", tipi::mime_type::application), tipi::tool::category::transformation);
}

void squadt_interactor::user_interactive_configuration(tipi::configuration& c) {
  using namespace tipi;
  using namespace tipi::layout;
  using namespace tipi::layout::elements;

  layout::tool_display::sptr display(new layout::tool_display);

  /* Create and add the top layout manager */
  layout::vertical_box::aptr top(new layout::vertical_box);

  /* First column */
  layout::manager* h = new layout::horizontal_box();

  h->add(new label("Output format : "));
  
  mcrl2::utilities::squadt::radio_button_helper < bes_output_format >
        format_selector(h, none, "none");

  format_selector.associate(h, vasy, "vasy");
  format_selector.associate(h, cwi, "cwi");

  if (c.option_exists(option_selected_output_format)) {
    format_selector.set_selection(static_cast < bes_output_format > (
        c.get_option_argument< size_t >(option_selected_output_format, 0)));
  }
  
  /* Attach row */
  top->add(h, margins(0,5,0,5));

  top->add(new label("Transformation strategy :"));

  mcrl2::utilities::squadt::radio_button_helper < transformation_strategy >
        transformation_selector(top, lazy, "lazy: only boolean equations reachable from the initial state");

  if (c.option_exists(option_transformation_strategy)) {
    transformation_selector.set_selection(static_cast < transformation_strategy > (
        c.get_option_argument< size_t >(option_transformation_strategy, 0)));
  }
  
  button* okay_button = new button("OK");

  top->add(okay_button, layout::top);

  display->set_manager(top);

  m_communicator.send_display_layout(display);

  /* Wait until the ok button was pressed */
  okay_button->await_change();

  c.add_option(option_transformation_strategy).append_argument(transformation_method_enumeration,
                                static_cast < transformation_strategy > (transformation_selector.get_selection()));
  c.add_option(option_selected_output_format).append_argument(output_format_enumeration,
                                static_cast < bes_output_format > (format_selector.get_selection()));

  if (c.get_option_argument< size_t >(option_selected_output_format)!=none)
  {
    /* Add output file to the configuration */
    if (c.output_exists(bes_file_for_output)) {
      tipi::object& output_file = c.get_output(bes_file_for_output);
   
      output_file.set_location(c.get_output_name(".txt"));
    }
    else {
      c.add_output(bes_file_for_output, tipi::mime_type("txt", tipi::mime_type::application), 
                   c.get_output_name(".txt"));
    }
  }

  send_clear_display();
}

bool squadt_interactor::check_configuration(tipi::configuration const& c) const {
  bool result = true;

  result &= c.input_exists(pbes_file_for_input);
  result &= c.option_exists(option_transformation_strategy);
  result &= c.option_exists(option_selected_output_format);
  if (result && (c.get_option_argument< size_t >(option_selected_output_format)!=none))
  { /* only check for the existence of an outputfile if the output format does
       not equal none */
    result &= c.output_exists(bes_file_for_output);
  }

  return (result);
}

bool squadt_interactor::perform_task(tipi::configuration& c) {
  static std::string strategies[] = { "lazy" };
  static std::string formats[]    = { "none", "vasy", "cwi" };

  t_tool_options tool_options;

  tool_options.opt_outputformat = formats[c.get_option_argument< size_t >(option_selected_output_format)];
  tool_options.opt_strategy     = strategies[c.get_option_argument< size_t >(option_transformation_strategy)];
  tool_options.infilename       = c.get_input(pbes_file_for_input).get_location();

  if (c.output_exists(bes_file_for_output)) {
    tool_options.outfilename      = c.get_output(bes_file_for_output).get_location();
  }

  send_clear_display();

  bool result = process(tool_options);
 
  return (result);
}
#endif


//Main Program
//------------
int main(int argc, char** argv)
{
  //Initialise ATerm library and lowlevel-functions
  
  ATerm bottom;
  ATinit(argc, argv, &bottom);
  gsEnableConstructorFunctions();

#ifdef ENABLE_SQUADT_CONNECTIVITY
  if (mcrl2::utilities::squadt::interactor< squadt_interactor >::free_activation(argc, argv)) {
    return 0;
  }
#endif

  return process(parse_command_line(argc, argv));
}

//function calculate_bes
//-------------------
void calculate_bes(pbes pbes_spec, t_tool_options tool_options)
{
  if (!pbes_spec.is_well_formed())
  {
    gsErrorMsg("The PBES is not well formed. Pbes2bes cannot handle this kind of PBES's\nComputation aborted.\n");
    exit(1);
  }
  if (!pbes_spec.is_closed())
  {
    gsErrorMsg("The PBES is not closed. Pbes2bes cannot handle this kind of PBES's\nComputation aborted.\n");
    exit(1);
  }

  do_lazy_algorithm(pbes_spec, tool_options);
  //return new pbes
  return;
}

//function add_propositional_variable_instantiations_to_indexed_set
//and translate to pbes expression to a bes_expression in BDD format.

static bes::bes_expression add_propositional_variable_instantiations_to_indexed_set_and_translate(
                   const lps::pbes_expression p,
                   atermpp::indexed_set &variable_index,
                   unsigned long &nr_of_generated_variables,
                   const bool to_bdd)
{ // std::cerr << " add_prop_var " << pp(p) << std::endl;
  if (is_propositional_variable_instantiation(p))
  { pair<unsigned long,bool> pr=variable_index.put(p);
    if (pr.second) /* add p to indexed set */
    { nr_of_generated_variables++;
    }
    if (to_bdd)
    { return bes::if_(bes::variable(pr.first),bes::true_(),bes::false_());
    }
    else 
    { return bes::variable(pr.first);
    }
  }
  else if (pbes_expr::is_and(p))
  { bes::bes_expression b1=add_propositional_variable_instantiations_to_indexed_set_and_translate(
                            pbes_expr::lhs(p),variable_index,nr_of_generated_variables,to_bdd);
    if (is_false(b1))
    { return b1;
    }
    bes::bes_expression b2=add_propositional_variable_instantiations_to_indexed_set_and_translate(
                            pbes_expr::rhs(p),variable_index,nr_of_generated_variables,to_bdd);
    if (is_false(b2))
    { return b2;
    }
    if (is_true(b1))
    { return b2;
    }
    if (is_true(b2))
    { return b1;
    }
    if (to_bdd)
    { return BDDif(b1,b2,bes::false_());
    }
    else
    { return and_(b1,b2);
    }
  }
  else if (pbes_expr::is_or(p))
  { 
    bes::bes_expression b1=add_propositional_variable_instantiations_to_indexed_set_and_translate(
                            pbes_expr::lhs(p),variable_index,nr_of_generated_variables,to_bdd);
    if (bes::is_true(b1))
    { return b1;
    }

    bes::bes_expression b2=add_propositional_variable_instantiations_to_indexed_set_and_translate(
                            pbes_expr::rhs(p),variable_index,nr_of_generated_variables,to_bdd);
    if (bes::is_true(b2))
    { return b2;
    }
    if (bes::is_false(b1))
    { return b2;
    }
    if (bes::is_false(b2))
    { return b1;
    }

    if (to_bdd)
    { return BDDif(b1,bes::true_(),b2);
    }
    else
    { return or_(b1,b2);
    }
  }
  else if (pbes_expr::is_true(p))
  { return bes::true_();
  }
  else if (pbes_expr::is_false(p))
  { return bes::false_();
  }
    
  cerr << "Unexpected expression. Most likely because expression fails to rewrite to true or false: " << pp(p) << "\n";
  assert(0);
  return bes::false_();
}

//function do_lazy_algorithm
//------------------------------

// Global variables
//  atermpp::indexed_set variable_index(10000, 50);     
  bes::equations bes_equations;

static void do_lazy_algorithm(pbes pbes_spec, t_tool_options tool_options)
{

  // Verbose msg: doing naive algorithm
  gsVerboseMsg("Computing BES from PBES using lazy algorithm...\n");
  
  data_specification data = pbes_spec.data();

  // Variables in which the result is stored
  propositional_variable_instantiation new_initial_state;
  equation_system new_equation_system;
  
  // Variables used in whole function
  unsigned long nr_of_processed_variables = 0;
  unsigned long nr_of_generated_variables = 1;

  atermpp::indexed_set variable_index(10000, 50);

  variable_index.put(bes::true_()); /* Put first a dummy term that
                                       gets index 0 in the indexed set, to
                                       prevent variables from getting an index 0 */

  // Data rewriter
  Rewriter *rewriter = createRewriter(data,tool_options.rewrite_strategy);
  variable_index.put(pbes_expression_rewrite_and_simplify(pbes_spec.initial_state(),rewriter,tool_options));

  // Needed hashtables
  equation_system eqsys = pbes_spec.equations();
  atermpp::table pbes_equations(2*eqsys.size(), 50);   // (propvarname, pbes_equation)

  // Vector with the order of the variable names used for sorting the result

  atermpp::table variable_rank(2*eqsys.size(),50);

  // Fill the pbes_equations table
  gsVerboseMsg("Retrieving pbes_equations from equation system...\n");

  assert(eqsys.size()>0); // There should be at least one equation
  fixpoint_symbol current_fixpoint_symbol=eqsys.begin()->symbol();

  unsigned long rank=1;

  for (equation_system::iterator eqi = eqsys.begin(); eqi != eqsys.end(); eqi++)
  { 
    pbes_equations.put(eqi->variable().name(), 
        pbes_equation(eqi->symbol(),eqi->variable(),
            pbes_expression_rewrite_and_simplify(eqi->formula(),rewriter,tool_options)));
    if (eqi->symbol()!=current_fixpoint_symbol)
    { current_fixpoint_symbol=eqi->symbol();
      rank=rank+1;
    }
    variable_rank.put(eqi->variable().name(),atermpp::aterm_int(rank));
  }

  gsVerboseMsg("Computing BES....\n");
  // As long as there are states to be explored
  while (nr_of_processed_variables < nr_of_generated_variables)
  {
    propositional_variable_instantiation current_variable_instantiation = 
          propositional_variable_instantiation(variable_index.get(nr_of_processed_variables+1));
    // std::cerr << " current expression " << pp(current_variable_instantiation) << std::endl;
    
    // Get equation which belongs to the current propvarinst and their needed parts
    pbes_equation current_pbeq = pbes_equation(pbes_equations.get(current_variable_instantiation.name()));
    propositional_variable current_variable = current_pbeq.variable();
    pbes_expression current_pbes_expression = current_pbeq.formula();

    // Add the required substitutions
    data_expression_list::iterator elist=current_variable_instantiation.parameters().begin();
    for(data_variable_list::iterator vlist=current_variable.parameters().begin() ;
           vlist!=current_variable.parameters().end() ; vlist++)
    { 
      assert(elist!=current_variable_instantiation.parameters().end());
      // std::cerr << "Substitution: " << *vlist << "  " << *elist << std::endl;
      if (tool_options.opt_precompile_pbes)
      { rewriter->setSubstitution(*vlist,(aterm)*elist);
      }
      else
      { rewriter->setSubstitution(*vlist,rewriter->toRewriteFormat(*elist));
      }
      elist++;
    }
    assert(elist==current_variable_instantiation.parameters().end());
    lps::pbes_expression new_pbes_expression = pbes_expression_substitute_and_rewrite(
                              current_pbes_expression, data, rewriter,tool_options);
    // std::cerr << " new pbes expression " << pp(new_pbes_expression) << std::endl;
    bes::bes_expression new_bes_expression=
         add_propositional_variable_instantiations_to_indexed_set_and_translate(
                      new_pbes_expression,variable_index,nr_of_generated_variables,
                      tool_options.opt_outputformat=="none");

    bes_equations.add_equation(
              nr_of_processed_variables+1,
              current_pbeq.symbol(),
              atermpp::aterm_int(variable_rank.get(current_pbeq.variable().name())).value(),
              new_bes_expression);

    nr_of_processed_variables++;
    if (nr_of_processed_variables % 100 == 0)
    { 
      gsVerboseMsg("Processed %d and generated %d boolean variables\n", 
                                 nr_of_processed_variables,nr_of_generated_variables);
    }
  }
}

/* substitute true or false in fixpoint equation */
static bes_expression substitute_fp(
                const bes_expression b,
                const bes::variable_type v)
{ if (is_if(b))
  {
    if (v==bes::get_variable(condition(b)))
    {
      /* first check whether v and cond refer to the same variable, as
       * cond can then be substituted by true (for nu) or false (for mu) */
      if (bes_equations.get_fixpoint_symbol(v)==fixpoint_symbol::mu())
      { return else_branch(b); // substitute_bex(v,w,else_branch(b));
      }
      else
      { return then_branch(b); // substitute_bex(v,w,then_branch(b));
      }
    }
    else 
    { /* the condition is not equal to v */
      bes_expression b1=substitute_fp(then_branch(b),v);
      bes_expression b2=substitute_fp(else_branch(b),v);
      if ((b1==then_branch(b)) && (b2==else_branch(b)))
      { return b;
      }
      bes_expression br=BDDif(bes::if_(condition(b),bes::true_(),bes::false_()),b1,b2);
      return br;
    }
  }
  assert(is_true(b)||is_false(b));
  return b;
}


static void add_variables_to_set(
                bes::variable_type v,
                bes_expression b,
                std::vector<std::set <bes::variable_type> > &variable_occurrence_set,
                const unsigned long current_rank)
{ 
  if (bes::is_true(b)||bes::is_false(b))
  { return;
  }
  assert(is_if(b));
  if (current_rank==bes_equations.get_rank(get_variable(condition(b))))
  { variable_occurrence_set[get_variable(condition(b))].insert(v);
  }
  // Using hash tables this can be made more efficient, by employing
  // sharing of the aterm representing b.
  add_variables_to_set(v,then_branch(b),variable_occurrence_set,current_rank);
  add_variables_to_set(v,else_branch(b),variable_occurrence_set,current_rank);
}


/* substitute boolean equation expression */
static bes_expression substitute_rank(
                const bes_expression b,
                const unsigned long current_rank,
                const atermpp::vector<bes_expression> &approximation)
{ /* substitute variables with rank larger and equal
     than current_rank with their approximations. */
     
  if (is_if(b))
  { 
    bes::variable_type v=bes::get_variable(condition(b));
    if (bes_equations.get_rank(v)==current_rank)
    {
      if (bes::is_true(approximation[v]))
      { return substitute_rank(then_branch(b),current_rank,approximation);
      }
      if (bes::is_false(approximation[v]))
      { return substitute_rank(else_branch(b),current_rank,approximation);
      }

      bes_expression b1=substitute_rank(then_branch(b),current_rank,approximation);
      bes_expression b2=substitute_rank(else_branch(b),current_rank,approximation);
      return BDDif(approximation[v],b1,b2);
    }
    else
    { /* the condition is not equal to v */
      bes_expression b1=substitute_rank(then_branch(b),current_rank,approximation);
      bes_expression b2=substitute_rank(else_branch(b),current_rank,approximation);
      if ((b1==then_branch(b)) && (b2==else_branch(b)))
      { return b;
      }
      bes_expression br=BDDif(bes::if_(condition(b),bes::true_(),bes::false_()),b1,b2);
      return br;
    }
  }

  assert(bes::is_true(b)||bes::is_false(b));
  return b;
}

/* substitute boolean equation expression */
static bes_expression evaluate_bex(
                const bes_expression b,
                const atermpp::vector<bes_expression> &approximation,
                const unsigned long rank)
{ /* substitute the approximation for variables in b, given
     by approximation, for all those variables that have a 
     rank higher or equal to the variable rank */

  if (is_if(b))
  { 
    bes::variable_type v=bes::get_variable(condition(b));
    if (bes_equations.get_rank(v)>=rank)
    { 
      bes_expression b1=evaluate_bex(then_branch(b),approximation,rank);
      bes_expression b2=evaluate_bex(else_branch(b),approximation,rank);
      return BDDif(approximation[v],b1,b2);
    }
    else
    { /* the condition has lower rank than the variable rank,
         leave it untouched */
      bes_expression b1=evaluate_bex(then_branch(b),approximation,rank);
      bes_expression b2=evaluate_bex(else_branch(b),approximation,rank);
      if ((b1==then_branch(b)) && (b2==else_branch(b)))
      { return b;
      }
      bes_expression br=BDDif(bes::if_(condition(b),bes::true_(),bes::false_()),b1,b2);
      return br;
    }
  }

  assert(bes::is_true(b)||bes::is_false(b));
  return b;
}

static bool solve_bes()
{ 
  gsVerboseMsg("Solving BES...\n");
  atermpp::vector<bes_expression> approximation(bes_equations.nr_of_equations()+1);

  /* Set the approximation to its initial value */
  for(bes::variable_type v=bes_equations.nr_of_equations(); v>0; v--)
  { if (bes::is_true(bes_equations.get_rhs(v)))
    { approximation[v]=bes::true_();
    }
    else if (bes::is_false(bes_equations.get_rhs(v)))
    { approximation[v]=bes::false_();
    }
    else if (bes_equations.get_fixpoint_symbol(v)==fixpoint_symbol::mu())
    { approximation[v]=bes::false_();
    } 
    else
    { approximation[v]=bes::true_();
    }
  }

  for(unsigned long current_rank=bes_equations.max_rank;
      current_rank>0 ; current_rank--)
  { 
    std::vector<std::set <bes::variable_type> > 
              variable_occurrence_set(bes_equations.nr_of_equations()+1);
    set <bes::variable_type> todo;

    /* put all variables of equations in the variable occurrence set, provided
       the rank of these variables have the current rank. A variable v is put
       in the set at index w, if w occurs in the rhs of v.
       Moreover, put all variables of current_rank in todo, because these are
       involved in the current iteration process.  */

    for(bes::variable_type v=bes_equations.nr_of_equations(); v>0; v--)
    { if (bes_equations.get_rank(v)==current_rank)
      { todo.insert(v);
        add_variables_to_set(v,bes_equations.get_rhs(v),variable_occurrence_set,current_rank);
      }
    }
  
    /* Calculate the stable solution for the current rank */
    for( ; todo.size()>0 ; )
    { set<bes::variable_type>::iterator w= todo.begin();
      bes::variable_type w_value=*w;
      todo.erase(w);
      for(set <bes::variable_type>::iterator u=variable_occurrence_set[w_value].begin();
          u!=variable_occurrence_set[w_value].end(); u++)
      { 
        bes_expression t=evaluate_bex(bes_equations.get_rhs(*u),approximation,current_rank);
        
        assert(bes_equations.get_rank(*u)==current_rank);
        if (t!=approximation[*u])
        { approximation[*u]=t;
          todo.insert(*u);
        }
      }
    }

    /* substitute the stable solution for the current rank in all other
       equations. */
 
    for(bes::variable_type v=bes_equations.nr_of_equations(); v>0; v--)
    { if (bes_equations.get_rank(v)==current_rank)
      { bes_equations.set_rhs(v,approximation[v]);
      }
      else 
      { bes_equations.set_rhs(v,substitute_rank(bes_equations.get_rhs(v),current_rank,approximation));
      }
    }
  }

  assert(bes::is_true(approximation[1])||
         bes::is_false(approximation[1]));
  return bes::is_true(approximation[1]);  /* 1 is the index of the initial variable */
}

//function load_pbes
//------------------
pbes load_pbes(t_tool_options tool_options)
{
  string infilename = tool_options.infilename;

  pbes pbes_spec;
  if (infilename == "-")
  {
    try
    {
      pbes_spec.load("-");
    }
    catch (std::runtime_error e)   
    {
      gsErrorMsg("Cannot open PBES from stdin\n");
      exit(1);
    }
  }
  else
  {
    try
    {
      pbes_spec.load(infilename);
    }
    catch (std::runtime_error e)   
    {
      gsErrorMsg("Cannot open PBES from '%s'\n", infilename.c_str());
      exit(1);
    }
  }
  return pbes_spec;
} 

//function save_bes_in_vasy_format
//--------------------------------

typedef enum { both, and_form, or_form} expression_sort;

static bes_expression translate_equation_for_vasy(const unsigned long i,
                                        const bes_expression b,
                                        const expression_sort s)
{
  if (bes::is_true(b))
  { return b;
  }
  else if (bes::is_false(b))
  { return b;
  }
  else if (bes::is_and(b))
  { 
    if (s==or_form)
    { /* make a new equation B=b, and return B */
      bes::variable_type v=bes_equations.nr_of_equations()+1;
      bes_equations.add_equation(v,
                                 bes_equations.get_fixpoint_symbol(i),
                                 bes_equations.get_rank(i),
                                 b);
      return bes::variable(v);
    }
    else
    {
      bes_expression b1=translate_equation_for_vasy(i,lhs(b),and_form);
      bes_expression b2=translate_equation_for_vasy(i,rhs(b),and_form);
      return and_(b1,b2);
    }
  }
  else if (bes::is_or(b))
  { 
    if (s==and_form)
    { /* make a new equation B=b, and return B */
      bes::variable_type v=bes_equations.nr_of_equations()+1;
      bes_equations.add_equation(v,
                                 bes_equations.get_fixpoint_symbol(i),
                                 bes_equations.get_rank(i),
                                 b);
      return bes::variable(v);
    }
    else
    {
      bes_expression b1=translate_equation_for_vasy(i,lhs(b),or_form);
      bes_expression b2=translate_equation_for_vasy(i,rhs(b),or_form);
      return or_(b1,b2);
    }
  }
  else if (bes::is_variable(b))
  {
    return b;
  }
  else
  {
    gsErrorMsg("The generated equation system is not a BES. It cannot be saved in VASY-format.\n");
    exit(1);
  }
  return b;
}


static void save_bes_in_vasy_format(string outfilename)
{
  gsVerboseMsg("Converting result to VASY-format...\n");
  // Use an indexed set to keep track of the variables and their vasy-representations

  /* First translate the right hand sides of the equations such that they only 
     contain only conjunctions of disjunctions. Note that dynamically new
     equations are added during the translation process in "translate_equation_for_vasy" 
     that must alos be translated. */

  for(unsigned long i=1; i<=bes_equations.nr_of_equations() ; i++)
    { 
        bes_equations.set_rhs(i,translate_equation_for_vasy(i,bes_equations.get_rhs(i),both));
    }

  /* Second give a consecutive index to each variable of a particular rank */

  std::vector<unsigned long> variable_index(bes_equations.nr_of_equations()+1);
  for(unsigned long r=1 ; r<=bes_equations.max_rank ; r++)
  { unsigned long index=0;
    for(unsigned long i=1; i<=bes_equations.nr_of_equations() ; i++)
    { if (bes_equations.get_rank(i)==r)
      { 
        variable_index[i]=index;
        index++;
      }
    }
  }

  /* Third save the equations in the forms of blocks of equal rank */

  ofstream outputfile;
  if (outfilename!="-")
  { outputfile.open(outfilename.c_str(), ios::trunc);
    if (!outputfile.is_open())
    { gsErrorMsg("Could not save BES to %s\n", outfilename.c_str());
      exit(1);
    }
  }

  for(unsigned long r=1 ; r<=bes_equations.max_rank ; r++)
  { bool first=true;
    for(unsigned long i=1; i<=bes_equations.nr_of_equations() ; i++)
    { if (bes_equations.get_rank(i)==r)
      { if (first)
        { ((outfilename=="-")?cout:outputfile) << 
             "block " << 
             ((bes_equations.get_fixpoint_symbol(i)==fixpoint_symbol::mu()) ? "mu  B" : "nu B") <<
             r-1 <<
             " is " << endl;
           first=false;
        }
        ((outfilename=="-")?cout:outputfile) << "  X" << variable_index[i] << " = ";
        save_rhs_in_vasy_form(((outfilename=="-")?cout:outputfile),
                                 bes_equations.get_rhs(i),
                                 variable_index,
                                 r);
        ((outfilename=="-")?cout:outputfile) << endl;
      }
    }
    ((outfilename=="-")?cout:outputfile) << "end block" << endl << endl;
  }

  outputfile.close();
}

//function save_rhs_in_vasy_form
//---------------------------
static void save_rhs_in_vasy_form(ostream &outputfile, 
                                     bes_expression b,
                                     std::vector<unsigned long> &variable_index,
                                     const unsigned long current_rank)
{
  if (bes::is_true(b))
  { outputfile << "true";
  }
  else if (bes::is_false(b))
  { outputfile << "false";
  }
  else if (bes::is_and(b))
  {
    //BESAnd(a,b) => a and b
    save_rhs_in_vasy_form(outputfile,lhs(b),variable_index,current_rank);
    outputfile << " and ";
    save_rhs_in_vasy_form(outputfile,rhs(b),variable_index,current_rank);
  }
  else if (bes::is_or(b))
  {
    //BESOr(a,b) => a or b
    save_rhs_in_vasy_form(outputfile,lhs(b),variable_index,current_rank);
    outputfile << " or ";
    save_rhs_in_vasy_form(outputfile,rhs(b),variable_index,current_rank);
  }
  else if (bes::is_variable(b))
  {
    // PropVar => <Int>
    outputfile << "X" << variable_index[get_variable(b)];
    if (bes_equations.get_rank(get_variable(b))!=current_rank)
    { outputfile << "_" << bes_equations.get_rank(get_variable(b))-1;
    }
  }
  else
  {
    gsErrorMsg("The generated equation system is not a BES. It cannot be saved in VASY-format.\n");
    exit(1);
  }
  return;
}

//function save_bes_in_cwi_format
//--------------------------------
static void save_bes_in_cwi_format(string outfilename)
{
  gsVerboseMsg("Converting result to CWI-format...\n");
  // Use an indexed set to keep track of the variables and their cwi-representations

  ofstream outputfile;
  if (outfilename!="-")
  { outputfile.open(outfilename.c_str(), ios::trunc);
    if (!outputfile.is_open())
    { gsErrorMsg("Could not save BES to %s\n", outfilename.c_str());
      exit(1);
    }
  }

  for(unsigned long r=1 ; r<=bes_equations.max_rank ; r++)
  { for(unsigned long i=1; i<=bes_equations.nr_of_equations() ; i++)
    { if (bes_equations.get_rank(i)==r)
      { ((outfilename=="-")?cout:outputfile) << 
              ((bes_equations.get_fixpoint_symbol(i)==fixpoint_symbol::mu()) ? "min X" : "max X") << i << "=";
        save_rhs_in_cwi_form(((outfilename=="-")?cout:outputfile),bes_equations.get_rhs(i));
        ((outfilename=="-")?cout:outputfile) << endl;
      }
    }
  }

  outputfile.close();
}

//function save_rhs_in_cwi
//---------------------------
static void save_rhs_in_cwi_form(ostream &outputfile, bes_expression b)
{
  if (bes::is_true(b))
  { outputfile << "T";
  }
  else if (bes::is_false(b))
  { outputfile << "F";
  }
  else if (bes::is_and(b))
  {
    //BESAnd(a,b) => (a & b)
    outputfile << "(";
    save_rhs_in_cwi_form(outputfile,lhs(b));
    outputfile << "&";
    save_rhs_in_cwi_form(outputfile,rhs(b));
    outputfile << ")";
  }
  else if (bes::is_or(b))
  {
    //BESOr(a,b) => (a | b)
    outputfile << "(";
    save_rhs_in_cwi_form(outputfile,lhs(b));
    outputfile << "|";
    save_rhs_in_cwi_form(outputfile,rhs(b));
    outputfile << ")";
  }
  else if (bes::is_variable(b))
  {
    // PropVar => <Int>
    outputfile << "X" << get_variable(b);
  }
  else
  { 
    gsErrorMsg("The generated equation system is not a BES. It cannot be saved in CWI-format.\n");
    exit(1);
  }
  return;
}

//function parse_command_line
//---------------------------
t_tool_options parse_command_line(int argc, char** argv)
{
  t_tool_options tool_options;
  string opt_outputformat;
  string opt_strategy;
  bool opt_precompile_pbes=false;
  string opt_rewriter;
  vector< string > file_names;

  po::options_description desc;

  desc.add_options()
      ("strategy,s",  po::value<string>(&opt_strategy)->default_value("lazy"), "use strategy arg (default 'lazy');\n"
       "The following strategies are available:\n"
       "lazy    Compute only boolean equations which can be reached from the initial state\n")
      ("rewriter,R", po::value<string>(&opt_rewriter)->default_value("inner"), "indicate the rewriter to be used. Options are:\n"
       "inner   interpreting innermost rewriter (default),\n"
       "jitty   interpreting just in time rewriter,\n"
       "innerc  compiling innermost rewriter (not for Windows),\n"
       "jittyc  compiling just in time rewriter (fastest, not for Windows).\n")
      ("precompile,P", "Precompile the pbes for faster rewriting. Does not work when the toolset is compiled in debug mode")
      ("output,o",  po::value<string>(&opt_outputformat)->default_value("none"), "use outputformat arg (default 'none');\n"
               "available outputformats are none, vasy and cwi")
      ("verbose,v",  "turn on the display of short intermediate gsMessages")
      ("debug,d",    "turn on the display of detailed intermediate gsMessages")
      ("version",    "display version information")
      ("help,h",    "display this help")
      ;

  po::options_description hidden("Hidden options");
  hidden.add_options()
      ("file_names",  po::value< vector< string > >(), "input/output files")
      ;

  po::options_description cmdline_options;
  cmdline_options.add(desc).add(hidden);

  po::options_description visible("Allowed options");
  visible.add(desc);

  po::positional_options_description p;
  p.add("file_names", -1);

  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv).options(cmdline_options).positional(p).run(), vm);
  po::notify(vm);

  if (vm.count("help"))
  {
    cerr << "Usage: " << argv[0] << " [OPTION]... [INFILE [OUTFILE]]" << endl;
    cerr << "Solves PBES from stdin or INFILE, or outputs an equivalent BES." << endl;
    cerr << "By default the result is written to stdout, otherwise to OUTFILE." << endl;
    cerr << endl;
    cerr << desc;

    exit(0);
  }
 
  if (vm.count("precompile"))
  { opt_precompile_pbes=true;
  }

  if (vm.count("version"))
  {
    cerr << NAME << " " << VERSION <<  " (revision " << REVISION << ")" << endl;

    exit(0);
  }

  if (vm.count("debug"))
  {
    gsSetDebugMsg();
  }

  if (vm.count("verbose"))
  {
    gsSetVerboseMsg();
  }

  if (vm.count("output")) // Output format
  {
    opt_outputformat = vm["output"].as< string >();
    if (!((opt_outputformat == "none") || (opt_outputformat == "vasy") || (opt_outputformat == "cwi")))
    {
      gsErrorMsg("Unknown outputformat specified. Available outputformats are none, vasy and cwi\n");
      exit(1);
    }
  }

  if (vm.count("strategy")) // Bes solving strategy (currently only one available)
  {
    opt_strategy = vm["strategy"].as< string >();
    if (!(opt_strategy == "lazy"))
    {
      gsErrorMsg("Unknown strategy specified. The only vailable strategy is lazy\n");
      exit(1);
    }
  }

  if (vm.count("rewriter")) // Select the rewiter to be used
  { 
    opt_rewriter = vm["rewriter"].as< string >();
    if (!(opt_rewriter == "inner") &&
        !(opt_rewriter == "jitty") &&
        !(opt_rewriter == "innerc") &&
        !(opt_rewriter == "jittyc"))
    { 
      gsErrorMsg("Unknown rewriter specified. Available rewriters are inner, jitty, innerc and jittyc\n");
      exit(1);
    }

        
  }
  
  if (vm.count("file_names"))
  {
    file_names = vm["file_names"].as< vector< string > >();
  }

  string infilename;
  string outfilename;
  if (file_names.size() == 0)
  {
    // Read from and write to stdin
    infilename = "-";
    outfilename = "-";
  }
  else if ( 2 < file_names.size())
  {
    cerr << NAME << ": Too many arguments" << endl;
    exit(1);
  }
  else
  {
    infilename = file_names[0];
    if (file_names.size() == 2)
    {
      outfilename = file_names[1];
    }
    else
    {
      outfilename = "-";
    }
  }
  
  tool_options.infilename = infilename;
  tool_options.outfilename = outfilename;
  
  tool_options.opt_outputformat = opt_outputformat;
  tool_options.opt_strategy = opt_strategy;
  if (opt_rewriter=="inner")
  { tool_options.rewrite_strategy=GS_REWR_INNER;
  }
  else if (opt_rewriter=="jitty")
  { tool_options.rewrite_strategy=GS_REWR_JITTY;
  }
  else if (opt_rewriter=="innerc")
  { tool_options.rewrite_strategy=GS_REWR_INNERC;
  }
  else if (opt_rewriter=="jittyc")
  { tool_options.rewrite_strategy=GS_REWR_JITTYC;
  }
  else assert(0); // Unknown rewriter specified. Should have been caught above.

  tool_options.opt_precompile_pbes=opt_precompile_pbes;
  
  return tool_options;
}


