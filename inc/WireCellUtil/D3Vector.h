/** Vector a 3-vector of double.
 *
 * See also WireCell::Point.
 */

#ifndef WIRECELLDATA_VECTOR
#define WIRECELLDATA_VECTOR

#include <iostream>
#include <cmath>
#include <algorithm>
 
namespace WireCell {

    /** Dimension-3 vector class.
     *
     * Adapted in laziness from:
     * http://rosettacode.org/wiki/Vector_products#C.2B.2B
     *
     */
    template< class T >
    class D3Vector {
 
	template< class U >
	friend std::ostream & operator<<( std::ostream & , const D3Vector<U> & ) ;   
 
    public :
	T x , y , z, bogus ;  

	D3Vector( T a=0, T b=0, T c=0) : x(a), y(b), z(c) { }
	// copy constructor
	D3Vector( const D3Vector& o) : x(o.x), y(o.y), z(o.z) { }
	// assignment
	D3Vector& operator=(const D3Vector& o) {
	    x = o.x; y = o.y, z = o.z;
	    return *this;
	}
	// compare
	
	// convert
	template< class TT >
	D3Vector( const D3Vector<TT>& o) : x(o.x), y(o.y), z(o.z) { }
 
	T operator[](std::size_t index) const {
	    if (index == 0) return x;
	    if (index == 1) return y;
	    if (index == 2) return z;
	    return bogus;
	}
	T& operator[](std::size_t index) {
	    if (index == 0) return x;
	    if (index == 1) return y;
	    if (index == 2) return z;
	    return bogus;
	}
	    
	/// Return the vector sum of this vector and the other.
	D3Vector<T> sum( const D3Vector & rhs ) const {
	    return D3Vector(x+rhs.x, y+rhs.y, z+rhs.z);
	}

	/// Return the dot product of this vector and the other.
	T dot ( const D3Vector & rhs ) const {
	    T scalar = x * rhs.x + y * rhs.y + z * rhs.z ;
	    return scalar;
	}
 
	/// Return the magnitude of this vector.
	T magnitude() const {
	    return std::sqrt(x*x+y*y+z*z);
	}

	/// Return a normalized vector in the direction of this vector.
	D3Vector norm() const {
	    T m = this->magnitude();
	    if (m <= 0) {
		return D3Vector();
	    }
	    return D3Vector(x/m, y/m, z/m);
	}	    

	/// Return the cross product of this vector and the other.
	D3Vector cross ( const D3Vector & rhs ) const {
	    T a = y * rhs.z - z * rhs.y ;
	    T b = z * rhs.x - x * rhs.z ;
	    T c = x * rhs.y - y * rhs.x ;
	    D3Vector product( a , b , c ) ;
	    return product ;
	}
 
	/// Return the triple cross product of this vector and the other two.
	D3Vector triplevec( D3Vector & a , D3Vector & b ) const {
	    return cross ( a.cross( b ) ) ;
	}
	/// Return the dot-cross product of this vector and the other two. 
	T triplescal( D3Vector & a, D3Vector & b ) const {
	    return dot( a.cross( b ) ) ;
	}
 
	bool operator< (const D3Vector& rhs) const {
	    if (z < rhs.z) return true;
	    if (y < rhs.y) return true;
	    if (x < rhs.x) return true;
	    return false;
	}


    } ;
 
    template< class T >
    std::ostream & operator<< ( std::ostream & os , const D3Vector<T> & vec ) {
	os << "("  << vec.x << " " << vec.y << " " << vec.z << ")" ;
	return os ;
    }

    template< class T >
    D3Vector<T> operator-(const D3Vector<T> a, const D3Vector<T> b) {
	return D3Vector<T>(a.x-b.x, a.y-b.y, a.z-b.z);
    }

    template< class T >
    D3Vector<T> operator+(const D3Vector<T> a, const D3Vector<T> b) {
	return D3Vector<T>(a.x+b.x, a.y+b.y, a.z+b.z);
    }

    template< class T >
    D3Vector<T> operator*(const D3Vector<T> a, T s) {
	return D3Vector<T>(a.x*s, a.y*s, a.z*s);
    }
    
    template< class T >
    bool operator==(const D3Vector<T>& a,const D3Vector<T>& b){
      if (a.x==b.x&& a.y==b.y && a.z==b.z){
	return true;
      }else{
	return false;
      }
    }
    
    template< class T >
    D3Vector<T> operator*(T s, const D3Vector<T> a) {
	return a*s;
    }


//    template< class T >
//    std::pair< float, float >
//    box_interesect(const D3Vector<T>& minbound, const D3Vector<T>& maxbound,
//		   const D3Vector<T>& point, const D3Vector<T>& ray)


} // namespace WireCell

#endif
