# Configinator5000

Place holder

library to help with both parsing and validation of config files.

For some configuration files, validating the structure of the parsed settings 
(required keys are present, no unknown keys are present, values have acceptable types.)
can be as time consuming as the parsing of the syntax.

## Design Goals
- Based on [libconfig](https://github.com/hyperrealm/libconfig)
- C++17 based.
- will allow the user to specify a schema for the target configuration.
- will be setup to work nicely with FetchConfig.
- throw as little as possible without making the interface horrible.

## Differences from libconfig
- Only one integer type (long)
- Only one float type (double)
- Hex numbers are integers *only*


```
//sample schema file
// valid types are "string", "int", "float" , "group", "list", "array"
// note : 1000 will parse as either a float or an int, but 1000.0 will only parse as a float.
default-osc = { type : "string", required : "no" }
midi { type : "group", required : "yes"
  // key* means it has any number of keys with given form
  key* = { type : "group",
    // keys are the exact keys that must be in the group.
    // both keys and key* are usable inside the same group.
    keys = {
      in : { type : "String", required : "yes" }
      osc : { type : "string", required : "no" }
    }
  }
}
```
```C++
// sample usage
#include <configinator5000.hpp>
#include <string>

std::string schema = R"DELIM(
// long schema ....
)DELIM"s

Configinator5000::Configinator cfg;

if (!cfg.set_schema(schema)) {
  std::cerr << "Bad Schema\n";
}

if (! cfg.parse(file_name)) {
  std::cerr << "Yikes\n";
}

// Or
if (! cfg.parse(my_istream)) {
  std::cerr << "Yikes again!\n";
}
```

# API

## class Setting

The heart of the system. A Setting represents a value (not a key/value) - it
can be either a scalar (long, bool, double, string) or composite (group, list,
array).


###  `Setting(setting_type t = setting_type::BOOL)`
Create a Setting of the desired type of value. The actual
value is the default initialized value for the type.

### `Setting(bool b)`
### `Setting(int i)`
### `Setting(long i)`
### `Setting(double f)`
### `Setting(std::string s)`
### `Setting(const char * c)`
Create a Setting with the desired value and type. Some apparent duplicates are
there to help disabiguate the overloads.

### `Setting & set_value(bool v)`
### `Setting & set_value(int v)`
### `Setting & set_value(long v)`
### `Setting & set_value(double v)`
### `Setting & set_value(const std::string v)`
### `Setting & set_value(const char * v)`

Update the Setting to have the value and type specified. These are mutators.
The reference to the object is returned to facilitate chaining.

