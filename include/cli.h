
#ifndef KONSTRUCTS_CLI_H
#define KONSTRUCTS_CLI_H

namespace konstructs {

    /**
     * This class contains cli logic
     */
    class Cli {

    public:

        /**
         * Print the cli command line options to stdout
         */
        static void print_usage();

        static void argument_parser(int argc, char **argv, Settings settings);
    };

}


#endif //KONSTRUCTS_CLI_H
