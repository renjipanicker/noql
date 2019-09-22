#V0.1
- done

#V1.0
- get and test examples of advanced queries - "longest river in India?"

#V1.0
- external adaptor - sqlite3
- namespace in noql code
- add support for datetime
- native function implementation (DATEDIFF, etc)

#V2.0
- implement ruleset (set of rules in a block)

#DONE
- logging (spdlog)
- basic operators
- full Json adaptor with setter
- query bindings (@x, etc)
- check if the query was handled before. If yes, return from cache (memoisation in QueryEngine)
- native call for arity == 1, search for key-value pair, if supported by adaptor
- Json adaptor read/write to files
- external adaptor - mongodb
- Make Mongo support optional
- REPL
- REPL empty line error
- REPL print OK (or feedback) after command
- load noql code from file
- different adaptors (mongodb/etc)
- enable/disable spdlog using #define
