// Author(s): Jan Friso Groote
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file pbesrewr.cpp
/// \brief Rewrites and simplifies the rhs's of a pbes

// ======================================================================
//
// file          : pbesrewr
// date          : 2-08-2007
// version       : 0.1.0
//
// author        : Jan Friso Groote <J.F.Groote@tue.nl>
//
// ======================================================================

#define NAME "pbesrewr"
#define AUTHOR "Jan Friso Groote"

//C++
#include <cstdio>
#include <exception>
#include <iostream>
#include <string>

//Boost
#include <boost/program_options.hpp>

//MCRL2-specific
#include "mcrl2/pbes/pbes.h"
#include "mcrl2/pbes/utility.h"
#include "mcrl2/core/messaging.h"
#include "mcrl2/utilities/version_info.h"
#include "mcrl2/utilities/aterm_ext.h"

#include "mcrl2/data/rewrite.h"

using namespace std;
using namespace ::mcrl2::utilities;
using namespace mcrl2::core;
using namespace mcrl2::lps;
using namespace mcrl2::pbes_system;

namespace po = boost::program_options;

typedef struct{
  string infile_name, outfile_name;
  RewriteStrategy rewrite_strategy;
} t_tool_options;

// Name of the file to read input from

t_tool_options parse_command_line(int argc, char** argv)
{
  po::options_description desc;
  t_tool_options tool_options;
  string opt_rewriter;

  desc.add_options()
      ("rewriter,R", po::value<string>(&opt_rewriter)->default_value("jitty"), "use rewrite strategy arg:\n"
       "'inner' for the innermost rewriter,\n"
       "'innerc' for the compiled innermost rewriter,\n"
       "'jitty' for the jitty rewriter (default), or\n"
       "'jittyc' for the compiled jitty rewriter")
      ("help,h",    "display this help")
      ("verbose,v",  "turn on the display of short intermediate messages")
      ("debug,d",    "turn on the display of detailed intermediate messages")
      ("version",    "display version information")
      ;
  
  po::options_description hidden("Hidden options");
  hidden.add_options()
      ("INFILE",    po::value<string>(), "input file")
      ("OUTFILE",    po::value<string>(), "output file")
      ;
  
  po::options_description cmdline_options;
  cmdline_options.add(desc).add(hidden);
  
  po::options_description visible("Allowed options");
  visible.add(desc);
  
  po::positional_options_description p;
  p.add("INFILE", 1);
  p.add("OUTFILE", -1);
  
  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv).options(cmdline_options).positional(p).run(), vm);
  po::notify(vm);
  
  if (vm.count("help"))
  {
    cout << "Usage: " << argv[0] << " [OPTION]... [INFILE] [OUTFILE]" << endl;
    cout << "Rewrite the PBES in INFILE, remove quantified variables and write the resulting" << endl;
    cout << "PBES to OUTFILE. If INFILE is not present, stdin is used. If OUTFILE is not" << endl;
    cout << "present, stdout is used" << endl;
    cout << endl;
    cout << "Options:" << endl;
    cout << desc;
    cout << endl;
    cout << "Report bugs at <http://www.mcrl2.org/issuetracker>." << endl;
    
    exit(0);
  }
  
  if (vm.count("version"))
  {
    print_version_information(NAME, AUTHOR);
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

  
  tool_options.infile_name = (0 < vm.count("INFILE")) ? vm["INFILE"].as<string>() : "-";  
  tool_options.outfile_name = (0 < vm.count("OUTFILE")) ? vm["OUTFILE"].as<string>() : "-";  

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


  return tool_options;
}

int main(int argc, char** argv)
{
  MCRL2_ATERM_INIT(argc, argv)

  t_tool_options tool_options = parse_command_line(argc, argv);

  pbes<> old_pbes_specification;

  /// If PBES can be loaded from file_name, then
  /// - Rewrite all data expressions
  /// - Translate all data expressions true and false to pbes expressions true and false
  /// - Simplify all and an or expressions
  /// - Remove all quantified variables that do not bind anything.

  try
  {
    old_pbes_specification.load(tool_options.infile_name);

    atermpp::vector<pbes_equation> old_eqsys = old_pbes_specification.equations();
    atermpp::vector<pbes_equation> new_eqsys;
  
    Rewriter *r=createRewriter(old_pbes_specification.data(),tool_options.rewrite_strategy);
    
    for (atermpp::vector<pbes_equation>::iterator fp_i = old_eqsys.begin(); fp_i != old_eqsys.end(); fp_i++)
    {
      new_eqsys.push_back(pbes_equation(
                               fp_i->symbol(),
                               fp_i->variable(),
                               pbes_expression_rewrite_and_simplify(fp_i->formula(),r)));
    }
    
    pbes<> new_pbes_specification(
             old_pbes_specification.data(),
             new_eqsys,
             old_pbes_specification.free_variables(),
             old_pbes_specification.initial_state());
    new_pbes_specification.save(tool_options.outfile_name);
    
  }
  catch (std::runtime_error e)
  {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
