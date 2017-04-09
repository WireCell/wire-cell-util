/** Persist structured information.
 *
 * Any and all structured information that needs to be loaded or saved
 * uses the methods in WireCell::Persist to do so.  The transient data
 * model is that of JsonCPP's Json::Value.  The persistent format is
 * either JSON or if the toolkit has support compiled in, Jsonnet.
 *
 * Note, "external" data which may be voluminous, complex or otherwise
 * inconvenient to convert to JSON would not use WireCell::Persist but
 * rather be brought between files and an IData model using an
 * ISink/ISource.
 *
 * See also WireCellUtil/Configuration.h for how the configuration
 * layer uses Json::Value objects.  Large configuration items like
 * wire geometry and field response are also loaded as JSON.
 */

#ifndef WIRECELL_PERSIST
#define WIRECELL_PERSIST

#include <json/json.h>


namespace WireCell {
    namespace Persist {

	/// Save the data structure held by the given top Json::Value
	/// in to a file of the given name.  The format of the file is
	/// determined by the file name extension.  Valid extensions
	/// are:
	///
	/// - .json :: JSON text format
	/// - .json.bz2 :: JSON text format compressed with bzip2
	///
	/// If pretty is true then format the JSON text.
	void dump(const std::string& filename, const Json::Value& top, bool pretty=false);
        /// As above but dump to a JSON text string.
        // fixme: no "pretty" for dumps() is implemented.
        std::string dumps(const Json::Value& top, bool pretty=false);

	/// Load a file and return the top JSON value.  See
	/// Persist::dump() for supported file formats.
	Json::Value load(const std::string& filename);
        /// As above but load from JSON text string.
	Json::Value loads(const std::string& text);
    }
}


#endif
