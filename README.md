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


### Constructors
-  `Setting(setting_type t = setting_type::BOOL)`
Create a Setting of the desired type of value. The actual
value is the default initialized value for the type.

- `Setting(bool b)`
- `Setting(int i)`
- `Setting(long i)`
- `Setting(double f)`
- `Setting(std::string s)`
- `Setting(const char * c)`
Create a Setting with the desired value and type. Some apparent duplicates are
there to help disabiguate the overloads.

### Type probes

- `bool is_composite()`
- `bool is_scalar()`

- `bool is_boolean()`
- `bool is_integer()`
- `bool is_float()`
- `bool is_string()`
- `bool is_group()`
- `bool is_list()`
- `bool is_array()`

- `bool is_numeric()`

### Setting/Updating the Scalar Value

- `Setting & set_value(bool v)`
- `Setting & set_value(int v)`
- `Setting & set_value(long v)`
- `Setting & set_value(double v)`
- `Setting & set_value(const std::string v)`
- `Setting & set_value(const char * v)`
Update the Setting to have the value and type specified. These are mutators.
The reference to the object is returned to facilitate chaining.

If the Setting is currently composite, these will remove any attached children
and switch the type to the correct scalar type.

**NOTE** The reference may become invalid if more children are added to the
composite. Don't hold on to it for long.


### Working with composites

- `Setting& make_list()`
- `Setting& make_group()`
- `Setting& make_array()`
Switches the Setting to be the requested type of composite. If it already is
correct, this is a no-op. If the type actually changes, any existing children
will be discarded.

- `int count()`
Return how many children the composite has. Actually also works with scalars
where it always returns 0.

- `Setting& add_child(setting_type t)`
Add a child of type `t`. The value for scalars will be the default initialized
value. The throws an exception for groups since children for groups must have
names. Also throws for scalars.

**NOTE** The reference may become invalid if more children are added to the
composite. Don't hold on to it for long.

- `template\<typename T\> Setting& add_child(T value)`
Attempts to deduce the correct scalar type for T and create a child of that
type initialized to the value. This throws an exception for groups since
children for groups must have names. Also throws for scalars.

**NOTE** The reference may become invalid if more children are added to the
composite. Don't hold on to it for long.

#### Setting& add_child(std::string name, setting_type t)
#### Setting& add_child(std::string name, T value)
Variants of the above for groups. Throws for scalars or non-group composites.

**NOTE** The reference may become invalid if more children are added to the
composite. Don't hold on to it for long.

#### Setting &at(int idx)
Return a reference to the child at index idx. Negative values count from the
end. The method throws if the index is out of range or the Setting is a scalar.

Note that this does work for groups, but you don't get access to the name.

#### bool exists(std::string name)
Checks if a given key exists in a group. Returns true if:
- The Setting is a group
- The key exists.
Returns true otherwise.

#### Setting& at(std::string name)
Returns a reference to the child added with name `name`. Throws if such a child
does not exist or the Setting is not a group.
