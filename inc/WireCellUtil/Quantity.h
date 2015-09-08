#ifndef WIRECELL_QUANTITY
#define WIRECELL_QUANTITY

#include <cmath>
#include <ostream>

namespace WireCell {

    // This is heavily cribbed from Boost's units/example/measurement.hpp

    /** Provide a quantity with a value and an uncertainty and all the
     * little math operators. 
     *
     *  Beware this assumes all quantities are uncorrelated!
     */
    template<typename Value>
    class QuantityT {
    public:
	typedef QuantityT<Value>	this_type;
	typedef Value			value_type;

	QuantityT(const value_type& mean = value_type(), const value_type& sigma = value_type())
	    : m_mean(mean), m_sigma(sigma)
	{	}
	QuantityT(const QuantityT& other)
	    : m_mean(other.m_mean), m_sigma(other.m_sigma)
	{	}


	~QuantityT() {}

        this_type& operator=(const this_type& other)
        {
            if (this == &other) return *this;
            m_mean = other.m_mean;
            m_sigma = other.m_sigma;
            return *this;
        }

        operator value_type() const { return m_mean; }
        
        value_type mean() const { return m_mean; }
        value_type sigma() const { return m_sigma; }


        this_type& operator+=(const value_type& exact)
        {
            m_mean += exact;
            return *this;
        }
        
        this_type& operator-=(const value_type& exact)
        { 
            m_mean -= exact; 
            return *this; 
        }
        
        this_type& operator*=(const value_type& exact)
        { 
            m_mean *= exact; 
            m_sigma *= exact; 
            return *this; 
        }
        
        this_type& operator/=(const value_type& exact)            
        { 
            m_mean /= exact; 
            m_sigma /= exact; 
            return *this; 
        }
        

        this_type& operator*=(const this_type& other) {
	    // relative uncertainties
	    const value_type mrs = m_sigma/m_mean;
	    const value_type ors = other.m_sigma/other.m_mean;

	    m_mean *= other.m_mean;
	    m_sigma = m_mean * std::hypot(mrs, ors);

	    return *this;
	}

        this_type& operator/=(const this_type& other) {
	    // relative uncertainties
	    const value_type mrs = m_sigma/m_mean;
	    const value_type ors = other.m_sigma/other.m_mean;

	    m_mean /= other.m_mean;
	    m_sigma = m_mean * std::hypot(mrs, ors);

	    return *this;
	}

        this_type& operator-=(const this_type& other) {
	    m_mean -= other.m_mean;
	    m_sigma = std::hypot(m_sigma, other.m_sigma);
	    return *this;
	}
        this_type& operator+=(const this_type& other) {
	    m_mean += other.m_mean;
	    m_sigma = std::hypot(m_sigma, other.m_sigma);
	    return *this;
	}


	bool operator<(const this_type& other) const {
	    return m_mean < other.m_mean;
	}
	bool operator>(const this_type& other) const {
	    return m_mean > other.m_mean;
	}
	bool operator==(const this_type& other) const {
	    return m_mean == other.m_mean;
	}
	bool operator!=(const this_type& other) const {
	    return m_mean != other.m_mean;
	}

    private:

	value_type m_mean, m_sigma;
    };

    typedef QuantityT<double> Quantity;

} //namespace WireCell

inline std::ostream & operator<<(std::ostream &os, const WireCell::Quantity& q)
{
    os << "(" << q.mean() << " +/- " << q.sigma() << ")";
    return os;
}

inline WireCell::Quantity operator-(const WireCell::Quantity& other) 
{
    return WireCell::Quantity(-1*other.mean(), other.sigma());
}

inline WireCell::Quantity operator*(const WireCell::Quantity& lhs,
				    const WireCell::Quantity& rhs)
{
    WireCell::Quantity res = lhs;
    res *= rhs;
    return res;
}
inline WireCell::Quantity operator/(const WireCell::Quantity& lhs,
				    const WireCell::Quantity& rhs)
{
    WireCell::Quantity res = lhs;
    res /= rhs;
    return res;
}
inline WireCell::Quantity operator+(const WireCell::Quantity& lhs,
				    const WireCell::Quantity& rhs)
{
    WireCell::Quantity res = lhs;
    res += rhs;
    return res;
}
inline WireCell::Quantity operator-(const WireCell::Quantity& lhs,
				    const WireCell::Quantity& rhs)
{
    WireCell::Quantity res = lhs;
    res -= rhs;
    return res;
}

// some comparisons with other, scalar types

// fixme: *should* this be true:  should exact values be considered equal?
template<typename T>
WireCell::Quantity operator==(const WireCell::Quantity& lhs, const T& scalar)
{ return lhs.mean() == scalar; }

template<typename T>
WireCell::Quantity operator!=(const WireCell::Quantity& lhs, const T& scalar)
{ return lhs.mean() != scalar; }

template<typename T>
WireCell::Quantity operator<(const WireCell::Quantity& lhs, const T& scalar)
{ return lhs.mean() < scalar; }

template<typename T>
WireCell::Quantity operator>(const WireCell::Quantity& lhs, const T& scalar)
{ return lhs.mean() > scalar; }


template<typename T>
WireCell::Quantity operator==(const T& scalar, const WireCell::Quantity& rhs)
{ return scalar == rhs.mean(); }

template<typename T>
WireCell::Quantity operator!=(const T& scalar, const WireCell::Quantity& rhs)
{ return scalar != rhs.mean(); }

template<typename T>
WireCell::Quantity operator<(const T& scalar, const WireCell::Quantity& rhs)
{ return scalar < rhs.mean(); }

template<typename T>
WireCell::Quantity operator>(const T& scalar, const WireCell::Quantity& rhs)
{ return scalar > rhs.mean(); }


#endif
