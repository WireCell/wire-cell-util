#ifndef WIRECELL_SIGNAL
#define WIRECELL_SIGNAL

#include <boost/signals2.hpp>

#include <iostream>		// debug

namespace WireCell {

    /** A class with a signal which accepts a slot that produces an
     * instance of the Data type in a shared_ptr.  The Data type is
     * expected to inherit from IData.  The actual
     * boost::signals2::signal is held in a shared pointer so that any
     * subclass can be copyable but be aware that the signal is
     * shared.
     *
     * Subclass calls protected fire() to get next data object.
     */
    template <class Data>
    class Signal {
    public:
	
	typedef typename Data::pointer data_pointer;
	typedef boost::signals2::signal<data_pointer ()> signal;
	typedef std::shared_ptr<signal> signal_pointer;
	typedef typename signal::slot_type slot;

	/// Subclass must call in constructor
	Signal() : m_signal(new signal) {
	    //std::cerr << (void*)m_signal.get() << std::endl;
	}
	virtual ~Signal() {}

	void connect(const slot& s) { m_signal->connect(s); }

    protected:

	/// Subclass can call to get result of a signal
	data_pointer fire() { return *(*m_signal)(); }

    private:
	// no subclass access 
	signal_pointer m_signal;

    };

}

#endif
