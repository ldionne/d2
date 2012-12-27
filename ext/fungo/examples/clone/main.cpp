// Copyright Edd Dawson 2010
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <fungo/fungo.hpp>
#include <iostream>

// Here, we'll create a little cloneable exception hierarchy:

struct hyperdrive_problem
{
    virtual ~hyperdrive_problem() { }

    // A function to create a clone of the derived object
    virtual hyperdrive_problem *replicate() const = 0;

    // A function to throw a copy of the derived object
    virtual void throw_replica() const = 0;
};

struct damaged_motivator : hyperdrive_problem
{
    virtual damaged_motivator *replicate() const { return new damaged_motivator(); }
    virtual void throw_replica() const { throw damaged_motivator(*this); }
};

struct alluvial_dampers_malfunction : hyperdrive_problem
{
    virtual alluvial_dampers_malfunction *replicate() const { return new alluvial_dampers_malfunction(); }
    virtual void throw_replica() const { throw alluvial_dampers_malfunction(*this); }
};


// Given such an exception hierarchy, we can tell fungo about the base class by specializing 
// fungo::clone_policy as follows:

namespace fungo
{
    template<>
    struct clone_policy<hyperdrive_problem>
    {
        static hyperdrive_problem *clone(const hyperdrive_problem &problem) { return problem.replicate(); }   
        static void rethrow(const hyperdrive_problem &problem) { problem.throw_replica(); }   
    };

    // We don't need to specialize destroy_clone_policy, as the default template does the appropriate 
    // thing. But for the sake of the example:
    template<>
    struct destroy_clone_policy<hyperdrive_problem>
    {
        static void destroy_clone(hyperdrive_problem *problem) { delete problem; }   
    };
}

int main()
{
    fungo::catcher ruth;
    ruth.learn<hyperdrive_problem>();

    // We've told the catcher about the base class exception of the cloneable hierarchy.
    // There's no need to tell it about the derived classes:

    fungo::exception_cage cage;

    try { throw damaged_motivator(); }
    catch (...) { ruth.store_current_exception(cage); }

    // cage now holds an exception

    try { cage.rethrow(); }
    catch (const damaged_motivator &)
    {
        std::cout << "caught damaged_motivator\n";
    }



    // Let's also try alluvial_dampers_malfunction for good measure
    
    try { throw alluvial_dampers_malfunction(); }
    catch (...) { ruth.store_current_exception(cage); }

    // cage now holds an exception

    try { cage.rethrow(); }
    catch (const alluvial_dampers_malfunction &)
    {
        std::cout << "caught alluvial_dampers_malfunction\n";
    }

    return 0;
}
