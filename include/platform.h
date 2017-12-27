
#ifndef KONSTRUCTS_PLATFORM_H
#define KONSTRUCTS_PLATFORM_H

namespace konstructs {

    /**
     * This class contains platform specific code
     */
    class Platform {

    public:

        /**
         * Init the winsock ddl under WIN32, do nothing on other platforms.
         */
        static int init_winsock();
    };
}


#endif //KONSTRUCTS_PLATFORM_H
