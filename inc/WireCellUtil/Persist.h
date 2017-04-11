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
	/// Persist::dump() for supported file formats.  In addition
	/// if filename ends in .jsonnet the file will be evaluated as
	/// Jsonnet.  Pure JSON can be evaluated resulting in a no-op
	/// but for very large JSON files this is best avoided.
	Json::Value load(const std::string& filename);

        /// As above but load from JSON text string.  Setting
        /// `isjsonnet` to true will enable Jsonnet evaluation.
	Json::Value loads(const std::string& text, bool isjsonnet=false);

        /// If Jsonnet support is compiled in, this will evaluate the
        /// input text as Jsonnet code.  If no support or if the text
        /// is already plain JSON then this method is an effective
        /// pass-through.  If support is built in and an error occurs
        /// the returned string will be empty and the Jsonnet
        /// evaluator will print an error.  This method may be called
        /// by load() and loads() if their `jsonnet=true` is passed.
        /// The JSONNET_PATH external environment variable can be set
        /// to let the evaluator find any imported jsonnet files.  If
        /// the filename is not set then any error messages will refer
        /// to "<stdin>".
        std::string evaluate_jsonnet(const std::string& text,
                                     const std::string& filename = "");
        
    }
}


#endif
