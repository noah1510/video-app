#pragma once

#include <iostream>
#include <exception>

namespace sakurajin{
    class Helper{
    private:
        static void print_exception(const std::exception& e, int level){
            std::cerr << std::string(level, ' ') << "exception: " << e.what() << '\n';
            try {
                std::rethrow_if_nested(e);
            } catch(const std::exception& nestedException) {
                print_exception(nestedException, level+1);
            } catch(...) {}
        }
    public:
        static void print_exception(const std::exception& e){
            print_exception(e,0);
        }
    };
}
