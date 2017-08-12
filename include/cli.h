
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

        /**
         * Simple argument parser that updates the settings struct
         * @param argc      From main()
         * @param argv      From main()
         * @param settings  The settings object to update
         */
        static void argument_parser(int argc, char **argv, Settings *settings);
    };

}


#endif //KONSTRUCTS_CLI_H
