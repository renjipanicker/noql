**NOQL is an innovative paradigm for querying NoSQL document databases.**

_Author: Renji Panicker (renjipanicker@gmail.com)_

NOQL is a Domain-Specific Language, just like SQL.
It is heavily inspired by Prolog and uses logic programming to form complex queries in a way that expands the possibilities of NoSQL databases far beyond its current usage. For example, it can be used for knowledge modelling.

*IMPORTANT: Knowledge of Prolog is not necessary to understand NOQL.*

# Build instructions.
## 1. Prerequisites and Dependencies:
The following prerequisites should be installed prior to build are:

|Prerequisite   |Version        |Description      |
|---------------|---------------|-----------------|
|clang          |7 and above    |C++17 compiler   |
|lldb           |any latest     |debugger         |
|cmake          |3.14 and above |build generator  |
|git            |any latest     |SCM              |
|make or ninja  |any latest     |build system     |
|pthreads       |system default |pthreads library |

Commands to install all prerequisites for Ubuntu 18.04 LTS:
```
sudo snap install cmake --classic
sudo apt install clang lldb git libpthread-stubs0-dev ninja-build -y
```
*NOTE: NOQL requires CMake 3.14 or above. On Ubuntu 18.04LTS, the default version is 3.10, hence we use snap to install version cmake 3.15.*

During the build process, the following dependencies will be automatically downloaded and built:

|Dependency     |Description         |Usage   |Switch        |Default|
|---------------|--------------------|--------|--------------|-------|
|lemonex        |Small lexer/parser  |Required|
|spdlog         |Logging support     |Optional|ENABLE_SPDLOG |ON     |
|nlohmann::json |JSON library        |Optional|ENABLE_JSON   |ON     |


Then follow the steps in the next section to download and build NOQL.

## 2. Fetch the code from github:
```
git clone https://github.com/renjipanicker/noql.git
cd noql
```
## 3. Create a build directory:
```
mkdir build
cd build
```
## 4. Use cmake to generate the build system:
```
cmake -DCMAKE_BUILD_TYPE=Debug -GNinja ..
```
You may replace **Debug** with **Release**, and **Ninja** with any other build system you prefer.

You may also selectively enable or disable features using the switches mentioned in the table above, as shown here:
```
cmake -DCMAKE_BUILD_TYPE=Debug -DENABLE_MONGOC=ON -GNinja ..
```
*NOTE: Additional dependencies required for mongodb support are documented at the end of this page.*

## 5. Build the project
You may now build the project as follows:
```
ninja
```

At the end of the build process, the output directory will hold:
- libnoql.a: A static library that contains the NOQL functionality and can be incorporated into any C++17 project.
- noql-cli: An REPL shell.

## 6. Run noql
[Optional] You may now run all the unit tests to ensure noql works as expected:
```
./noql-cli -t all
```

Finally, you may now launch the noql shell and follow along the examples in the next section:
```
./noql-cli
```

# Basic background.
A document database stores and retrieves documents.
A JSON object, for example, is a document.\
A JSON object consists of an hierarchical key-value pair structure.\
For example, this JSON object is a document:
```
{
    "id":"Anakin",
    "age":32,
    "height":183.00,
    "male":true,
    "spouse":"Padme",
    "children":["Luke", "Leia"],
}
```
This document is stored in a database such as mongodb, which can then be queried and returned.
noql views this single document as a series of *(Object,Attribute,Value)* tuples.

So noql sees the above document as a set of tuples as follows:
```
("Anakin", "age", 32)
("Anakin", "height", 183.00)
("Anakin", "male", true)
("Anakin", "spouse", "Padme")
("Anakin", "children", "Luke")
("Anakin", "children", "Leia")
```

With this background in mind, let us look at our first example.

*[The following examples use the Skywalker family from Star Wars for representation, featuring Anakin Skywalker, his wife Padme, their son Luke and daughter Leia, with a guest appearance by Han Solo as Luke's friend]*

# Basic query example.
Insert a document into the underlying database as follows at the prompt:
```
INSERT DOCUMENT {"id":"Leia", "parent":"Padme", "gender":"female"};
```
*NOTE: You can copy-paste the above command to the '>' prompt.*

The most basic query in NOQL is as follows:
```
?parent("Leia", @p)
```
This will return:
```
Found 1 result.
parent("Leia", "Padme")
```
**Explanation**: A query in NOQL is in the form of a function, where the function name is the name of the attribute being queried.

The function has 2 parameters - the object being queried, and the variable to receive the value. In this case, *@p* is the variable in the query, and "Padme" is the value received into it.

In other words, the *(Object,Attribute,Value)* tuple mentioned above translates to a function in the form *Attribute(Object,Value)*. This is to be read as **"Attribute of Object is Value"**.

The significance of this form will become clear when we look at the concept of a Rule next.

# Basic rule example.
Insert the following two documents at the command prompt.\
*NOTE: The first one is already inserted from the earlier example, so you only need to insert the second one.*
```
INSERT DOCUMENT {"id":"Leia", "parent":"Padme", "gender":"female"};
INSERT DOCUMENT {"id":"Padme"};
```
We have now inserted a second document representing Padme, and we would like to query for "Who is Padme's child?".
```
?child("Padme", @p)
```
This will return:
```
Found 0 results.
```

We have no way to query for Padme's child, unless the second object has an attribute such as the following:

**NOTE: Do not execute this command, it is only for the purpose of discussion**
```
INSERT DOCUMENT {"id":"Padme", "child":"Leia"};
```
We don't have such an attribute in our database, but we can logically infer that **If Padme is the parent of Leia, then Leia is the child of Padme**.

This inference can be expressed as a Rule, as follows. Go ahead and type or copy-paste the following command at the prompt:
```
RULE child(@x,@y) := parent(@y,@x);
```
A rule consists of a head and a body. In the above example, the "child(@x,@y)" is the head, which is the inferred function. The "parent(@y,@x)" is the body of the rule.

Now we repeat the query for Padme's child:
```
?child("Padme", @p)
```
This will return:
```
Found 1 result.
"child"("Padme","Leia")
```
We are now able to make a logical inference using a rule.

# Complex rule example.
You may have noticed that the document we inserted for Leia has an attribute ```"gender":"female"```
Let us use that to create a slightly more complex rule consisting of 2 functions in the body:
```
RULE daughter(@x,@y) := parent(@y,@x) && gender(@y, "female");
```
We can now query for Padme's daughter:
```
?daughter("Padme", @p)
```
This will return:
```
Found 1 result.
"daughter"("Padme","Leia")
```

# Invoke rule from rule
The power of NOQL lies in the fact that we can invoke a rule from within another rule.
```
RULE child(@x,@y) := parent(@y,@x);
RULE daughter(@x,@y) := child(@x,@y) && gender(@y, "female");

?daughter("Padme", @p)
Found 1 result.
"daughter"("Padme","Leia")
```

# Query by value.
A major feature of NOQL is the abililty to query by value.
```
RULE child(@x,@y) := parent(@y,@x);
RULE daughter(@x,@y) := child(@x,@y) && gender(@y, "female");

?daughter(@p, "Leia")
Found 1 result.
"daughter"("Padme","Leia")
```
Here the query is "Whose daughter is Leia?", and we get the answer "Padme".

# Recursion.
NOQL rules can call themselves recursively, as seen below.
```
INSERT DOCUMENT {"id":"Luke", "friend":"Solo"};
RULE friend(@x,@y) := friend(@y,@x);

?friend("Solo", @p)
Found 1 result.
"friend"("Solo","Luke")

?friend("Luke", @p)
Found 1 result.
"friend"("Luke","Solo")
```

# Arrays.
NOQL treats individual items in an array as separate elements.
```
INSERT DOCUMENT {"id":"Leia", "parent":["Padme","Anakin"], "gender":"female"};

?parent("Leia", @p)
Found 2 results.
"parent"("Leia","Padme")
"parent"("Leia","Anakin")

?parent(@p, "Padme")
Found 1 result.
"parent"("Leia","Padme")

?parent(@p, "Anakin")
Found 1 result.
"parent"("Leia","Anakin")
```

# Expressions.
NOQL has support for basic arithmetic and boolean expressions.
```
INSERT DOCUMENT {"id":"Leia", "age":17};
INSERT DOCUMENT {"id":"Luke", "age":19};
RULE adult(@x) := age(@x,@y) && (@y >= 18);

?adult("Leia")
Found 0 results.

?adult("Luke")
Found 1 result.
"adult"("Luke")
```
The currently supported operators are:
```
- + - * / %
- < <= > >= == !=
```

# Arity
The arity of a function is the number of arguments that function takes (Arguments, Arity... get it?).

This is an important concept. All functions in the examples we have seen so far has an arity of 2, that is, they take 2 arguments each.
However, the functions in any NOQL rule can take any number of arguments.

For example:
```
RULE max(@x,@y, @z) := (@x > @y) && (@z = @x);
RULE max(@x,@y, @z) := (@x <= @y) && (@z = @y);

?max(10,12,@z)
Found 1 result.
"max"(10,12,12)

?max(12,10,@z)
Found 1 result.
"max"(12,10,12)

```
This is a set of two rules that returns the max of 2 numbers. In this example, the function *max* has an arity of 3.

As mentioned earlier, in general, a NOQL function or query can have any arity. But only those with an arity of specifically 2 translate to queries into the underlying database (json or mongodb). The rest only translate into queries for rules.

# Key-Value pairs
This feature will be implemented in future. A function with an arity of 1 will be able to query an underlying key-value database such as leveldb.

# Data manipulation statements
## INSERT statement
NOQL supports an INSERT statement in the form:
```
INSERT DOCUMENT <json>;
```
This is used to insert a json object into a document database.

In future there will be support for the following form:
```
INSERT PAIR <json>;
```
This will be used to insert a key-value pair into a KV dattabase.

## UPDATE statement (TODO)
NOQL supports an UPDATE statement in the form:
```
UPDATE DOCUMENT <json> WHERE <criteria>;
```
This is used to update all documents that match the selection criteria.

## UPSERT statement (TODO)
NOQL supports an UPSERT statement in the form:
```
UPSERT DOCUMENT <json> WHERE <criteria>;
```
This is used to update all documents that match the selection criteria. If no documents match the criteria, a new document is inserted.

## DELETE statement (TODO)
NOQL supports an DELETE statement in the form:
```
DELETE DOCUMENT <json> WHERE <criteria>;
```
This is used to delete all documents that match the selection criteria.

## VERIFY statement
A VERIFY statement is used to execute unit tests. It is of the form:
```
VERIFY <query> := [<function>];
```
The LHS specifies the query to be executed, and the RHS is a list of expected values.

# C++ API
The NOQL library exposes a C++ APi that enables the developer to incorporate NOQL into any C++ application.
This API is conceptually similar to most SQL API's and consists of the following components:

## Connection
A connection object is used to open a connection to an underlying database. The database is specified using a connection string. Here is a minimal example:
```
#include "noql/noql.hpp"

int main(int argc, const char** argv){
    noq::init();

    std::string connectionString = "json://memory/";
    noq::Connection connection;
    connection.open(connectionString);

    return 0;
}
```
This opens a connection to an in-memory JSON database.

You can copy-paste the above code to a file and build it as follows:
```
clang++ -I../include -std=c++17 noqltest.cpp lib/libnoql.a -lpthread
```
*NOTE: This assumes libnoql.a is in the ./lib/ directory*

If NOQL has been compiled with support for MongoDB, you could a connection string such as the following to connect to a mongodb server:
```
    std::string connectionString = "mongodb://localhost:27017/myappname?database=mydb&collection=mycoll&id=id";
```
A connection also stores all the rules that are defined before executing a query.
```
#include "noql/noql.hpp"

int main(int argc, const char** argv){
    noq::init();

    std::string connectionString = "json://memory/";
    noq::Connection connection;
    connection.open(connectionString);

    std::string ruleString = "RULE child(@x,@y) := parent(@y,@x);";
    connection.execString(ruleString);

    return 0;
}
```
Similarly, one can execute INSERT and VERIFY statements on the connection.

## Query
A Query object represnts a query made on a connection. Here is a minimal example:
```
#include "noql/noql.hpp"

int main(int argc, const char** argv){
    noq::init();

    std::string connectionString = "json://memory/";
    noq::Connection connection;
    connection.open(connectionString);

    // insert a document
    std::string insertString = "INSERT DOCUMENT {\"id\":\"Leia\", \"parent\":\"Padme\", \"gender\":\"female\"};";
    connection.execString(insertString);

    // execute a query
    noq::Query qry;
    qry.exec("gender(\"Leia\", @y)", connection);
    for(auto& r : qry){
        std::cout << r << std::endl;
    }

    return 0;
}
```

## Query binding
It is also possible to precompile queries in advance and bind values just before executing them.
```
#include "noql/noql.hpp"

int main(int argc, const char** argv){
    noq::init();

    std::string connectionString = "json://memory/";
    noq::Connection connection;
    connection.open(connectionString);

    // insert a document
    std::string insertString = "INSERT DOCUMENT {\"id\":\"Leia\", \"parent\":\"Padme\", \"gender\":\"female\"};";
    connection.execString(insertString);

    // execute a query
    noq::Query qry;
    qry.compile("gender(@x, @y)");
    qry.set("x", "Leia");
    qry.exec(connection);
    for(auto& r : qry){
        std::cout << r << std::endl;
    }

    return 0;
}
```
This is useful when a query has to be executed multiple times, where you can reduce the time required to compile the query string.

## Adaptor
An adaptor is used to connect to a backend database. NOQL currently implements 2 addaptors, for JSON and MongoDb.

You may need to create a new adaptor to connnect to another database, or to a JSON file or a mongodb with a different schema.

The file AdaptorTemplate.hpp in the source directory can be used as a starting point to implement your own adaptor.

Once created, a new adaptor has to be registered with NOQL, like this:
```
    noq::Connection::registerAdaptor("foo", std::make_unique<MyAdaptor::MyFactory>());
```
After registration, the new adaptor can be used using an appropriate connection string:
```
    std::string connectionString = "foo://host/path/?connection_param1=value1&connection_param2=value2";
    noq::Connection connection;
    connection.open(connectionString);
```

# Building with support for mongoc
To include support for mongodb, the following prerequisites must be manually installed.

|Prerequisite   |Version       |Description           |Switch        |Default|
|---------------|--------------|----------------------|--------------|-------|
|mongoc         |any latest    |Mongo client library  |ENABLE_MONGOC |OFF    |

NOTE: The find_package() command for mongoc is currently disabled because the default version that is installed on Ubuntu 18.04LTS has some errors in its configuration files.

Nonetheless, the NOQL build will succeed as long as the libmongoc-dev package is installed on the system.
