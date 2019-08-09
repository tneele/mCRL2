// Author(s): Maurice Laveaux
// Copyright: see the accompanying file COPYING or copy at
// https://github.com/mCRL2org/mCRL2/blob/master/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "mcrl2/lps/stochastic_specification.h"

namespace mcrl2
{

/// \brief Given a list of assignments and parameters returns a list of assignments that only contain the assignments
///        for the given parameters and not for the potential other variables.
/// \returns A list of assignments only over the given parameters.
data::assignment_list project(const data::assignment_list& assignments, const data::variable_list& parameters)
{
  data::assignment_list result;
  for (auto& assignment : assignments)
  {
    // If the variable is in the parameters then copy the assignment.
    if (std::find_if(parameters.begin(), parameters.end(), [&](const data::variable& param) -> bool { return param == assignment.lhs(); } ) != parameters.end())
    {
      result.push_front(assignment);
    }
  }

  return result;
}

/// \brief Prints the parameters as comma separated values.
template<typename Container, typename F>
void print_values(const std::string& identifier, const Container& elements, F name)
{
  mCRL2log(log::info) << identifier << ": ";

  bool first = true;
  for (const auto& element : elements)
  {
    if (!first)
    {
      mCRL2log(log::info) << ", ";
    }
    mCRL2log(log::info) << name(element);
    first = false;
  }

  mCRL2log(log::info) << "\n";
}

template<typename Container>
void print_names(const std::string& identifier, const Container& variables)
{
  print_values(identifier, variables, [](const data::variable& var) { return var.name(); });
}

template<template<typename ...Element> class Container, typename El>
void print_elements(const std::string& identifier, const Container<El>& elements)
{
  print_values(identifier, elements, [](const El& element) { return element; });
}

bool is_independent(const data::variable_list& parameters, const std::set<data::variable>& dependencies, const data::assignment_list& other_assignments)
{
  // We are independent whenever all dependencies are included in our own parameters and the other process does no assignments.
  return std::includes(parameters.begin(), parameters.end(), dependencies.begin(), dependencies.end()) && other_assignments.empty();
}

template<typename LinearProcess>
data::variable_list get_other_parameters(const LinearProcess& process, const data::variable_list& parameters)
{
  // The parameters of the "other" component process.
  data::variable_list other_parameters;

  for (auto& param : process.process_parameters())
  {
    if (std::find(parameters.begin(), parameters.end(), param) == parameters.end())
    {
      other_parameters.push_front(param);
    }
  }

  return other_parameters;
}

/// \brief Creates a single summand for the cleave process.
template<bool owning = false>
lps::stochastic_action_summand cleave_summand(
  const lps::stochastic_specification& spec,
  std::size_t summand_index,
  const data::variable_list& parameters,
  const data::variable_list& other_parameters,
  std::vector<process::action_label>& sync_labels,
  const process::action& tag,
  const process::action& intern)
{
  lps::stochastic_action_summand summand = spec.process().action_summands()[summand_index];

  /// Compute the set S^i_V

  // Find the dependencies of the condition.
  std::set<data::variable> dependencies = data::find_free_variables(summand.condition());

  // Find the dependencies of the action that are not parameters of this process.
  std::set<data::variable> action_dependencies;
  for (auto& action : summand.multi_action().actions())
  {
    auto dependencies = data::find_free_variables(action);
    action_dependencies.insert(dependencies.begin(), dependencies.end());
  }

  // Find the dependencies of the assignments of the other process.
  std::set<data::variable> assignment_dependencies;
  std::set<data::variable> other_assignment_dependencies;
  for (auto& assignment : summand.assignments())
  {
    auto dependencies = data::find_free_variables(assignment.rhs());
    if (std::find(parameters.begin(), parameters.end(), assignment.lhs()) != parameters.end())
    {
      // This is an assignment for our parameters
      assignment_dependencies.insert(dependencies.begin(), dependencies.end());
    }
    else
    {
      other_assignment_dependencies.insert(dependencies.begin(), dependencies.end());
    }
  }

  // Remove our own parameters from our own assignments and our action dependencies.
  for (auto& param : parameters)
  {
    action_dependencies.erase(param);
    assignment_dependencies.erase(param);
  }

  // Remove the other parameters from its assignments.
  for (auto& param : other_parameters)
  {
    other_assignment_dependencies.erase(param);
  }

  // Gather all the necessary dependencies S^i_V.
  dependencies.insert(action_dependencies.begin(), action_dependencies.end());
  dependencies.insert(assignment_dependencies.begin(), assignment_dependencies.end());
  dependencies.insert(other_assignment_dependencies.begin(), other_assignment_dependencies.end());

  // Remove the global variables.

  // This version crashes, but would be nicer/more efficient:
  // dependencies.erase(spec.global_variables().begin(), spec.global_variables().end());
  for (auto& variable : spec.global_variables())
  {
    dependencies.erase(variable);
  }

  print_names(std::string("Dependencies (S^i_X) of summand ") += std::to_string(summand_index), dependencies);

  // Add a summation for every parameter of the other process that we depend on.
  data::variable_list variables = summand.summation_variables();
  for (auto& variable : other_parameters)
  {
    if (dependencies.count(variable) > 0)
    {
      variables.push_front(variable);
    }
  }

  // Create the actsync(p, e_i) action for our dependencies p and e_i
  data::data_expression_list values;
  data::sort_expression_list sorts;
  for (auto& dependency : dependencies)
  {
    values.push_front(data::data_expression(dependency));
    sorts.push_front(dependency.sort());
  }

  sync_labels.emplace_back(std::string("actsync_") += std::to_string(summand_index), sorts);

  // Remove the dependencies on local variables for checking parameter dependencies.
  data::variable_list variables_summand = summand.summation_variables();
  for (auto& variable : variables_summand)
  {
    dependencies.erase(variable);
  }

  auto other_assignments = project(summand.assignments(), other_parameters);
  auto assignments = project(summand.assignments(), parameters);

  lps::multi_action action;
  if (owning)
  {
    // Here the action is performed by the current process.

    // Convert tau actions to a visible action.
    process::action_list actions;

    for (const process::action& action : summand.multi_action().actions())
    {
      if (process::is_tau(action))
      {
        mCRL2log(log::info) << "Replaced tau.\n";
        actions.push_front(intern);
      }
      else
      {
        actions.push_front(action);
      }
    }

    if (other_assignment_dependencies.empty() && action_dependencies.empty() && )

    if (!is_independent(parameters, dependencies, other_assignments))
    {
      // This summand depends on the other process.
      actions.push_front(process::action(sync_labels.back(), values));
    }
    else
    {
      // We are independent, so tag it with our label.
      actions.push_front(tag);
    }

    action = lps::multi_action(actions);
  }
  else
  {
    if (!is_independent(other_parameters, dependencies, assignments))
    {
      // The other process depends on our parameters and we do not perform state updates.
      process::action_list actions;
      actions.push_front(process::action(sync_labels.back(), values));
      action = lps::multi_action(actions);
    }
    else
    {
      return lps::stochastic_action_summand(variables, data::sort_bool::false_(), action, assignments, summand.distribution());
    }
  }

  return lps::stochastic_action_summand(variables, summand.condition(), action, assignments, summand.distribution());
}

/// \brief Performs the a dependency cleave based on the given parameters V, and the indices J.
lps::stochastic_specification dependency_cleave(const lps::stochastic_specification& spec,
  const data::variable_list& parameters,
  const std::list<std::size_t>& indices,
  bool right_process)
{
  // Check sanity conditions, no timed or stochastic processes.
  auto& process = spec.process();

  if (process.has_time())
  {
    throw runtime_error("Cleave does not support timed processes");
  }

  // The parameters of the "other" component process.
  data::variable_list other_parameters = get_other_parameters(process, parameters);

  print_names("Parameters", parameters);
  print_names("Other parameters", other_parameters);
  print_elements("Indices", indices);

  // Extend the action specification with an actsync (that is unique) for every summand with the correct sorts, a tag and an intern action.
  std::vector<process::action_label> labels;

  // Add the tags for the left and right processes
  if (right_process)
  {
    labels.emplace_back(process::action_label("tag_right", {}));
  }
  else
  {
    labels.emplace_back(process::action_label("tag_left", {}));
  }

  process::action tag(labels.back(), {});

  labels.emplace_back(process::action_label("intern", {}));
  process::action intern(labels.back(), {});

  // Change the summands to include the parameters of the other process and added the sync action.
  lps::stochastic_action_summand_vector cleave_summands;

  // Add the summands that generate the action label.
  for (auto& index : indices)
  {
    if (index < process.action_summands().size())
    {
      cleave_summands.push_back(cleave_summand<true>(spec, index, parameters, other_parameters, labels, tag, intern));
    }
  }

  // Add the other summand that do not own the action. Indices should be sorted before this loop.
  auto it = indices.begin();
  for (std::size_t index = 0; index < process.action_summands().size(); ++index)
  {
    // Invariant: The index of *it is always higher than the loop index or it is the end
    if (it != indices.end())
    {
      if (*it < index)
      {
        // We have past the last index of the array.
        ++it;
      }
      if (it != indices.end() && *it == index)
      {
        // This summand was already created above.
        continue;
      }
    }

    // Index is not an element of indices.
    cleave_summands.push_back(cleave_summand<false>(spec, index, parameters, other_parameters, labels, tag, intern));
  }

  // Add the labels to the LPS action specification.
  auto cleave_action_labels = spec.action_labels();
  for (const auto& label : labels)
  {
    cleave_action_labels.push_front(label);
  }

  lps::deadlock_summand_vector no_deadlock_summands;
  lps::stochastic_linear_process cleave_process(parameters, no_deadlock_summands, cleave_summands);

  lps::stochastic_process_initializer cleave_initial(project(spec.initial_process().assignments(), parameters), spec.initial_process().distribution());

  // Create the new LPS and return it.
  return lps::stochastic_specification(spec.data(), cleave_action_labels, spec.global_variables(), cleave_process, cleave_initial);
}

} // namespace mcrl2