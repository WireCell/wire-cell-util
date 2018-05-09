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
#include "libjsonnet++.h"
#include <boost/filesystem.hpp>
#include <vector>
#include <string>

namespace WireCell {
    namespace Persist {

        /// Return true file exists (no file resolution performed).
        bool exists(const std::string& filename);


        /** Return full path to a file of the given filename.  If the
         * file is not directly located and is a relative path then
         * the file will be first located in the current working
         * directory.  Failing that if the `WIRECELL_PATH` environment
         * variable is defined and set as a `:`-separated list it will
         * be checked. Failure to resolve returns an empty string. */
        std::string resolve(const std::string& filename);

        /** Return a string holding the entire contents of the file.
         * File resolution is performed.  WireCell::IOError is thrown
         * if file is not found. */
        std::string slurp(const std::string& filename);


	/// Save the data structure held by the given top Json::Value
	/// in to a file of the given name.  The format of the file is
	/// determined by the file name extension.  Valid extensions
	/// are:
	///
	/// - .json :: JSON text format
	/// - .json.bz2 :: JSON text format compressed with bzip2
	///
	/// If `pretty` is true then format the JSON text with
	/// indents.  If also compressed, this formatting can actually
	/// lead to *smaller* files.
	void dump(const std::string& filename, const Json::Value& top, bool pretty=false);

        /// As above but dump to a JSON text string.
        // fixme: no "pretty" for dumps() is implemented.
        std::string dumps(const Json::Value& top, bool pretty=false);

        /// This can hold either Jsonnet external variable/value pairs
        /// The value is a string representation of a simple scalar
        /// value, or a data structure if used as code.  This type may
        /// also be used to hold JSON path/value pairs.
        typedef std::map<std::string, std::string> externalvars_t;

	/** Load a file and return the top JSON value.  
         
            If extension is `.jsonnet` and Jsonnet support is compiled
            in, evaluate the file and use the resulting JSON.  Other
            supported extensions include raw (`.json`) or compressed
            (`.json.bz2`) files.  

            WireCell::IOError is thrown if file is not found.
        */
        Json::Value load(const std::string& filename,
                         const externalvars_t& extvar = externalvars_t(),
                         const externalvars_t& extcode = externalvars_t());

        /** Load a JSON or Jsonnet string, returning a Json::Value. */
	Json::Value loads(const std::string& text,
                          const externalvars_t& extvar = externalvars_t(),
                          const externalvars_t& extcode = externalvars_t());

        /** Explicitly evaluate contents of file with Jsonnet.  If no
            support for Jsonnet is built, return the contents of
            file.  Return empty string if Jsonnet evaluation failes. 

            WireCell::IOError is thrown if file is not found.
            WireCell::ValueError is thrown parsing fails.
        */
        std::string evaluate_jsonnet_file(const std::string& filename,
                                          const externalvars_t& extvar = externalvars_t(),
                                          const externalvars_t& extcode = externalvars_t());

        /** Explicitly evaluate text with JSonnet.  If no support for
            Jsonnet is built, return the text. Return empty string if
            Jsonnet evaluation failes. 

            WireCell::ValueError is thrown parsing fails.            
        */
        std::string evaluate_jsonnet_text(const std::string& text,
                                          const externalvars_t& extvar = externalvars_t(),
                                          const externalvars_t& extcode = externalvars_t());

        /** Explicitly convert JSON text to Json::Value object */
        Json::Value json2object(const std::string& text);
        

        /** Convert a collection to a Json::Value */
        // not really about persistence....
        template <typename Iterable>
        Json::Value iterable2json(Iterable const& cont) {
            Json::Value v;
            for (auto&& element: cont) {
                v.append(element);
            }
            return v;
        }

        // This provides a super set of the functionality as the above
        // free functions.  It wraps a persistent Jsonnet parser and
        // in particular allows more control over the load path. 
        class Parser {
        public:
            typedef std::vector<std::string> pathlist_t;

            Parser(const pathlist_t& load_paths = pathlist_t(),
                   const externalvars_t& extvar = externalvars_t(),
                   const externalvars_t& extcode = externalvars_t());

            // Load a Jonnet file (or .json or .json.bz2) and return the Json object
            Json::Value load(const std::string& filename);

            // Load Jsonnet text and return the Json object
            Json::Value loads(const std::string& text);

            // Resolve absolute path to a file against load path
            std::string resolve(const std::string& filename);

        private:
            jsonnet::Jsonnet m_jsonnet;
            std::vector<boost::filesystem::path> m_load_paths;

        };
    }
}


#endif
