// Boost.Geometry Index
//
// R-tree nodes based on run-time polymorphism, storing std::vectors
//
// Copyright (c) 2011-2013 Adam Wulkiewicz, Lodz, Poland.
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_INDEX_DETAIL_RTREE_NODE_NODE_DEFAULT_HPP
#define BOOST_GEOMETRY_INDEX_DETAIL_RTREE_NODE_NODE_DEFAULT_HPP

#include <boost/container/vector.hpp>

#include <boost/geometry/index/detail/rtree/node/dynamic_visitor.hpp>
#include <boost/geometry/index/detail/rtree/node/auto_deallocator.hpp>

namespace boost { namespace geometry { namespace index {

namespace detail { namespace rtree {

template <typename Value, typename Parameters, typename Box, typename Allocators>
struct dynamic_internal_node<Value, Parameters, Box, Allocators, node_d_mem_dynamic_tag>
    : public dynamic_node<Value, Parameters, Box, Allocators, node_d_mem_dynamic_tag>
{
    typedef typename Allocators::leaf_allocator_type::template rebind<
        std::pair<Box, typename Allocators::node_pointer>
    >::other elements_allocator_type;

    typedef boost::container::vector<
        std::pair<Box, typename Allocators::node_pointer>,
        elements_allocator_type
    > elements_type;

    template <typename Al>
    inline dynamic_internal_node(Al const& al)
        : elements(al)
    {}

    void apply_visitor(dynamic_visitor<Value, Parameters, Box, Allocators, node_d_mem_dynamic_tag, false> & v) { v(*this); }
    void apply_visitor(dynamic_visitor<Value, Parameters, Box, Allocators, node_d_mem_dynamic_tag, true> & v) const { v(*this); }

    elements_type elements;
};

template <typename Value, typename Parameters, typename Box, typename Allocators>
struct dynamic_leaf<Value, Parameters, Box, Allocators, node_d_mem_dynamic_tag>
    : public dynamic_node<Value, Parameters, Box, Allocators, node_d_mem_dynamic_tag>
{
    typedef typename Allocators::leaf_allocator_type::template rebind<
        Value
    >::other elements_allocator_type;

    typedef boost::container::vector<
        Value,
        elements_allocator_type
    > elements_type;

    template <typename Al>
    inline dynamic_leaf(Al const& al)
        : elements(al)
    {}

    void apply_visitor(dynamic_visitor<Value, Parameters, Box, Allocators, node_d_mem_dynamic_tag, false> & v) { v(*this); }
    void apply_visitor(dynamic_visitor<Value, Parameters, Box, Allocators, node_d_mem_dynamic_tag, true> & v) const { v(*this); }

    elements_type elements;
};

// nodes traits

template <typename Value, typename Parameters, typename Box, typename Allocators>
struct node<Value, Parameters, Box, Allocators, node_d_mem_dynamic_tag>
{
    typedef dynamic_node<Value, Parameters, Box, Allocators, node_d_mem_dynamic_tag> type;
};

template <typename Value, typename Parameters, typename Box, typename Allocators>
struct internal_node<Value, Parameters, Box, Allocators, node_d_mem_dynamic_tag>
{
    typedef dynamic_internal_node<Value, Parameters, Box, Allocators, node_d_mem_dynamic_tag> type;
};

template <typename Value, typename Parameters, typename Box, typename Allocators>
struct leaf<Value, Parameters, Box, Allocators, node_d_mem_dynamic_tag>
{
    typedef dynamic_leaf<Value, Parameters, Box, Allocators, node_d_mem_dynamic_tag> type;
};

// visitor traits

template <typename Value, typename Parameters, typename Box, typename Allocators, bool IsVisitableConst>
struct visitor<Value, Parameters, Box, Allocators, node_d_mem_dynamic_tag, IsVisitableConst>
{
    typedef dynamic_visitor<Value, Parameters, Box, Allocators, node_d_mem_dynamic_tag, IsVisitableConst> type;
};

template <typename Element, typename Value, typename Translator>
struct translator_wrapper_helper
{
    typedef typename Element::first_type element_indexable_type;
    typedef typename Element::first_type const& element_indexable_result;
};

template <typename Value, typename Translator>
struct translator_wrapper_helper<Value, Value, Translator>
{
    typedef typename translator::indexable_type<Translator>::type element_indexable_type;
    typedef typename Translator::result_type element_indexable_result;
};

template <typename Value, typename Translator, typename Tag>
struct translator_wrapper
    : public Translator
{
    translator_wrapper(Translator const& t = Translator()) : Translator(t) {}

    template <typename Element>
    struct element_indexable_type
    {
        typedef typename translator_wrapper_helper<Element, Value, Translator>::element_indexable_type type;
    };

    template <typename Element>
    struct element_indexable_result
    {
        typedef typename translator_wrapper_helper<Element, Value, Translator>::element_indexable_result type;
    };

    typename element_indexable_result<Value>::type
    element_indexable(Value const& v) const { return Translator::operator()(v); }

    template <typename Element>
    typename element_indexable_result<Element>::type
    element_indexable(Element const& el) const { return el.first; }
};

// element's indexable type

template <typename Element, typename Translator>
struct element_indexable_type
{
    typedef typename Translator::template element_indexable_type<Element>::type type;
};

// element's indexable getter

template <typename Element, typename Translator>
typename Translator::template element_indexable_result<Element>::type
element_indexable(Element const& el, Translator const& tr)
{
    return tr.element_indexable(el);
}

// nodes elements

template <typename Node>
struct elements_type
{
    typedef typename Node::elements_type type;
};

template <typename Node>
inline typename elements_type<Node>::type &
elements(Node & n)
{
    return n.elements;
}

template <typename Node>
inline typename elements_type<Node>::type const&
elements(Node const& n)
{
    return n.elements;
}

// elements derived type
template <typename Elements, typename NewValue>
struct container_from_elements_type
{
    typedef boost::container::vector<NewValue> type;
};

// allocators

template <typename Allocator, typename Value, typename Parameters, typename Box>
class allocators<Allocator, Value, Parameters, Box, node_d_mem_dynamic_tag>
    : public Allocator::template rebind<
        typename internal_node<Value, Parameters, Box, allocators<Allocator, Value, Parameters, Box, node_d_mem_dynamic_tag>, node_d_mem_dynamic_tag>::type
    >::other
    , public Allocator::template rebind<
        typename leaf<Value, Parameters, Box, allocators<Allocator, Value, Parameters, Box, node_d_mem_dynamic_tag>, node_d_mem_dynamic_tag>::type
    >::other
{
public:
    typedef typename Allocator::size_type size_type;

    typedef typename Allocator::template rebind<
        typename node<Value, Parameters, Box, allocators, node_d_mem_dynamic_tag>::type
    >::other::pointer node_pointer;

    typedef typename Allocator::template rebind<
        typename internal_node<Value, Parameters, Box, allocators, node_d_mem_dynamic_tag>::type
    >::other::pointer internal_node_pointer;

    typedef typename Allocator::template rebind<
        typename internal_node<Value, Parameters, Box, allocators, node_d_mem_dynamic_tag>::type
    >::other internal_node_allocator_type;

    typedef typename Allocator::template rebind<
        typename leaf<Value, Parameters, Box, allocators, node_d_mem_dynamic_tag>::type
    >::other leaf_allocator_type;

    inline allocators()
        : internal_node_allocator_type()
        , leaf_allocator_type()
    {}

    inline explicit allocators(Allocator const& alloc)
        : internal_node_allocator_type(alloc)
        , leaf_allocator_type(alloc)
    {}

    inline allocators(BOOST_FWD_REF(allocators) a)
        : internal_node_allocator_type(boost::move(a.internal_node_allocator()))
        , leaf_allocator_type(boost::move(a.leaf_allocator()))
    {}

    void swap(allocators & a)
    {
        boost::swap(internal_node_allocator(), a.internal_node_allocator());
        boost::swap(leaf_allocator(), a.leaf_allocator());
    }

    bool operator==(allocators const& a) const { return leaf_allocator() == a.leaf_allocator(); }
    template <typename Alloc>
    bool operator==(Alloc const& a) const { return leaf_allocator() == leaf_allocator_type(a); }

    Allocator allocator() const { return Allocator(leaf_allocator()); }

    internal_node_allocator_type & internal_node_allocator() { return *this; }
    internal_node_allocator_type const& internal_node_allocator() const { return *this; }
    leaf_allocator_type & leaf_allocator() { return *this; }
    leaf_allocator_type const& leaf_allocator() const { return *this; }
};

// create_node_impl

template <typename BaseNodePtr, typename Node>
struct create_dynamic_node
{
    template <typename AllocNode>
    static inline BaseNodePtr apply(AllocNode & alloc_node)
    {
        typedef boost::container::allocator_traits<AllocNode> Al;
        typedef typename Al::pointer P;

        P p = Al::allocate(alloc_node, 1);

        if ( 0 == p )
            throw std::bad_alloc(); // TODO throw different exception

        auto_deallocator<AllocNode> deallocator(alloc_node, p);

        alloc_node.construct(p, Node(alloc_node));
        // TEMPORARILY COMMENTED OUT
        //Al::construct(alloc_node, p, alloc_node);

        deallocator.release();
        return p;
    }
};

// destroy_node_impl

template <typename Node>
struct destroy_dynamic_node
{
    template <typename AllocNode, typename BaseNodePtr>
    static inline void apply(AllocNode & alloc_node, BaseNodePtr n)
    {
        typedef boost::container::allocator_traits<AllocNode> Al;
        typedef typename Al::pointer P;

        P p(&static_cast<Node&>(rtree::get<Node>(*n)));
        alloc_node.destroy(p);
        // TEMPORARILY COMMENTED OUT
        //Al::destroy(alloc_node, p);
        Al::deallocate(alloc_node, p, 1);
    }
};

// create_node

template <typename Allocators, typename Value, typename Parameters, typename Box, typename Tag>
struct create_node<
    Allocators,
    dynamic_internal_node<Value, Parameters, Box, Allocators, Tag>
>
{
    static inline typename Allocators::node_pointer
    apply(Allocators & allocators)
    {
        return create_dynamic_node<
            typename Allocators::node_pointer,
            dynamic_internal_node<Value, Parameters, Box, Allocators, Tag>
        >::apply(allocators.internal_node_allocator());
    }
};

template <typename Allocators, typename Value, typename Parameters, typename Box, typename Tag>
struct create_node<
    Allocators,
    dynamic_leaf<Value, Parameters, Box, Allocators, Tag>
>
{
    static inline typename Allocators::node_pointer
    apply(Allocators & allocators)
    {
        return create_dynamic_node<
            typename Allocators::node_pointer,
            dynamic_leaf<Value, Parameters, Box, Allocators, Tag>
        >::apply(allocators.leaf_allocator());
    }
};

// destroy_node

template <typename Allocators, typename Value, typename Parameters, typename Box, typename Tag>
struct destroy_node<
    Allocators,
    dynamic_internal_node<Value, Parameters, Box, Allocators, Tag>
>
{
    static inline void apply(Allocators & allocators, typename Allocators::node_pointer n)
    {
        destroy_dynamic_node<
            dynamic_internal_node<Value, Parameters, Box, Allocators, Tag>
        >::apply(allocators.internal_node_allocator(), n);
    }
};

template <typename Allocators, typename Value, typename Parameters, typename Box, typename Tag>
struct destroy_node<
    Allocators,
    dynamic_leaf<Value, Parameters, Box, Allocators, Tag>
>
{
    static inline void apply(Allocators & allocators, typename Allocators::node_pointer n)
    {
        destroy_dynamic_node<
            dynamic_leaf<Value, Parameters, Box, Allocators, Tag>
        >::apply(allocators.leaf_allocator(), n);
    }
};

}} // namespace detail::rtree

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_INDEX_DETAIL_RTREE_NODE_NODE_DEFAULT_HPP
