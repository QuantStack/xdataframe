/***************************************************************************
* Copyright (c) 2017, Johan Mabille and Sylvain Corlay                     *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XFRAME_XCOORDINATE_HPP
#define XFRAME_XCOORDINATE_HPP

#include <map>

#ifndef DEFAULT_LABEL_LIST
#include <cstddef>
#include "xtl/xbasic_fixed_string.hpp"
#include "xtl/xmeta_utils.hpp"
#define DEFAULT_LABEL_LIST xtl::mpl::vector<int,std::size_t, xtl::xfixed_string<55>>
#endif

#include "xaxis_variant.hpp"

namespace xf
{

    namespace detail
    {
        template <class S, class L>
        struct coordinate_axis;

        template <class S, template <class...> class L, class... T>
        struct coordinate_axis<S, L<T...>>
        {
            using type = xtl::variant<xaxis<T, S>...>;
        };

        template <class S, class L>
        using coordinate_axis_t = typename coordinate_axis<S, L>::type;
    }

    /***************
     * xcoordinate *
     ***************/

    template <class K, class S, class L = DEFAULT_LABEL_LIST>
    class xcoordinate
    {
    public:

        using self_type = xcoordinate<K, S, L>;
        using label_list = L;
        using axis_type = xaxis_variant<L, S>;
        using map_type = std::map<K, axis_type>;
        using key_type = typename map_type::key_type;
        using mapped_type = typename map_type::mapped_type;
        using index_type = S;
        using value_type = typename map_type::value_type;
        using reference = typename map_type::reference;
        using const_reference = typename map_type::const_reference;
        using pointer = typename map_type::pointer;
        using const_pointer = typename map_type::const_pointer;
        using size_type = typename map_type::size_type;
        using difference_type = typename map_type::difference_type;
        using iterator = typename map_type::iterator;
        using const_iterator = typename map_type::const_iterator;

        explicit xcoordinate(const map_type& axes);
        explicit xcoordinate(map_type&& axes);
        xcoordinate(std::initializer_list<value_type> init);
        template <class... LB>
        explicit xcoordinate(std::pair<K, xaxis<LB, S>>... axes);

        bool empty() const;
        size_type size() const;
        
        bool contains(const key_type& key) const;
        const mapped_type& operator[](const key_type& key) const;

        template <class KB, class LB>
        index_type operator[](const std::pair<KB, LB>& key) const;

        const map_type& data() const noexcept;

        const_iterator begin() const noexcept;
        const_iterator end() const noexcept;

        const_iterator cbegin() const noexcept;
        const_iterator cend() const noexcept;

        template <class... Args>
        std::pair<bool, bool> merge(const Args&... coordinates);

        template <class... Args>
        std::pair<bool, bool> intersect(const Args&... coordinates);

        bool operator==(const self_type& rhs) const noexcept;
        bool operator!=(const self_type& rhs) const noexcept;

    private:
        
        template <class LB1, class... LB>
        void insert_impl(std::pair<K, xaxis<LB1, S>> axis, std::pair<K, xaxis<LB, S>>... axes);
        void insert_impl();

        template <class... Args>
        std::pair<bool, bool> merge_impl(const self_type& c, const Args&... coordinates);
        std::pair<bool, bool> merge_impl();

        template <class... Args>
        std::pair<bool, bool> merge_empty(const self_type& c, const Args&... coordinates);
        std::pair<bool, bool> merge_empty();

        template <class... Args>
        std::pair<bool, bool> intersect_impl(const self_type& c, const Args&... coordinates);
        std::pair<bool, bool> intersect_impl();

        map_type m_coordinate;
    };

    template <class OS, class K, class S, class L>
    OS& operator<<(OS& out, const xcoordinate<K, S, L>& c);

    template <class K, class S, class L>
    xcoordinate<K, S> coordinate(const std::map<K, xaxis_variant<L, S>>& axes);

    template <class K, class S, class L>
    xcoordinate<K, S> coordinate(std::map<K, xaxis_variant<L, S>>&& axes);

    template <class K, class S, class... L>
    xcoordinate<K, S> coordinate(std::pair<K, xaxis<L, S>>... axes);

    template <class K, class S, class L, class... Args>
    std::pair<bool, bool> merge_coordinates(xcoordinate<K, S, L>& output, const Args&... coordinates);

    template <class K, class S, class L, class... Args>
    std::pair<bool, bool> intersect_coordinates(xcoordinate<K, S, L>& output, const Args&... coordinates);

    /******************************
     * is_coordinate_metafunction *
     ******************************/

    namespace detail
    {
        template <class T>
        struct is_coordinate_impl : std::false_type
        {
        };

        template <class K, class S, class L>
        struct is_coordinate_impl<xcoordinate<K, S, L>> : std::true_type
        {
        };
    }

    template <class T>
    struct is_coordinate : detail::is_coordinate_impl<std::decay_t<T>>
    {
    };

    /******************************
     * xcoordinate implementation *
     ******************************/

    template <class K, class S, class L>
    xcoordinate<K, S, L>::xcoordinate(const map_type& axes)
        : m_coordinate(axes)
    {
    }

    template <class K, class S, class L>
    xcoordinate<K, S, L>::xcoordinate(map_type&& axes)
        : m_coordinate(std::move(axes))
    {
    }

    template <class K, class S, class L>
    xcoordinate<K, S, L>::xcoordinate(std::initializer_list<value_type> init)
        : m_coordinate(init)
    {
    }

    template <class K, class S, class L>
    template <class... LB>
    inline xcoordinate<K, S, L>::xcoordinate(std::pair<K, xaxis<LB, S>>... axes)
    {
        insert_impl(std::move(axes)...);
    }
    
    template <class K, class S, class L>
    inline bool xcoordinate<K, S, L>::empty() const
    {
        return m_coordinate.empty();
    }

    template <class K, class S, class L>
    inline auto xcoordinate<K, S, L>::size() const -> size_type
    {
        return m_coordinate.size();
    }

    template <class K, class S, class L>
    inline bool xcoordinate<K, S, L>::contains(const key_type& key) const
    {
        return m_coordinate.find(key) != m_coordinate.end();
    }

    template <class K, class S, class L>
    inline auto xcoordinate<K, S, L>::operator[](const key_type& key) const -> const mapped_type&
    {
        return m_coordinate.at(key);
    }

    template <class K, class S, class L>
    template <class KB, class LB>
    inline auto xcoordinate<K, S, L>::operator[](const std::pair<KB, LB>& key) const -> index_type
    {
        return (*this)[key.first][key.second];
    }

    template <class K, class S, class L>
    inline auto xcoordinate<K, S, L>::data() const noexcept -> const map_type&
    {
        return m_coordinate;
    }

    template <class K, class S, class L>
    inline auto xcoordinate<K, S, L>::begin() const noexcept -> const_iterator
    {
        return cbegin();
    }

    template <class K, class S, class L>
    inline auto xcoordinate<K, S, L>::end() const noexcept -> const_iterator
    {
        return cend();
    }

    template <class K, class S, class L>
    inline auto xcoordinate<K, S, L>::cbegin() const noexcept -> const_iterator
    {
        return m_coordinate.cbegin();
    }

    template <class K, class S, class L>
    inline auto xcoordinate<K, S, L>::cend() const noexcept -> const_iterator
    {
        return m_coordinate.cend();
    }
    
    template <class K, class S, class L>
    template <class... Args>
    inline std::pair<bool, bool> xcoordinate<K, S, L>::merge(const Args&... coordinates)
    {
        return empty() ? merge_empty(coordinates...) : merge_impl(coordinates...);
    }

    template <class K, class S, class L>
    template <class... Args>
    inline std::pair<bool, bool> xcoordinate<K, S, L>::intersect(const Args&... coordinates)
    {
        return empty() ? std::pair<bool, bool>({ false, false }) : intersect_impl(coordinates...);
    }

    template <class K, class S, class L>
    inline bool xcoordinate<K, S, L>::operator==(const self_type& rhs) const noexcept
    {
        return m_coordinate == rhs.m_coordinate;
    }

    template <class K, class S, class L>
    inline bool xcoordinate<K, S, L>::operator!=(const self_type& rhs) const noexcept
    {
        return !(*this == rhs);
    }

    template <class K, class S, class L>
    template <class LB1, class... LB>
    inline void xcoordinate<K, S, L>::insert_impl(std::pair<K, xaxis<LB1, S>> axis, std::pair<K, xaxis<LB, S>>... axes)
    {
        m_coordinate.insert(value_type(std::move(axis.first), std::move(axis.second)));
        insert_impl(std::move(axes)...);
    }

    template <class K, class S, class L>
    inline void xcoordinate<K, S, L>::insert_impl()
    {
    }

    template <class K, class S, class L>
    template <class... Args>
    inline std::pair<bool, bool> xcoordinate<K, S, L>::merge_impl(const self_type& c, const Args&... coordinates)
    {
        auto res = merge_impl(coordinates...);
        for(auto iter = c.begin(); iter != c.end(); ++iter)
        {
            auto inserted = m_coordinate.insert(*iter);
            const auto& key = inserted.first->first;
            auto& axis = inserted.first->second;
            if(inserted.second)
            {
                res.first = false;
            }
            else
            {
                res.second &= axis.merge(c[key]);
            } 
        }
        return res;
    }

    template <class K, class S, class L>
    inline std::pair<bool, bool> xcoordinate<K, S, L>::merge_impl()
    {
        return { true, true };
    }

    template <class K, class S, class L>
    template <class... Args>
    inline std::pair<bool, bool> xcoordinate<K, S, L>::merge_empty(const self_type& c, const Args&... coordinates)
    {
        m_coordinate = c.m_coordinate;
        return merge_impl(coordinates...);
    }

    template <class K, class S, class L>
    inline std::pair<bool, bool> xcoordinate<K, S, L>::merge_empty()
    {
        return merge_impl();
    }

    template <class K, class S, class L>
    template <class... Args>
    inline std::pair<bool, bool> xcoordinate<K, S, L>::intersect_impl(const self_type& c, const Args&... coordinates)
    {
        auto res = intersect_impl(coordinates...);
        res.first &= (size() == c.size());
        for(auto iter = m_coordinate.begin(); iter != m_coordinate.end(); ++iter)
        {
            auto citer = c.data().find(iter->first);
            if(citer == c.data().end())
            {
                iter = m_coordinate.erase(iter);
                res.first = false;
            }
            else
            {
                res.second &= (iter->second).intersect(c[iter->first]);
            }
        }
        return res;
    }

    template <class K, class S, class L>
    inline std::pair<bool, bool> xcoordinate<K, S, L>::intersect_impl()
    {
        return { true, true };
    }

    template <class OS, class K, class S, class L>
    inline OS& operator<<(OS& out, const xcoordinate<K, S, L>& c)
    {
        for(auto& v: c)
        {
            out << v.first << ": " << v.second << std::endl;
        }
        return out;
    }

    template <class K, class S, class L>
    xcoordinate<K, S> coordinate(const std::map<K, xaxis_variant<L, S>>& axes)
    {
        return xcoordinate<K, S>(axes);
    }

    template <class K, class S, class L>
    xcoordinate<K, S> coordinate(std::map<K, xaxis_variant<L, S>>&& axes)
    {
        return xcoordinate<K, S>(std::move(axes));
    }

    template <class K, class S, class... L>
    inline xcoordinate<K, S> coordinate(std::pair<K, xaxis<L, S>>... axes)
    {
        return xcoordinate<K, S>(std::move(axes)...);
    }

    template <class K, class S, class L, class... Args>
    inline std::pair<bool, bool> merge_coordinates(xcoordinate<K, S, L>& output, const Args&... coordinates)
    {
        return output.merge(coordinates...);
    }

    template <class K, class S, class L, class... Args>
    inline std::pair<bool, bool> intersect_coordinates(xcoordinate<K, S, L>& output, const Args&... coordinates)
    {
        return output.intersect(coordinates...);
    }
}

#endif
