#ifndef WIRECELL_FANINOUT
#define WIRECELL_FANINOUT

#include <boost/signals2.hpp>
#include <deque>
#include <map>

namespace WireCell {

    /** A fanout which takes in Data from a connected slot and buffers it
     * into set of addressable queues.
     */
    template<typename Data, typename Address = int>
    class Fanout {
    public:
	// The fanout is held as a map from address to queue
	typedef Data value_type;
	typedef Address address_type;
	typedef std::deque<value_type> data_queue;
	typedef std::map< int, data_queue > queue_map;

	// Our signal for providers of input data
	typedef boost::signals2::signal<value_type ()> signal;
	// The slot type our signal accepts
	typedef typename signal::slot_type slot;

	// Connect a slot to our signal.
	void connect(const slot& s) { m_signal.connect(s); }

	/// Register an address.
	void address(const address_type& addr) {
	    m_fan[addr] = data_queue();
	}

	/// Return a data from the given address.  If the address has not
	/// yet been registered it will be but any previously returned
	/// data will not be seen.
	value_type operator()(const address_type& addr) {
	    data_queue& dq = m_fan[addr];
	    if (!dq.size()) {
		value_type dat = *m_signal();
		for (auto it = m_fan.begin(); it != m_fan.end(); ++it) {
		    it->second.push_back(dat);
		}
	    }
	    value_type dat = dq.front();
	    dq.pop_front();
	    return dat;	
	}

    private:
	signal m_signal;
	queue_map m_fan;
    };

    // Fan-in concept is inherent in boost::signals2, but does require
    // some "combiner" to enact whatever fan-in policy is desired.
    // This most obvious one is one which synchronizes all input into
    // a simple collection.
    template<typename Collection>
    struct Fanin {
	typedef Collection result_type;
	typedef typename Collection::value_type value_type;

	template<typename InputIterator>
	result_type operator()(InputIterator first, InputIterator last) const {
	    result_type ret;
	    while (first != last) {
		ret.push_back(*first);
		++first;
	    }
	    return ret;
	}
    };

    
}


#endif
