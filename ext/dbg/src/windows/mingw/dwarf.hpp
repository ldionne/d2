// Copyright Edd Dawson 2012
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef DWARF_HPP_0207_29062012
#define DWARF_HPP_0207_29062012

namespace dbg 
{
    class file;

    // Loads and contains the DWARF debugging information for a process.
    // Allows lookup of function names based on code addresses.
    class dwarf
    {
        public:
            // Creates an empty() dwarf object.
            dwarf();

            // Attempts to load the MinGW DWARF data from the specified module,
            // whose corresponding file is specified. A dwarf_error is thrown
            // on failure.
            dwarf(const void *module_image, file &original_pe);

            ~dwarf();

            bool empty() const;

            // Swaps the guts of this dwarf object with those of other.
            void swap(dwarf &other);

            // Returns the name of the function spanning the specified code address,
            // or a null-valued pointer if no such function could be found.
            const char *function_spanning(const void *program_counter) const;

        private:
            dwarf(const dwarf &);
            dwarf &operator= (const dwarf &);

        private:
            class impl;
            impl *p;
    };

} // dbg

#endif // DWARF_HPP_0207_29062012
