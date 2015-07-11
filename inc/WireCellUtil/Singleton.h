#ifndef WIRECELL_SINGLETON
#define WIRECELL_SINGLETON

namespace WireCell {

    template <class T>
    class Singleton
    {
    public:
	static T& Instance() {
	    if (!instance) instance = new T;
	    return *instance;
	}

    private:
	Singleton();
	~Singleton();
	Singleton(Singleton const&);
	Singleton& operator=(Singleton const&);
	static T* instance;
    };

    template <class T> T* Singleton<T>::instance = 0;
}

#endif
