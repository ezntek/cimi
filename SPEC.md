# Comments

`// this is a comment`

```
/*
 * this is a multiline comment
 */
```

# Imports

(loads everything into the current namespace)

`include "file.cimi"`

# Identifiers

`[a-z A-Z _ ][a-z A-Z _ 0-9]`

# Literals

```
3               // int
5.5             // float
5.0             // float
"aaa"           // string
'a'             // char
true false      // bool
{"a", "b", "c"} // array
```

# Basic builtins

```
print(args: any...)
println(args: any...)
read(buf: string)
toupper(str: string)
tolower(str: string)
slice(str: string, begin: int, end: int)
```

# variables and constants

```
const name = value
let name = value

const name: type = value
let name: type = value
```

# Conditionals

```
if condition then
    // code
end

if condition then 
    // code
else
    // code
end

if condition then
    // code
else if condition then
    // code
end
```

# Loops

```
while condition do
    break
end

// do while
do
    continue
while condition

for i = 1,5
    continue
end

for i = 10,1,-1
    continue
end
```

# Pattern Matching

```
switch expr
    case expr
        // next
    continue // fallthrough
    case expr
        // code
    break
    default
        // code
    break
end
```

# Functions

```
fn ident(val: type)
    return
end

fn ident(val: type): type
    return value
end

fn ident
    return
end
```
