#include "geom/polyhedron.h"
#include "geom/translate.h"
#include <iostream>
#include <boost/program_options.hpp>
#include "print_exception.h"
#include <vector>
#include <string>
#include "../throw_if.h"
#include "cxxcam/Math.h"
#include "rs274_transform.h"

namespace po = boost::program_options;
using namespace cxxcam;

// TODO add options to align by bounding box, e.g. top at z0, xy0 at center or corners
int main(int argc, char* argv[]) {
    po::options_description options("nc_transform");
    std::vector<std::string> args(argv, argv + argc);
    args.erase(begin(args));

    options.add_options()
        ("help,h", "display this help and exit")
        ("translate_x,x", po::value<double>(), "Translate along x axis")
        ("translate_y,y", po::value<double>(), "Translate along y axis")
        ("translate_z,z", po::value<double>(), "Translate along z axis")
        ("rotate_x,a", po::value<double>(), "Rotate around x axis")
        ("rotate_y,b", po::value<double>(), "Rotate around y axis")
        ("rotate_z,c", po::value<double>(), "Rotate around z axis")
        ("model,m", "transform model")
    ;

    try {
        auto parsed = po::command_line_parser(args).options(options).run();

        po::variables_map vm;
        store(parsed, vm);

        if(vm.count("help")) {
            std::cout << options << "\n";
            return 0;
        }

        if (vm.count("model")) {
            geom::polyhedron_t model;
            throw_if(!(std::cin >> geom::format::off >> model), "Unable to read model from file");

            // apply transformations in order
            for (auto& option : parsed.options) {
                if(option.string_key == "model")
                    continue;

                auto value = boost::lexical_cast<double>(option.value[0]);
                if(option.string_key == "translate_x") {
                    model = geom::translate(model, value, 0, 0);
                } else if(option.string_key == "translate_y") {
                    model = geom::translate(model, 0, value, 0);
                } else if(option.string_key == "translate_z") {
                    model = geom::translate(model, 0, 0, value);
                } else if(option.string_key == "rotate_x") {
                    auto theta = units::plane_angle{value * units::degrees};
                    auto q = math::normalise(math::axis2quat(1, 0, 0, theta));
                    model = geom::rotate(model, q.R_component_1(), q.R_component_2(), q.R_component_3(), q.R_component_4());
                } else if(option.string_key == "rotate_y") {
                    auto theta = units::plane_angle{value * units::degrees};
                    auto q = math::normalise(math::axis2quat(0, 1, 0, theta));
                    model = geom::rotate(model, q.R_component_1(), q.R_component_2(), q.R_component_3(), q.R_component_4());
                } else if(option.string_key == "rotate_z") {
                    auto theta = units::plane_angle{value * units::degrees};
                    auto q = math::normalise(math::axis2quat(0, 0, 1, theta));
                    model = geom::rotate(model, q.R_component_1(), q.R_component_2(), q.R_component_3(), q.R_component_4());
                }
            }

            std::cout << model;
        } else {
            // TODO store transformations / rotations in list
            // create rs274 transformer with transformations, go.
        }

    } catch(const po::error& e) {
        print_exception(e);
        std::cout << options << "\n";
        return 1;
    } catch(const std::exception& e) {
        print_exception(e);
        return 1;
    }
    return 0;
}
