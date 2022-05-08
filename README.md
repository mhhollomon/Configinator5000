# Configinator5000

Place holder

library to help with both parsing and validation of config files.

For some configuration files, validating the structure of the parsed settings 
(required keys are present, no unknown keys are present, values have acceptable types.)
can be as time consuming as the parsing of the syntax.

- will be based on libconfig
- will be C++17 based.
- will allow the user to specify a schema for the target configuration.
- will be setup to work nicely with FetchConfig.
- header only ???

```
//sample schema file
// valid types are "string", "int", "float" , "group", "list"
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
