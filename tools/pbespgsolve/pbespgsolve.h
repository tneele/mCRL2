// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file tools/pbespgsolve/pbespgsolve.h
/// \brief add your file description here.

#ifndef MCRL2_TOOLS_PBESPGSOLVE_H
#define MCRL2_TOOLS_PBESPGSOLVE_H

#ifndef WITH_MCRL2
#define WITH_MCRL2
#endif

#include <iostream>
#include "mcrl2/utilities/execution_timer.h"
#include "ParityGame.h"
#include "SmallProgressMeasures.h"
#include "PredecessorLiftingStrategy.h"
#include "RecursiveSolver.h"
#include "ComponentSolver.h"

namespace mcrl2
{

namespace pbes_system
{

enum pbespg_solver_type
{
  spm_solver,
  recursive_solver
};

/// \return A string representation of the edge direction.

inline
std::string print(StaticGraph::EdgeDirection edge_direction)
{
  if (edge_direction == StaticGraph::EDGE_SUCCESSOR)
  {
    return "successor";
  }
  else if (edge_direction == StaticGraph::EDGE_PREDECESSOR)
  {
    return "predecessor";
  }
  else if (edge_direction == StaticGraph::EDGE_BIDIRECTIONAL)
  {
    return "bidirectional";
  }
  return "unknown edge direction";
}

/// \return A string representation of the solver type.

inline
std::string print(pbespg_solver_type solver_type)
{
  if (solver_type == spm_solver)
  {
    return "spm_solver";
  }
  else if (solver_type == recursive_solver)
  {
    return "recursive_solver";
  }
  return "unknown solver";
}

struct pbespgsolve_options
{
  pbespg_solver_type solver_type;
#ifdef MCRL2_PBESPGSOLVE_ENABLE_SCC_DECOMPOSITION
  bool use_scc_decomposition;
#endif
  bool verify_solution;

  pbespgsolve_options()
    : solver_type(spm_solver),
#ifdef MCRL2_PBESPGSOLVE_ENABLE_SCC_DECOMPOSITION
      use_scc_decomposition(true),
#endif
      verify_solution(true)
  {
  }
};

class pbespgsolve_algorithm
{
  protected:
    std::auto_ptr<LiftingStrategyFactory> lift_strat_factory;
    std::auto_ptr<ParityGameSolverFactory> subsolver_factory;
    std::auto_ptr<ParityGameSolverFactory> solver_factory;
    mcrl2::utilities::execution_timer& m_timer;
    pbespgsolve_options m_options;

  public:

    pbespgsolve_algorithm(mcrl2::utilities::execution_timer& timing, pbespgsolve_options options = pbespgsolve_options())
      : m_timer(timing),
        m_options(options)
    {
      if (options.solver_type == spm_solver)
      {
        // Create a lifting strategy factory:
        lift_strat_factory.reset(
          new PredecessorLiftingStrategyFactory);

        // Create a SPM solver factory:
        solver_factory.reset(
          new SmallProgressMeasuresSolverFactory(lift_strat_factory.get()));
      }
      else if (options.solver_type == recursive_solver)
      {
        // Create a recursive solver factory:
        solver_factory.reset(new RecursiveSolverFactory);
      }
      else
      {
        throw mcrl2::runtime_error("pbespgsolve: unknown solver type");
      }

#ifdef MCRL2_PBESPGSOLVE_ENABLE_SCC_DECOMPOSITION
      if (options.use_scc_decomposition)
      {
        // Wrap solver factory into a component solver factory:
        subsolver_factory = solver_factory;
        solver_factory.reset(
          new ComponentSolverFactory(*subsolver_factory));
      }
#endif
    }

    template <typename Container>
    bool run(pbes<Container>& p)
    {
      m_timer.start("initialization");
      // Generate the game from a PBES:
      verti goal_v;
      ParityGame pg;
      pg.assign_pbes(p, &goal_v); // N.B. mCRL2 could raise an exception here

      // Create a solver:
      std::auto_ptr<ParityGameSolver> solver(solver_factory->create(pg));
      m_timer.finish("initialization");

      // Solve the game:
      m_timer.start("solving");
      ParityGame::Strategy solution = solver->solve();
      m_timer.finish("solving");

      if (solution.empty())
      {
        throw mcrl2::runtime_error("pbespgsolve: solving failed!\n");
      }

      verti error_vertex;

      // Optional: verify the solution
      if (m_options.verify_solution && !pg.verify(solution, &error_vertex))
      {
        throw mcrl2::runtime_error("pbespgsolve: verification of the solution failed!\n");
      }

      return pg.winner(solution, goal_v) == ParityGame::PLAYER_EVEN;
    }
};

} // namespace pbes_system

} // namespace mcrl2

#endif // MCRL2_TOOLS_PBESPGSOLVE_H
