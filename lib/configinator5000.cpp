#include <configinator5000.hpp>

#include <string_view>
#include <list>
#include <charconv>
#include <sstream>


namespace Configinator5000 {

    struct parse_loc {
        std::string_view sv;
        int offset = 0;
        int line = 0;
    };

    struct error_info {
        std::string message;
        parse_loc loc;

        error_info() = default;
        error_info(const std::string &mesg, const parse_loc &l) :
            message{mesg}, loc{l} {}

        friend std::ostream & operator<<(std::ostream &strm, const error_info& ei) {
            strm << "line " << ei.loc.line << " : " << ei.message << "\n";
            return strm;
        }
    };

    struct error_list {
        std::string_view src;
        std::list<error_info> errors;

        int count() const {
            return static_cast<int>(errors.size());
        }

        bool empty() const { return errors.empty(); }

        void add(const std::string &mesg, const parse_loc &l) {
            errors.emplace_back(mesg, l);
        }

        friend std::ostream & operator<<(std::ostream &strm, const error_list & el){
            for (auto &e : el.errors) {
                strm << e;
            }
            return strm;
        }
    };

    struct Parser {

        std::string_view src;

        Setting *setting;

        parse_loc current_loc;

        error_list errors;

        Parser(std::string_view _src, Setting *s) : src{_src}, setting{s},
            current_loc{_src, 0, 0} {}

        /***********************************************************
         * Error utilities
         ***********************************************************/

        void record_error(const std::string &msg, const parse_loc &l) {
            errors.add(msg, l);
        }

        void record_error(const std::string & msg) {
            record_error(msg, current_loc);
        }

        /***********************************************************
         * Input utilities
         ***********************************************************/

        inline void consume(int count) {
            // note : others are responsible for line number
            current_loc.offset += count;
            current_loc.sv.remove_prefix(count);
        }

        inline bool eoi() { return current_loc.sv.size() == 0; }

        inline char peek() { if (!eoi()) { return current_loc.sv[0]; }
            else { return '\x00'; } }

        inline char peek(int pos) { return current_loc.sv[pos]; }

        inline bool valid_pos(int pos) {
            return (pos >= 0 and (unsigned)pos < current_loc.sv.size()); 
        }


        inline bool check_string(std::string_view o) {
            return current_loc.sv.compare(0, o.size(), o) == 0;
        }
        
        inline bool match_string(std::string_view o) {
            bool matched = check_string(o);
            if (matched) consume(o.size());
            return matched;
        }

        /****************************************************************
        * SKIP Processing
        ****************************************************************/
        bool skip() {

            //
            // States for the state machine
            //
            enum SKIP_STATE { skNormal, skLine, skBlock };

            SKIP_STATE state = skNormal;

            //
            // we'll be using this a lot.
            // So abbreviate it with a reference.
            //
            auto &cl = current_loc;

            //
            // When we enter a comment, we'll record where it started.
            // If, at the end, we're still looking for comment terminator,
            // we can use this location to put out an error message.
            //
            parse_loc comment_loc;

            bool skipping = true;
            int line_count = 0;
            while (skipping and not eoi()) {

                //std::cout << "Loop top pos = " << cl.offset <<
                //    " state = " << state <<
                //    " char = '" << peek() << "'\n";
                switch (state) {
                    case skNormal : // Normal state
                        if (peek(0) == '\n') {
                            line_count += 1;
                            consume(1);
                        } else if (std::isspace(peek(0))) {
                            consume(1);
                        } else if (peek(0) == '#') {
                            //std::cout << "--- Saw hash comment start\n";
                            comment_loc = cl;
                            consume(2);
                            state = skLine;
                        } else if (check_string("//")) {
                            //std::cout << "--- Saw line comment start\n";
                            comment_loc = cl;
                            consume(2);
                            state = skLine;
                        } else if (check_string("/*")) {
                            //std::cout << "--- Saw block comment start\n";
                            comment_loc = cl;
                            consume(2);
                            state = skBlock;
                        } else {
                            skipping = false;
                        }
                        break;
                    case skLine: // Line comment
                        if (peek(0) == '\n') {
                            //std::cout << "--- line comment end\n";
                            state = skNormal;
                            line_count += 1;
                        }
                        consume(1);
                        break;
                    case skBlock: // Block comment
                        if (match_string("*/")) {
                            //std::cout << "--- block comment end\n";
                            state = skNormal;
                        } else {
                            if (peek(0) == '\n') {
                                line_count += 1;
                            }
                            consume(1);
                        }
                        break;
                }
            }

            if (state != skNormal) {
                record_error("Unterminated comment starting here", comment_loc);
                return false;
            }

            current_loc.line += line_count;

            return true;
        };

        bool match_char(int pos, char c) {
            return (valid_pos(pos) and peek(pos) == c);
        }

        bool match_char(char c) {
            return peek() == c;
        }

        bool match_chars(int pos, const char * c) {
            if (!valid_pos(pos)) return false;
            char t  = peek(pos);

            while (*c) {
                if (*c == t) return true;
                ++c;
            }

            return false;
        }

        //##############   match_bool_value  #################

        bool match_bool_value(Setting *parent) {
            // if there aren't enough chars then short circuit
            if (!valid_pos(4)) {
                return false;
            }

            int pos = 0;
            if (match_chars(pos, "Ff")) {
                pos += 1;
                if (! match_chars(pos, "Aa")) return false;
                pos += 1;
                if (! match_chars(pos, "Ll")) return false;
                pos += 1;
                if (! match_chars(pos, "Ss")) return false;
                pos += 1;
                if (! match_chars(pos, "Ee")) return false;
                pos += 1;
                // must be at end of word
                if (valid_pos(pos) and std::isalnum(peek(pos))) return false;

                consume(pos);
                parent->bool_ = false;
                parent->type_ = Setting::setting_type::BOOL;
                return true;

            } else if (match_chars(pos, "Tt")) {
                pos += 1;
                if (! match_chars(pos, "Rr")) return false;
                pos += 1;
                if (! match_chars(pos, "Uu")) return false;
                pos += 1;
                if (! match_chars(pos, "Ee")) return false;
                pos += 1;
                // must be at end of word
                if (valid_pos(pos) and std::isalnum(peek(pos))) return false;

                consume(pos);
                parent->bool_ = false;
                parent->type_ = Setting::setting_type::BOOL;
                return true;
            }

            return false;

        }

        bool match_numeric_value(Setting *parent) {
            if (match_string("0x") or match_string("0X")) {
                // integer in hex format - from_chars doesn't like the 0x prefix
                // can't be anything else so commit.
                auto [ ptr, ec] = std::from_chars(current_loc.sv.data()+2,
                        current_loc.sv.data()+current_loc.sv.size(), parent->integer_, 16);
                if (ec == std::errc()) {
                    parent->type_ = Setting::setting_type::INTEGER;
                    int pos = ptr-current_loc.sv.data();
                    if (valid_pos(pos) and std::isalnum(*ptr)) {
                        // End of the number wasn't at a word boundary.
                        record_error("Hex prefix, but invalid hex number followed");
                        return false;
                    }
                    consume(pos);
                    return true;
                } else {
                    record_error("Hex prefix, but invalid hex number followed");
                    return false;
                }

            }

            if (match_chars(0, "+-0123456789")) {
                // either a base-10 integer or float.
                // This mess is because from_chars in g++ v10 doesn't work
                // for floats.

                int copy_size = std::min(current_loc.sv.size(), size_t(100));
                std::string subject(std::string(current_loc.sv.substr(0, copy_size)));

                bool worked = true;
                try {
                    size_t pos = 0;
                    parent->integer_ = std::stol(subject, &pos);
                    if (valid_pos(pos) and std::isalnum(peek(pos))) {
                        worked = false;
                    } else {
                        consume(pos);
                        parent->type_ = Setting::setting_type::INTEGER;
                        worked = true;
                    }
                } catch(std::exception &e) {
                    worked = false;
                }

                if (worked) return true;

                // try float
                worked = true;
                try {
                    size_t pos = 0;
                    parent->float_ = std::stod(subject, &pos);
                    if (valid_pos(pos) and std::isalnum(peek(pos))) {
                        worked = false;
                    } else {
                        consume(pos);
                        parent->type_ = Setting::setting_type::FLOAT;
                        worked = true;
                    }
                } catch(std::exception &e) {
                    worked = false;
                }

                if (worked) return true;
            }


            return false;
        }
        //##############   match_string_value  ###############

        bool match_string_value(Setting *parent) {
            if (!match_char('"')) return false;

            // consume the opening quotes
            consume(1);
            std::stringstream buf{};

            bool stop = false;
            while(not stop and not eoi()) {
                char c = peek();
                switch (c) {
                    case '\0' :
                        //must be eoi;
                        stop = true;
                        break;
                    case '\\' :
                        if (match_chars(1, "\\fnrtx\"")) {
                            switch(peek(1)) {
                            case 'f' :
                                buf << '\f';
                                consume(2);
                                break;
                            case 'n' :
                                buf << '\n';
                                consume(2);
                                break;
                            case '"' :
                                buf << '"';
                                consume(2);
                                break;
                            case 'r' :
                                buf << '\r';
                                consume(2);
                                break;
                            case '\\' :
                                buf << '\\';
                                consume(2);
                                break;
                            case 't' :
                                buf << '\t';
                                consume(2);
                                break;
                            case 'x' :
                                if (match_chars(2, "0123456789abcdefABCDEF") and
                                        match_chars(3, "0123456789abcdefABCDEF")) {
                                    char x = (peek(2) - '0') * 16 + (peek(3) - '0');
                                    buf << x;
                                    consume(4);
                                } else {
                                    record_error("Bad hex escape in string");
                                    consume(2); // just to keep us going
                                }
                                break;
                            }
                        } else {
                            record_error("Unrecognized escape sequence in string");
                            consume(1);
                        }
                    case '\n' :
                        record_error("Unterminated string");
                        return false;
                        break;
                    case '"' :
                        stop = true;
                        consume(1);
                        break;
                    default:
                        buf << c;
                        // this will be slow - rethink.
                        consume(1);
                        break;
                    }
                skip();
                if (!match_char('"'))
                    break;
            }

            parent->string_ = buf.str();
            parent->type_ = Setting::setting_type::STRING;

            return true;

        }
        //##############   match_scalar_value  ###############

        bool match_scalar_value(Setting *parent) {

            if (match_bool_value(parent)) return true;
            if (match_numeric_value(parent)) return true;
            if (match_string_value(parent)) return true;


            return false;
        }

        //##############   match_name #######################

        std::optional<std::string> match_name() {

            int pos = 0;
            if (peek(pos) == '*' or std::isalpha(peek(pos))) {
                pos += 1;
            } else {
                return std::nullopt;
            }

            while (valid_pos(pos)) {
                if ( peek(pos) == '*' or peek(pos) == '_' or
                        std::isalnum(peek(pos))) {
                    pos += 1;
                } else break;
            }


            auto retval = current_loc.sv.substr(0, pos);
            consume(pos);

            return std::string(retval);
        }

        //##############   parse_value_list    ##############
        bool parse_value_list(Setting * setting) {
            return true;
        }

        //##############   parse_setting_value ##############

        bool parse_setting_value(Setting * setting) {
            if (peek() == '{') {
                consume(1);
                skip();
                setting->make_group();
                parse_setting_list(setting);
                skip();
                if (peek(0) != '}') {
                    record_error("Didn't find close of setting group");
                    return false;
                }
            } else if (peek() == '(') {
                consume(1);
                skip();
                setting->make_list();
                parse_value_list(setting);
                skip();
                if (peek() != ')') {
                    record_error("Didn't find close of setting list");
                    return false;
                }
            } else if (peek() == '[') {
                consume(1);
                skip();
                setting->make_array();
                //parse_scalar_value_list(&new_setting);
                skip();
                if (peek() != ')') {
                    record_error("Didn't find close of value list");
                    return false;
                }
            } else {
                if (! match_scalar_value(setting)) {
                    record_error("Expecting a value");
                    return false;
                }
            }

            return true;
        }
        //##############   setting ##########################

        bool parse_setting(Setting * parent) {

            auto name = match_name();
            if ( ! name ) {
                return false;
            }

            skip();

            if (not match_chars(0, ":=")) {
                record_error("Expecting : or = after setting name");
                return false;
            }
            consume(1);

            auto new_setting = parent->create_child(*name);

            if (!new_setting) {
                record_error("Setting named "s + *name + " already defined in this context");
                return false;
            }

            skip();

            return parse_setting_value(new_setting);
            
        }

        //##############   setting_list #####################

        bool parse_setting_list(Setting * parent) {
            bool at_least_one = false;
            while (1) {
                if (!parse_setting(parent)) break;
                at_least_one = true;

                if (match_chars(0, ";,")) {
                    consume(1);
                }
                skip();
            }

            return at_least_one;
        }

        //##############   do_parse  #####################
        
        bool do_parse() {

            skip();

            parse_setting_list(setting);

            if (! eoi()) {
                record_error("Not at end of input!");
                return false;
            }

            return errors.empty();
        }

    };

    bool Config::parse_with_schema(const std::string &input, const SchemaNode *schema){

        cfg_.reset(new Setting(Setting::setting_type::GROUP));

        Parser p{input, cfg_.get()};

        bool retval = p.do_parse();

        return retval;
    }

} // end namespace Configinator5000
