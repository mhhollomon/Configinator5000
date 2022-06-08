// Grammar is here : https://hyperrealm.github.io/libconfig/libconfig_manual.html#Configuration-File-Grammar
//

#include <memory>
#include <string>
#include <map>
#include <vector>
#include <exception>
#include <sstream>
#include <fstream>

#include <type_traits>

using namespace std::literals::string_literals;

namespace Configinator5000 {

    // Forward Declaration
    class Parser;

    // These make up the schema tree
    // that is consulted for the rules
    class SchemaNode {
    };

    // These make up the Config Tree that
    // we give to the user.
    class Setting {
    public :
        enum class setting_type { STRING, BOOL, INTEGER, FLOAT, GROUP, LIST, ARRAY };

    private :
        friend class Config;
        friend class Parser;

        setting_type type_;

        // scalar containers (maybe change to std::variant?)
        long integer_;
        double float_;
        bool bool_;
        std::string string_;


        // container for the composite types
        std::vector<Setting> children_;

        // lookup for groups. Allows at(std::string) to be O(1)
        std::map<std::string, int> group_;

        // Arrays must all be the same type. Set when the first child is added to the array.
        setting_type array_type_ = setting_type::BOOL;

        void clear_subobjects() {
            children_.clear();
            group_.clear();
            string_.clear();
        }

        template<class T>
        setting_type deduce_scalar_type(T v) {
            if constexpr (std::is_same_v<T, bool>) {
                return setting_type::BOOL;
            } else if constexpr (std::is_convertible_v<std::string, T>) {
                return setting_type::STRING;
            } else if constexpr (std::is_integral_v<T>) {
                return setting_type::INTEGER;
            } else if constexpr(std::is_floating_point_v<T>) {
                return setting_type::FLOAT;
            } else {
                throw std::runtime_error("Could not deduce setting type from value");
            }
        }

        static constexpr bool is_scalar_type(setting_type t) {
            using st = setting_type;
            return (t == st::INTEGER or t == st::FLOAT or t == st::BOOL or t == st::STRING);
        }

        static constexpr bool is_composite_type(setting_type t) {
            using st = setting_type;
            return (t == st::GROUP or t == st::LIST or t == st::ARRAY);
        }
        

        /* Create a new named child and return a reference to it */
        Setting * create_child(const std::string &name) {
            if (!is_group()) throw std::runtime_error(
                    "Must be a group to add a named child");

            auto [ iter, done ] = group_.try_emplace(name, group_.size());

            if (done) {
                // It didn't exists before
                auto & new_ref = children_.emplace_back();
                return &new_ref;
            } else {
                return nullptr;
            }

        }

        Setting *create_child() {
            if (is_list() or is_array()) {
                return &(children_.emplace_back());
            }
            throw std::runtime_error("Wrong setting type for create_child()");
        }
    public :
        Setting(setting_type t = setting_type::BOOL) : type_{t} {}

        Setting(bool b) : type_(setting_type::BOOL), bool_(b) {}
        Setting(int i) : type_(setting_type::INTEGER), integer_(i) {}
        Setting(long l) : type_(setting_type::INTEGER), integer_(l) {}
        Setting(double f) :  type_(setting_type::FLOAT), float_(f) {}
        Setting(std::string s) : type_(setting_type::STRING), string_(s) {}
        Setting(const char * c) : type_(setting_type::STRING), string_(c) {}

        Setting & set_value(bool b) {
            if (!is_boolean()) {
                clear_subobjects();
                type_ = setting_type::BOOL;
            }
            bool_ = b;
            return *this;
        }

        Setting & set_value(int i) {
            if (!is_integer()) {
                clear_subobjects();
                type_ = setting_type::INTEGER;
            }
            integer_ = i;
            return *this;
        }

        Setting & set_value(long i) {
            if (!is_integer()) {
                clear_subobjects();
                type_ = setting_type::INTEGER;
            }
            integer_ = i;
            return *this;
        }
        
        Setting & set_value(double f) {
            if (!is_float()) {
                clear_subobjects();
                type_ = setting_type::FLOAT;
            }
            float_ = f;
            return *this;
        }


        Setting & set_value(const std::string s) {
            if (!is_string()) {
                clear_subobjects();
                type_ = setting_type::STRING;
            }
            string_ = s;
            return *this;
        }

        Setting & set_value(const char *c) {
            if (!is_string()) {
                clear_subobjects();
                type_ = setting_type::STRING;
            }
            string_ = c;
            return *this;
        }

        bool is_boolean() const { return (type_ == setting_type::BOOL); }
        bool is_integer() const { return (type_ == setting_type::INTEGER); }
        bool is_float()   const { return (type_ == setting_type::FLOAT); }
        bool is_string()  const { return (type_ == setting_type::STRING); }
        bool is_group()   const { return (type_ == setting_type::GROUP); }
        bool is_list()    const { return (type_ == setting_type::LIST); }
        bool is_array()   const { return (type_ == setting_type::ARRAY); }

        bool is_numeric()   const { return (is_integer() || is_float()); }
        bool is_composite() const { return (is_group() || is_list() || is_array()); }
        bool is_scalar()    const { return (is_scalar_type(type_)); }

        void make_list() {
            if (!is_list()) {
                clear_subobjects();
                type_ = setting_type::LIST;
            }
        }

        void make_group() {
            if (!is_group()) {
                clear_subobjects();
                type_ = setting_type::GROUP;
            }
        }

        void make_array() {
            if (! is_array()) {
                clear_subobjects();
                type_ = setting_type::ARRAY;
            }
        }

        bool exists(const std::string child) const {
            if (! is_group()) return false;

            auto const &iter = group_.find(child);
            if (iter == group_.end()) 
                return false;
            else
                return true;
        }

        template<class T> T get() const {
            if constexpr (std::is_same_v<T, bool>) {
                if (is_boolean()) {
                    return T(bool_);
                } else {
                    throw std::runtime_error("Bad type conversion\n");
                }
            } else if constexpr (std::is_integral_v<T>) {
                if (is_integer())
                    return T(integer_);
                else 
                    throw std::runtime_error("Bad type conversion\n");

            } else if constexpr(std::is_floating_point_v<T>) {
                if (is_float()) {
                    return T(float_);
                } else if (is_integer()) {
                    return T(integer_);
                } else {
                    throw std::runtime_error("Bad type conversion\n");
                }
            } else if constexpr (std::is_convertible_v<std::string, T>) {
                if (is_string()) {
                    return T(string_);
                } else {
                    throw std::runtime_error("Bad type conversion\n");
                }
            } else {
                throw std::runtime_error("Bad type conversion (not a scalar)\n");
            }

        }

        
        template<class T>
        Setting &add_child(T v) {

            if (is_group()) {
                throw std::runtime_error("Group children must have names");

            } else if (is_array()) {
                setting_type target_type = deduce_scalar_type(v);
                if (children_.size() > 0) {
                    if (array_type_ != target_type) {
                        throw std::runtime_error("All children of arrays must be the same type");
                    }
                } else {
                    array_type_ = target_type;
                }
                return children_.emplace_back(v);

            } else if (is_list()) {
                return children_.emplace_back(v);

            } else {
                throw std::runtime_error("Setting must be composite to add child");
            }
        }

        Setting &add_child(setting_type t) {
            if (is_group()) {
                throw std::runtime_error("Group children must have names");

            } else if (is_list()) {
                return children_.emplace_back(t);

            } else if (is_array()) {
                if (is_composite_type(t)) {
                    throw std::runtime_error("Arrays may only have scalar children");
                }

                if (children_.size() > 0) {
                    if (array_type_ != t) {
                        throw std::runtime_error("All children of arrays must be the same type");
                    }
                } else {
                    array_type_ = t;
                }
                return children_.emplace_back(t);
            } else {
                throw std::runtime_error("Setting must be composite to add child");
            }
        }
        
        template<class T>
        Setting &add_child( const std::string &name, T v) {

            if (!is_group()) {
                throw std::runtime_error("Only group children may have names");
            }

            auto [ _, done ] = group_.try_emplace(name, group_.size());

            if (done) {
                // It didn't exists before
                return children_.emplace_back(v);
            } else {
                throw std::runtime_error("Child with given key "s + name + " already exists");

            }
        }

        Setting &add_child( const std::string &name, setting_type t) {

            if (!is_group()) {
                throw std::runtime_error("Only group children may have names");
            }

            auto [ _, done ] = group_.try_emplace(name, group_.size());

            if (done) {
                // It didn't exists before
                return children_.emplace_back(t);
            } else {
                throw std::runtime_error("Child with given key "s + name + " already exists");

            }
        }

        int count() const {
            if (is_scalar()) {
                return 0;
            }

            return int(children_.size());
        }

        Setting &at(int idx) {
            if (! is_composite()) {
                throw std::runtime_error("at(int) called on a non-composite");
            }

            // negative indecies count from the end
            //   0    1   2
            //   -    -   -
            //  -3   -2  -1
            if (idx >= int(children_.size()) or idx < -int(children_.size())) {
                throw std::runtime_error("at(int) called with index out of range");
            }

            if (idx < 0) {
                idx += int(children_.size());
            }

            return children_.at(idx);
        }

        Setting &at(std::string name) {
            if (!is_group()) {
                throw std::runtime_error("at(string) called on a non-group");
            }

            auto iter = group_.find(name);
            if (iter == group_.end()) {
                throw std::runtime_error("at(string) : key "s + name + " does not exist in the group");
            }

            return children_.at(iter->second);
        }


    };

    class Config {
        std::unique_ptr<SchemaNode>schema_tree_;
        std::unique_ptr<Setting>cfg_;
    public :
        bool parse_file(std::string file_name) {
            std::ifstream strm{file_name};
            return parse(strm);
        }
        bool parse(std::ifstream &strm) {
            // too many copies, but shouldn't matter much.
            std::stringstream buffer;
            buffer << strm.rdbuf();
            return parse(buffer.str());
        }

        bool parse(const std::string &input) {
            return parse_with_schema(input, schema_tree_.get());
        }

    private:
        bool parse_with_schema(const std::string &input, const SchemaNode *schema);
    };


} // end namespace Configinator5000
