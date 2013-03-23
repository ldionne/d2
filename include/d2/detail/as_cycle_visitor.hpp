/*!
 * @file
 * This file defines the `as_cycle_visitor` class.
 */

#ifndef D2_DETAIL_AS_CYCLE_VISITOR_HPP
#define D2_DETAIL_AS_CYCLE_VISITOR_HPP

#include <boost/type_traits/remove_reference.hpp>
#include <boost/move/utility.hpp>


namespace d2 {
namespace detail {
/*!
 * Cycle visitor wrapping a functor.
 *
 * When the `cycle` method is called, the wrapped functor is called
 * with the same arguments.
 *
 * The `as_cycle_visitor` can also be called, in which case it will forward
 * to the wrapped functor as if `cycle` had been called.
 */
template <typename F>
struct as_cycle_visitor {
    explicit as_cycle_visitor(F const& f)
        : f_(f)
    { }

    explicit as_cycle_visitor(BOOST_RV_REF(F) f)
        : f_(boost::move(f))
    { }

    template <typename Cycle, typename Graph>
    void cycle(BOOST_FWD_REF(Cycle) c, BOOST_FWD_REF(Graph) g) const {
        f_(boost::forward<Cycle>(c), boost::forward<Graph>(g));
    }

    template <typename Cycle, typename Graph>
    void operator()(BOOST_FWD_REF(Cycle) c, BOOST_FWD_REF(Graph) g) const {
        cycle(boost::forward<Cycle>(c), boost::forward<Graph>(g));
    }

private:
    // mutable because we don't care whether the functor's operator() is
    // const or not and we want our `cycle` method to be const.
    F mutable f_;
};

//! Return a `as_cycle_visitor` visitor with a deduced functor type.
template <typename F>
as_cycle_visitor<typename boost::remove_reference<F>::type>
on_cycle(BOOST_FWD_REF(F) f) {
    return as_cycle_visitor<typename boost::remove_reference<F>::type>(
                                                        boost::forward<F>(f));
}
} // end namespace detail
} // end namespace d2

#endif // !D2_DETAIL_AS_CYCLE_VISITOR_HPP
