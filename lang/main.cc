/*
 * main.cc -- Main function of gash lang
 *
 * Author: Xiaoting Tang <tang_xiaoting@brown.edu>
 * Copyright: Xiaoting Tang (2018)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
  * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "gash_lang.hh"
#include <fstream>

using po::options_description;
using po::value;
using po::variables_map;
using po::store;
using po::notify;
using po::parse_command_line;

using std::ofstream;

using gashlang::run;

int main(int argc, char* argv[])
{
    options_description desc("Allowed options");
    desc.add_options()("help,h", "produce help message for gash_lang")(
        "input,i", value<string>(),
        "file path of input function description file")(
        "output,o", value<string>(), "file path of circuit output file")(
        "data_output,d", value<string>(), "file path of data output file")(
        "mode,m", value<string>(),
        "running mode: normal(garbling and "
        "evaluating), verify(simply execute circuit)");

    variables_map vm;
    store(parse_command_line(argc, argv, desc), vm);
    notify(vm);

    if (vm.count("help")) {
        cout << "Usage: options description [options]\n";
        ;
        cout << desc;
        return 0;
    }

    const char* input;
    const char* circ_out;
    const char* data_out;
    const char* mode;
    if (!vm.count("input")) {
        cout << "Require input file" << endl;
        return 0;
    } else
        input = vm["input"].as<string>().c_str();

    if (!vm.count("output")) {
        cout << "Missing circuit output file name, using `out.circ`" << endl;
        circ_out = "out.circ";
    } else
        circ_out = vm["output"].as<string>().c_str();
    if (!vm.count("data_output")) {
        cout << "Missing data output file name, using `data.txt`" << endl;
        data_out = "data.txt";
    } else
        data_out = vm["data_output"].as<string>().c_str();
    if (!vm.count("mode")) {
        cout << "Missing mode, usng `normal`" << endl;
        mode = "normal";
    } else {
        mode = vm["mode"].as<string>().c_str();
    }

    ofstream circ_file(circ_out, std::ios::out | std::ios::trunc);
    ofstream data_file(data_out, std::ios::out | std::ios::trunc);

    run(circ_file, data_file, circ_out, data_out, input, mode);
}
