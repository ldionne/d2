// Copyright Edd Dawson 2010
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <test_o_matic.hpp>
#include <fungo/fungo.hpp>
#include <stdexcept>

namespace
{
    struct base
    {
        virtual ~base() { }
    };

    struct small_exception : base { };

    struct little_exception : small_exception { };

    struct bigger_exception : small_exception { char block[10]; };

    struct fixture
    {
        fungo::catcher mitten;
        fungo::exception_cage cage;
    };

} // close anonymous namespace

TESTFIX("catcher: copy constructor", fixture)
{
    mitten.learn<std::runtime_error>();

    fungo::catcher mitten2(mitten);

    try { throw std::runtime_error("oops"); }
    catch (...) 
    { 
        TRY( mitten2.store_current_exception(cage) );
    }
}

TESTFIX("catcher: copy assignment", fixture)
{
    mitten.learn<std::runtime_error>();

    fungo::catcher mitten2;
    CHECK( &(mitten2 = mitten) == &mitten2 );

    try { throw std::runtime_error("oops"); }
    catch (...) 
    { 
        TRY( mitten2.store_current_exception(cage) );
    }
}

TESTFIX("catcher: fungo exceptions are pre-registered", fixture)
{
    try { throw fungo::clone_failure(); }
    catch (...) { mitten.store_current_exception(cage); }
    THROWS( cage.rethrow_and_clear(), fungo::clone_failure );

    try { throw fungo::unknown_exception(); }
    catch (...) { mitten.store_current_exception(cage); }
    THROWS( cage.rethrow(), fungo::unknown_exception );
}

TEST("catcher: learn() orders exceptions by size")
{
    REQUIRE(sizeof(small_exception) < sizeof(bigger_exception));

    {
        fungo::catcher mitten;
        fungo::exception_cage cage;

        mitten.learn<small_exception>();
        mitten.learn<bigger_exception>();

        try { throw bigger_exception(); }
        catch (...) { mitten.store_current_exception(cage); }

        THROWS( cage.rethrow(), bigger_exception );
    }

    {
        fungo::catcher mitten;
        fungo::exception_cage cage;

        mitten.learn<bigger_exception>();
        mitten.learn<small_exception>();

        try { throw bigger_exception(); }
        catch (...) { mitten.store_current_exception(cage); }

        THROWS( cage.rethrow(), bigger_exception );
    }
}

TEST("catcher: learn() orders exceptions by derived-ness")
{
    REQUIRE(sizeof(small_exception) == sizeof(little_exception));

    {
        fungo::catcher mitten;
        fungo::exception_cage cage;

        mitten.learn<small_exception>();
        mitten.learn<little_exception>();

        try { throw small_exception(); }
        catch (...) { mitten.store_current_exception(cage); }

        THROWS( cage.rethrow(), small_exception );
    }

    {
        fungo::catcher mitten;
        fungo::exception_cage cage;

        mitten.learn<little_exception>();
        mitten.learn<small_exception>();

        try { throw small_exception(); }
        catch (...) { mitten.store_current_exception(cage); }

        THROWS( cage.rethrow(), small_exception );
    }
}

namespace
{
    struct chucky 
    {
        static bool throw_on_copy;

        chucky() { }
    
        chucky(const chucky &) 
        {
            if (throw_on_copy)
                throw std::runtime_error("chucky copy");
        }
    };
    bool chucky::throw_on_copy = false;

    struct chucky_fixture : fixture
    {
        chucky_fixture() { chucky::throw_on_copy = false; }
    };

}

TESTFIX("catcher: store_current_exception() propagates unrecognised exceptions unaltered", fixture)
{
    try { throw std::runtime_error("oops"); }
    catch (...) 
    { 
        THROWS( mitten.store_current_exception(cage), std::runtime_error );
    }

    CHECK( cage.empty() );
}

TESTFIX("catcher: store_current_exception() propagates exception thrown from clone operation", chucky_fixture)
{
    mitten.learn<chucky>();

    try { throw chucky(); }
    catch (...) 
    { 
        chucky::throw_on_copy = true;
        THROWS( mitten.store_current_exception(cage), std::runtime_error );
    }

    CHECK( cage.empty() );
}

TESTFIX("catcher: no-throw store_current_exception() stores unknown_exception as a fallback", fixture)
{
    try { throw std::runtime_error("oops"); }
    catch (...) 
    { 
        TRY( mitten.store_current_exception(cage, std::nothrow) );
        THROWS( cage.rethrow(), fungo::unknown_exception );
    }
}

TESTFIX("catcher: no-throw store_current_exception() stores clone_failure when clone throws", chucky_fixture)
{
    mitten.learn<chucky>();

    try { throw chucky(); }
    catch (...) 
    { 
        chucky::throw_on_copy = true;
        TRY( mitten.store_current_exception(cage, std::nothrow) );
        THROWS( cage.rethrow(), fungo::clone_failure );
    }
}

TESTFIX("catcher: swap() method", fixture)
{
    mitten.learn<std::runtime_error>();

    fungo::catcher mitten2;
    mitten2.swap(mitten);

    try { throw std::runtime_error("oops"); }
    catch (...) { mitten2.store_current_exception(cage); }

    THROWS( cage.rethrow(), std::runtime_error );

    try { throw std::runtime_error("oops"); }
    catch (...) 
    {
        THROWS( mitten.store_current_exception(cage), std::runtime_error );
    }
}

TESTFIX("catcher: swap() free-function", fixture)
{
    using std::swap;

    mitten.learn<std::runtime_error>();

    fungo::catcher mitten2;
    swap(mitten, mitten2);

    try { throw std::runtime_error("oops"); }
    catch (...) { mitten2.store_current_exception(cage); }

    THROWS( cage.rethrow(), std::runtime_error );

    try { throw std::runtime_error("oops"); }
    catch (...) 
    {
        THROWS( mitten.store_current_exception(cage), std::runtime_error );
    }
}

namespace
{
    struct cloneable_base
    {
        virtual ~cloneable_base() { }

        virtual cloneable_base *clone() const = 0;
        virtual void throw_copy() const = 0;
    };

    struct cloneable_derived : cloneable_base
    {
        virtual cloneable_base *clone() const { return new cloneable_derived(*this); }
        virtual void throw_copy() const { throw cloneable_derived(*this); }
    };

    unsigned destroy_clones_called = 0;

    struct cloneable_fixture : fixture
    {
        cloneable_fixture() 
        { 
            destroy_clones_called = 0; 
            mitten.learn<cloneable_base>();
        }
    };
}

namespace fungo
{
    template<>
    struct clone_policy<cloneable_base>
    {
        static cloneable_base *clone(const cloneable_base &e) { return e.clone(); }
        static void rethrow(const cloneable_base &e) { e.throw_copy(); }
    };

    template<>
    struct destroy_clone_policy<cloneable_base>
    {
        static void destroy_clone(const cloneable_base *e) { delete e; destroy_clones_called++; }
    };
}

TESTFIX("catcher: picks up clone_policy for polymorphic base exception", cloneable_fixture)
{
    try { throw cloneable_derived(); }
    catch (...)
    {
        TRY( mitten.store_current_exception(cage); );
        THROWS( cage.rethrow(), cloneable_derived );
    }
}

TESTFIX("catcher: picks up destroy_clone_policy for polymorphic base exception", cloneable_fixture)
{
    try { throw cloneable_derived(); }
    catch (...)
    {
        TRY( mitten.store_current_exception(cage); );
        THROWS( cage.rethrow(), cloneable_derived );
    }

    CHECK( destroy_clones_called == 0 );
    cage.clear();
    CHECK( destroy_clones_called == 1 );
}

TESTFIX("exception_cage: initially empty", fixture)
{
    CHECK( cage.empty() );
    TRY( cage.rethrow() );
}

TESTFIX("exception_cage: clear()", fixture)
{
    try { throw fungo::unknown_exception(); }
    catch (...) { mitten.store_current_exception(cage); }

    CHECK( !cage.empty() );
    cage.clear();

    CHECK( cage.empty() );
    TRY( cage.rethrow() );
}

TESTFIX("exception_cage: rethrow_and_clear() throws and leaves cage empty", fixture)
{
    try { throw fungo::unknown_exception(); }
    catch (...) { mitten.store_current_exception(cage); }

    THROWS( cage.rethrow_and_clear(), fungo::unknown_exception );
    CHECK( cage.empty() );
}

TESTFIX("exception_cage: swap() method", fixture)
{
    try { throw fungo::unknown_exception(); }
    catch (...) { mitten.store_current_exception(cage); }

    fungo::exception_cage cage2;
    cage2.swap(cage);

    CHECK( cage.empty() );
    CHECK( !cage2.empty() );

    THROWS( cage2.rethrow(), fungo::unknown_exception );
}
