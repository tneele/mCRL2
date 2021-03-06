#!/usr/bin/env python

#~ Copyright 2013, 2014 Mark Geelen.
#~ Copyright 2014, 2015 Wieger Wesselink.
#~ Distributed under the Boost Software License, Version 1.0.
#~ (See accompanying file LICENSE_1_0.txt or http://www.boost.org/LICENSE_1_0.txt)

import os
import os.path
import shutil
import sys
sys.path += [os.path.join(os.path.dirname(__file__), '..', '..', 'build', 'python')]
import testrunner
from text_utility import read_text, write_text
from tools import Node, ToolFactory

class ToolInputError(Exception):
    def __init__(self, name, value):
        self.tool = name
        self.value = value
    def __str__(self):
        return repr(self.value)

class UnusedToolsError(Exception):
    def __init__(self, value):
        self.value = value
    def __str__(self):
        return repr(self.value)

class ToolCrashedError(Exception):
    def __init__(self, value):
        self.value = value
    def __str__(self):
        return repr(self.value)

class Test:
    def __init__(self, file, settings):
        import yaml
        from collections import Counter

        if not settings:
            raise RuntimeError('ERROR in Test.__init__: settings == None')

        self.verbose = settings.get('verbose', True)
        self.toolpath = settings.get('toolpath', '')
        self.cleanup_files = settings.get('cleanup_files', False)
        self.timeout = 5
        self.memlimit = 500000000
        self.allow_non_zero_return_values = settings.get('allow-non-zero-return-values', False)

        # Reads a test from a YAML file
        self.name = file
        f = open(file)
        data = yaml.safe_load(f)

        # Add tool arguments specified in settings
        if 'tools' in settings:
            for tool in settings['tools']:
                if 'args' in settings['tools'][tool]:
                    data['tools'][tool]['args'] += settings['tools'][tool]['args']

        # Overwrite node values specified in settings
        if 'nodes' in settings:
            for label in settings['nodes']:
                data['nodes'][label]['value'] = settings['nodes'][label]['value']

        # Overwrite result value with the one specified in settings
        if 'result' in settings:
            data['result'] = settings['result']

        if 'memlimit' in settings:
            self.memlimit = settings['memlimit']

        if 'timeout' in settings:
            self.timeout = settings['timeout']

        #print yaml.dump(data)

        self.nodes = []
        for label in data['nodes']: # create nodes
            self._add_node(data['nodes'][label], label)

        self.tools = []
        for label in data['tools']: # create tools
            assert isinstance(data['tools'], dict)
            self._add_tool(data['tools'][label], label)

        self.result_code = data['result']
        f.close()

        # These are the global variables used for the computation of the test result
        self.globals = {}
        for node in self.nodes:
            self.globals[node.label] = node
        for tool in self.tools:
            self.globals[tool.label] = tool

        # Contains a list of input nodes of this test, sorted by label
        self.input_nodes = self.compute_input_nodes()

    def __str__(self):
        import io
        out = io.StringIO()
        out.write('name        = ' + str(self.name)     + '\n')
        out.write('verbose     = ' + str(self.verbose)  + '\n')
        out.write('result_code = ' + str(self.result_code)      + '\n\n')
        out.write('\n'.join(['--- Node ---\n{0}'.format(node) for node in self.nodes]) + '\n\n')
        out.write('\n'.join(['--- Tool ---\n{0}'.format(tool) for tool in self.tools]) + '\n\n')
        out.write('\n'.join(['--- Init ---\n{0}'.format(node) for node in self.input_nodes]))
        return out.getvalue()

    # Returns the input nodes of the test, ordered by label
    def compute_input_nodes(self):
        outputs = []
        for tool in self.tools:
            outputs = outputs + tool.output_nodes
        result = [node for node in self.nodes if not node in outputs]
        return sorted(result, key = lambda node: node.label)

    def _add_node(self, data, label):
        value = None
        if 'value' in data:
            value = data['value']
        self.nodes.append(Node(label, data['type'], value))

    def _find_node(self, label):
        return

    def _add_tool(self, data, label):
        import platform
        input_nodes = [next(node for node in self.nodes if node.label == key) for key in data['input']]
        output_nodes = sorted([node for node in self.nodes if node.label in data['output']], key = lambda node: node.label)
        name = data['name']
        if platform.system() == 'Windows':
            name = name + '.exe'
        self.tools.append(ToolFactory().create_tool(label, data['name'], self.toolpath, input_nodes, output_nodes, data['args']))

    def setup(self, inputfiles):
        input_nodes = [node for node in self.input_nodes if node.value == None]
        if len(input_nodes) != len(inputfiles):
            raise RuntimeError('Invalid number of input files provided: expected {0}, got {1}'.format(len(input_nodes), len(inputfiles)))
        for i in range(len(inputfiles)):
            shutil.copy(inputfiles[i], self.input_nodes[i].filename())
            self.input_nodes[i].value = read_text(inputfiles[i])

    def result(self):
        # Returns the result of the test after all tools have been executed
        try:
            exec(self.result_code, self.globals)
        except Exception as e:
            if isinstance(e, KeyError):
                print('A KeyError occurred during evaluation of the test result: {}'.format(e))
                print('result_code', self.result_code)
                print(self)
            else:
                print('An exception occurred during evaluation of the test result: {}'.format(e))
                print('result_code', self.result_code)
                print(self)
            return False
        return self.globals['result']

    def remaining_tasks(self):
        # Returns a list of tools that can be executed and have not been executed before
        return [tool for tool in self.tools if tool.can_execute()]

    def cleanup(self):
        if self.cleanup_files:
            filenames = [node.filename() for node in self.nodes] + ['commands']
            for filename in filenames:
                try:
                    os.remove(filename)
                except Exception as e:
                    if self.verbose:
                        print(e)

    def dump_file_contents(self):
        filenames = [node.filename() for node in self.nodes]
        for file in filenames:
            if os.path.exists(file) and (file.endswith('.mcrl2spec') or file.endswith('.pbesspec') or file.endswith('.statefrm')):
                contents = read_text(file)
                print('Contents of file {}:\n{}'.format(file, contents))

    def run(self):
        import popen

        # Singlecore run
        tasks = self.remaining_tasks()
        commands = []
        while len(tasks) > 0:
            tool = tasks[0]
            try:
                commands.append(tool.command())
                returncode = tool.execute(timeout = self.timeout, memlimit = self.memlimit, verbose = self.verbose)
                if returncode != 0 and not self.allow_non_zero_return_values:
                    self.dump_file_contents()
                    raise RuntimeError('The execution of tool {} ended with return code {}'.format(tool.name, returncode))
            except popen.MemoryExceededError as e:
                if self.verbose:
                    print('Memory limit exceeded: ' + str(e))
                self.cleanup()
                return None
            except popen.TimeExceededError as e:
                if self.verbose:
                    print('Time limit exceeded: ' + str(e))
                self.cleanup()
                return None
            except popen.StackOverflowError as e:
                if self.verbose:
                    print('Stack overflow detected during execution of the tool ' + tool.name)
                self.cleanup()
                return None
            tasks = self.remaining_tasks()

        if not all(tool.executed for tool in self.tools):
            not_executed = [tool for tool in self.tools if not tool.executed]
            raise UnusedToolsError(not_executed)
        else:
            for node in self.nodes:
                if not os.path.exists(node.filename()):
                    raise RuntimeError('Error in test {}: output file {} is missing!'.format(self.name, node.filename()))
            write_text('commands', '\n'.join(commands))
            result = self.result()
            self.cleanup()
            return result

    # Returns the tool with the given label
    def tool(self, label):
        try:
            return next(tool for tool in self.tools if tool.label == label)
        except StopIteration:
            raise RuntimeError("could not find model a tool with label '{0}'".format(label))

    def print_commands(self, runpath):
        from graph_algorithms import topological_sort, insert_edge
        G = {}
        for node in self.nodes:
            G[node.label] = (set(), set())
        for tool in self.tools:
            G[tool.label] = (set(), set())
        for tool in self.tools:
            for node in tool.input_nodes:
                insert_edge(G, node.label, tool.label)
            for node in tool.output_nodes:
                insert_edge(G, tool.label, node.label)
        labels = topological_sort(G)
        toolmap = {}
        for tool in self.tools:
            toolmap[tool.label] = tool
        print('\n'.join([toolmap[label].command(runpath) for label in labels if label in toolmap]))

def run_yml_test(name, testfile, inputfiles, settings):
    for filename in [testfile] + inputfiles:
        if not os.path.isfile(filename):
            print('Error:', filename, 'does not exist!')
            return
    t = Test(testfile, settings)
    if 'verbose' in settings and settings['verbose']:
        print('Running test ' + testfile)
    t.setup(inputfiles)
    result = t.run()
    print('{} {}'.format(name, result))
    if result == False:
        for filename in inputfiles:
            text = read_text(filename)
            print('- file {}\n{}\n'.format(filename, text))
    return result

class TestRunner(testrunner.TestRunner):
    def __init__(self):
        super(TestRunner, self).__init__()
        self.settings = {'toolpath': self._tool_path,
                         'verbose': self._args.verbose,
                         'cleanup_files': not self._args.keep_files}
        self.tests = []

    def main(self):
        if self._args.print_names:
            self.print_names()
        if self._args.command is not None:
            try:
                test = self.tests[self._args.command]
                test.print_commands(os.path.join(os.getcwd(), test.name))
            except Exception as e:
                sys.exit(str(e))
        super(TestRunner, self).main()

    def ymlfile(self, name):
        return '{}/tests/specifications/{}.yml'.format(self._source_path, name)

    def mcrl2file(self, file):
        return self._source_path + file

    def _get_commandline_parser(self):
        parser = super(TestRunner, self)._get_commandline_parser()
        parser.add_argument('-v', '--verbose', dest='verbose', action='store_true', help='Display additional progress messages.')
        parser.add_argument('-k', '--keep-files', dest='keep_files', action='store_true', help='Keep the files produced by the test')
        parser.add_argument('-p', '--print-names', dest='print_names', action='store_true', help='Print the names and the numbers of the tests')
        parser.add_argument('-c', '--print-commands', dest='command', metavar='N', type=int, action='store', help='Print the commands of test N, or exit with return value 1 if N is too large.')
        return parser

    def names(self):
        for test in self.tests:
            yield test.name

    # displays names and numbers of the tests
    def print_names(self):
        for i, test in enumerate(self.tests):
            print('{} {}'.format(i, test.name))

    def run(self, testnum):
        if testnum < len(self.tests):
            test = self.tests[testnum]
            test.settings.update(self.settings)
            test.execute_in_sandbox()
        else:
            raise RuntimeError('Invalid test number')
