#include <argparse/argparse.hpp>
#include <iostream>
#include <string>

#include "egraph.h"
#include "theory.h"

EGraph create_egraph()
{
    Theory theory;

    theory.add_operator("one", 0);
    theory.add_operator("inv", 1);
    theory.add_operator("mul", AC);

    theory.add_rewrite_rule("identity", "(mul ?x (one))", "?x");
    theory.add_rewrite_rule("inverse", "(mul ?x (inv ?x))", "(one)");

    theory.add_operator("v0", 0);
    theory.add_operator("v1", 0);
    theory.add_operator("v2", 0);
    theory.add_operator("v3", 0);

    EGraph egraph(theory);

    return egraph;
}

int main(int argc, char **argv)
{
    argparse::ArgumentParser program("groups");

    size_t iterations = 0;
    program.add_argument("-i", "--iterations").required().store_into(iterations);

    try
    {
        program.parse_args(argc, argv);
    }
    catch (const std::exception& err)
    {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        std::exit(1);
    }

    std::string src;
    std::getline(std::cin, src);

    EGraph egraph = create_egraph();
    egraph.add_expr(src);
    egraph.saturate(iterations);

    egraph.dump_to_file("dump.txt");

    return 0;
}
