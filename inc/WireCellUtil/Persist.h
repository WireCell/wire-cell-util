#ifndef WIRECELL_PERSIST
#define WIRECELL_PERSIST

#include <jsoncpp/json/json.h>


namespace WireCell {
    namespace Persist {

	/// Save the data structure held by the given top Json::Value
	/// in to a file of the given name.  The format of the file is
	/// determined by the file name extension.  Valid extensions
	/// are:
	///
	/// - .json :: JSON text format
	/// - .json.bz1 :: JSON text format compressed with bzip2
	///
	/// If pretty is true then format the JSON text.
	void dump(const char* filename, Json::Value& top, bool pretty=false);

	/// Load a file and return the top JSON value.  See
	/// Persist::dump() for supported file formats.
	Json::Value load(const char* filename);

    }
}


#endif
