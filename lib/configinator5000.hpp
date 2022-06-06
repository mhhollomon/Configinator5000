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

        std::string name_;
        setting_type type_;
        long integer_;
        double float_;
        bool bool_;
        std::string string_;
        std::map<std::string, Setting> group_;
        std::vector<Setting> list_array_;

        void clear_composites() {
            if (is_group()) {
                group_.clear();
            } else if (is_string()) {
                string_.clear();
            } else if (is_list() or is_array()) {
                list_array_.clear();

            }
        }
        Setting(std::string n, setting_type t = setting_type::BOOL) : 
            name_{n}, type_{t} {}

        Setting & add_child(const std::string &name) {
            if (!is_group()) throw std::runtime_error(
                    "Must be a group to add a named child");

            auto [ iter, done ] = group_.emplace(name, Setting(name));

            return iter->second;
        }

        Setting &add_child() {
            if (is_list() or is_array()) {
                auto [iter,done] = list_array_.emplace(Setting());
                return *iter;
            }
            throw std::runtime_error("Wrong setting type for add child");
        }
    public :
        Setting(std::string n, int i) : name_(n), type_(setting_type::INTEGER), integer_(i) {}
        Setting(std::string n, double f) :  name_(n), type_(setting_type::FLOAT), float_(f) {}
        
        bool is_boolean() const { return (type_ == setting_type::BOOL); }
        bool is_integer() const { return (type_ == setting_type::INTEGER); }
        bool is_float()   const { return (type_ == setting_type::FLOAT); }
        bool is_string()  const { return (type_ == setting_type::STRING); }
        bool is_group()   const { return (type_ == setting_type::GROUP); }
        bool is_list()    const { return (type_ == setting_type::LIST); }
        bool is_array()   const { return (type_ == setting_type::ARRAY); }

        bool is_numeric()   const { return (is_integer() || is_float()); }
        bool is_composite() const { return (is_group() || is_list() || is_array()); }
        bool is_scalar()    const { return (! is_composite()); }

        void make_list() {
            clear_composites();
            type_ = setting_type::LIST;
        }

        void make_group() {
            clear_composites();
            type_ = setting_type::GROUP;
        }

        void make_array() {
            clear_composites();
            type_ = setting_type::ARRAY;
        }

        bool exists(std::string child) const {
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
                throw std::runtime_error("Bad type conversion\n");
            }

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
