#ifndef WIRECELLUTIL_INDEXEDSET
#define WIRECELLUTIL_INDEXEDSET

#include <map>
#include <vector>

namespace WireCell {

    /** IndexedSet - maintain a collection of exactly one thing in a
     * first added order */

    template<class TYPE>
    class IndexedSet {
    public:
	// the passed objects in order of first seen
	std::map<TYPE, int> index;
	std::vector<TYPE> collection;

	int operator()(const TYPE& obj) {
	    auto mit = index.find(obj);
	    if (mit != index.end()) {
		return mit->second;
	    }
	    int ind = collection.size();
	    index[obj] = ind;
	    collection.push_back(obj);
	    return ind;
	}
    };

}

#endif
