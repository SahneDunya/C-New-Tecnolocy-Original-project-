# C New Technology (CNT) Programming Language
This document explains the basics you need to know to start writing code in the C New Technology (CNT) programming language, covering the language's core features and coding principles. CNT is a systems-level programming language aiming to combine performance, memory safety, and modern programming concepts.

1. Basic Structure and Files
CNT source code files use the .cnt extension. Programs can consist of one or more .cnt files. The file system structure determines the module structure.
```
// main.cnt
// The main module
fn main() {
    // Program execution starts here
}
``` 
2. Syntax Basics
CNT syntax is inspired by languages like C++ and Rust.

Keywords: fn, struct, enum, let, mut, if, while, match, import, pub, return, break, continue, else, true, false, void.
Identifiers: Names for variables, functions, types, etc., start with a letter or an underscore (_), followed by letters, digits, or underscores. They are case-sensitive.
Semicolon (;): Most statements and expressions are terminated by a semicolon. The last expression in a block may omit the semicolon (in some cases, the value of the expression becomes the value of the block).
Blocks: Defined by curly braces {}. They create scopes.
<!-- end list -->
```
// Example syntax
let mut number: int = 10; // Variable declaration
if number > 5 { // Start of block
    number = number + 1; // Statement, terminated by semicolon
} // End of block
```
3. Data Types
CNT has various built-in (primitive) and composite data types.

3.1. Primitive Types
int: Integer number (Size is target-platform dependent, typically 32 or 64 bits).
float: Floating-point number (Typically 32 or 64 bit IEEE 754).
bool: Boolean value (true or false).
char: Unicode character (UTF-8, typically 8-bit).
string: String slice (similar to &str, UTF-8 encoded). It is immutable.
void: The unit type (), typically used as the return type for functions that do not return a value.
3.2. Composite Types
struct: A collection of structured data. Can have fields.
enum: A type defining a set of named variants, which may optionally have associated data.
array: A fixed-size sequence of elements of the same type ([Type; size]).
reference: A reference to another value (&Type, &mut Type).
pointer: A raw memory address (*Type, *mut Type) (if needed for C/C++ integration or specific use cases).
tuple: A fixed-size collection of elements of potentially different types ((Type1, Type2, ...)).
<!-- end list -->
````
// Type definitions and usage
struct Point {
    x: float;
    y: float;
}

enum Result {
    Success(int); // Associated data
    Error(string); // Associated data
}

let coordinates: [int; 3] = [1, 2, 3]; // Array
let mut point_ref: &mut Point = &mut a_point; // Mutable reference
let success_v: Result = Result::Success(100); // Enum variant usage
````
4. Variables
Variables are declared using the let keyword. By default, they are immutable. Use the mut keyword to make them mutable.
```
let age: int = 30; // Immutable integer
// age = 31; // Error! Immutable variable cannot be reassigned.

let mut score: float = 95.5; // Mutable float
score = score + 2.0; // Valid

let name = "Ahmet"; // Type inference: name is of type string
let mut is_active = true; // Type inference: is_active is of type bool
is_active = false; // Valid
```
The scope of variables is limited to the block in which they are declared. When they go out of scope, if the value they own requires Dropping, it is automatically cleaned up.

5. Functions
Functions are defined using the fn keyword. They have parameters and a return type.
```
// Function with no parameters, void return
fn hello() {
    // ... code ...
}

// Function with parameters and a return value
fn add(a: int, b: int) -> int {
    let result = a + b;
    return result; // Return a value explicitly
}

// Function with parameters, returning void
fn print(message: string) -> void {
     // ... printing operation ...
}

// Function calls
let sum = add(5, 3); // sum becomes 8
print("Hello World");
hello();
```
If the last expression in a function body is not terminated by a semicolon, the value of that expression becomes the function's return value (if the return type is not void).
```
fn subtract(a: int, b: int) -> int {
    a - b // No semicolon, the value of the expression (a - b) is returned
}
```

6. Control Flow
CNT supports common control flow constructs.

6.1. Conditional Statements (if, else if, else)
Standard conditional branching. Conditions must be of type bool.
```
let x = 10;
if x > 5 {
    // Runs if x is greater than 5
} else if x == 5 {
    // Runs if x is equal to 5
} else {
    // Runs in all other cases
}

// `if` can also be used as an expression (branches must have compatible types)
let status = if x > 5 { "Greater" } else { "Less or Equal" }; // status is of type string
```
6.2. Loops (while)
The block is executed repeatedly as long as the condition is true.
```
let mut i = 0;
while i < 5 {
    // Runs while i is less than 5
    i = i + 1;
}
```
break;: Exits the innermost loop.
continue;: Skips the rest of the current iteration of the innermost loop and proceeds to the next iteration (the condition is checked again).
<!-- end list -->
```
let mut j = 0;
while true { // Infinite loop
    if j == 3 {
        j = j + 1;
        continue; // Skip this iteration
    }
    if j > 5 {
        break; // Exit the loop
    }
    // This part is skipped when j == 3
    j = j + 1;
}
```
6.3. Pattern Matching (match)
The match expression is a powerful construct for controlling the flow of execution based on a value matching against different patterns. It's particularly useful with enum types.
```
enum UserStatus {
    Active;
    Inactive;
    Pending(int); // Pending duration (seconds)
}

let user_state = UserStatus::Pending(60);

match user_state {
    UserStatus::Active => {
        // Actions for active status
        print("User is active.");
    },
    UserStatus::Inactive => print("User is inactive."), // Single-line result
    UserStatus::Pending(duration) => { // Bind associated data to 'duration' variable
        print("User is pending. Remaining time: " + duration.toString() + " seconds.");
    },
    // _: print("Unknown status"); // Wildcard pattern: Catches anything not matched above
}
```
Important: A match expression must be exhaustive. This means it must cover all possible cases of the value being matched. Otherwise, you will get a compiler error. The _ wildcard pattern can be used to catch all remaining cases and ensure exhaustiveness.

Reachability: The order of match arms matters. The compiler may detect and warn or error on unreachable arms (arms that are completely covered by previous arms).

7. Ownership and Borrowing
The ownership system, a core feature of CNT, guarantees memory safety.

Rule 1: Each value in CNT has a owner.
Rule 2: There can only be one owner at a time.
Rule 3: When the owner goes out of scope, the value is Dropped (memory is freed).
These rules prevent data races and null pointer errors at compile-time.

7.1. Move vs. Copy
Move: Assignment (=) and function argument passing transfer ownership by default. The owner changes, and the old owner can no longer use the value. This is important for types that manage resources (like string or Struct/Enum types with a Drop implementation).
```
let s1: string = "hello";
let s2: string = s1; // Ownership of s1 is moved to s2
// print(s1); // Error! s1 is Moved, no longer valid.
print(s2); // Valid
```
Copy: Some simple types (like int, bool, float, char) and references (&, &mut) implement the Copy trait automatically. For these types, assignment or argument passing performs a copy. Both the old and the new location hold independent copies of the value. The old owner can still use the value. User-defined Struct/Enum types are automatically Copy if all their fields/variants are Copy and they do not have a Drop implementation.
````
let i1: int = 10;
let i2: int = i1; // i1 is copied
let i3: int = i1; // i1 is copied again
print(i1.toString()); // Valid (i1 can still be used)
print(i2.toString()); // Valid
````
7.2. Borrowing
To access a value temporarily without transferring ownership or copying it, references (&, &mut) are used. This is called borrowing.

&value: Takes an immutable reference to the value. The value can only be read through the reference.
&mut value: Takes a mutable reference to the value. The value can be read and written through the reference.
<!-- end list -->
```
let mut score = 100;
let score_ref: &int = &score; // Immutable reference
// *score_ref = 105; // Error! Cannot modify value through an immutable reference.

let mut points = 50;
let mut points_mut_ref: &mut int = &mut points; // Mutable reference
*points_mut_ref = *points_mut_ref + 10; // Valid (modified through a mutable reference)
```
7.3. Borrowing Rules
The CNT compiler enforces the following borrowing rules at compile-time:

At any given time, you can have either:
One mutable reference (&mut T),
or any number of immutable references (&T).
While a mutable reference (&mut) is active, neither the original value it points to nor any other references (& or &mut) to that value can be used. It must have exclusive access.
While immutable references (&) are active, the original value can only be read. It cannot be modified or moved.
<!-- end list -->
```
let mut data = 100;

let r1 = &data; // Valid (Immutable borrow)
let r2 = &data; // Valid (Another immutable borrow, OK)
let r3 = &mut data; // Error! Cannot take a mutable borrow while immutable borrows (r1 or r2) are active.
print((*r1).toString()); // Valid (Read through an immutable borrow)

// Borrows end when r1 and r2 go out of scope (or are no longer used).
 let mut data2 = 200;
 let mr1 = &mut data2; // Valid (Mutable borrow)
 let r4 = &data2; // Error! Cannot take an immutable borrow while a mutable borrow (mr1) is active.
 let mr2 = &mut data2; // Error! Cannot take another mutable borrow while a mutable borrow (mr1) is active.
 print(data2.toString()); // Error! Cannot use the original value while a mutable borrow (mr1) is active.
 *mr1 = 205; // Valid (Write through a mutable borrow)
```
// The borrow ends when mr1 goes out of scope (or is no longer used).
7.4. Lifetimes
Every reference has a lifetime – the scope for which the reference is valid. The CNT compiler checks at compile-time that references do not outlive the data they point to. The compiler usually infers lifetimes automatically. This prevents Dangling Pointer errors.
```
fn largest(a: &int, b: &int) -> &int {
    // The lifetime of the returned reference must be shorter than or equal to
    // the shorter of the lifetimes of a and b.
    // The compiler automatically checks this.
    if *a > *b { a } else { b }
}

let val1 = 10;
let val2 = 20;
let largest_ref = largest(&val1, &val2); // The lifetime of largest_ref cannot exceed the lifetimes of val1 and val2.
```
// When val1 and val2 go out of scope, largest_ref becomes invalid (the compiler will error if you try to use it).
Violating ownership and borrowing rules results in compile-time errors. This ensures that errors are caught during compilation rather than at runtime.

8. Modules and Imports
Modules are used to organize and reuse your code. The visibility of items (functions, structs, enums, global variables) is controlled with the pub keyword.

The pub Keyword: Adding pub before a declaration makes that item accessible from outside the module it's defined in. Functions, structs, enums, and global variables can be pub.
```
// my_module.cnt
pub fn public_func() { /* ... */ } // Callable from outside the module

fn private_func() { /* ... */ } // Only callable from within this module

pub struct PublicData { /* ... */ } // Usable from outside the module

struct PrivateData { /* ... */ } // Only usable from within this module

pub let PI: float = 3.14; // Global constant, accessible from outside the module
```
The import Statement: Used to bring items marked as pub in another module into your code.
```
// main.cnt
// Import public items from my_module.cnt (assuming it's in the same directory)
import my_module; // Items are accessed as my_module::public_func(), my_module::PublicData

import my_module as mm; // Using an alias: mm::public_func(), mm::PublicData

fn main() {
    my_module::public_func(); // Access using the module name directly
    let data: mm::PublicData; // Access using the alias
}
Module paths (module::path) follow the file system structure, and the compiler uses these paths to search for .hnt (Header/Interface) files. For example, the statement import std::io; would cause the compiler to look for a std/io.hnt file within its configured search paths.
```
9. Structs and Enums
9.1. Struct Definition and Usage
Structs are used to group related data into named fields.
```
struct User {
    id: int;
    name: string;
    is_active: bool;
}

let mut new_user: User; // Struct variable declaration (uninitialized if no initializer)

// Accessing fields and assigning values
new_user.id = 1;
new_user.name = "Ayşe";
new_user.is_active = true;

// Initializing using a struct literal
let another_user: User = {
    id: 2,
    name: "Mehmet",
    is_active: false,
};

// Accessing fields (dot operator .)
let username = new_user.name;
```
9.2. Enum Definition and Usage
Enums are used to represent a value that can be one of a set of named variants. Variants can have associated data.
```
enum StudentStatus {
    Enrolled;
    Active;
    Graduated(int); // Graduation year
    Dropped(string); // Reason
}

let status1 = StudentStatus::Enrolled; // Variant usage (no data)
let status2 = StudentStatus::Graduated(2023); // Variant usage (with associated data)

// Enum variants are typically accessed and processed using the `match` expression.
match status2 {
    StudentStatus::Enrolled => print("Student is enrolled."),
    StudentStatus::Active => print("Student is active."),
    StudentStatus::Graduated(year) => print("Student graduated in " + year.toString() + "."),
    StudentStatus::Dropped(reason) => print("Student dropped out: " + reason),
}
```
10. Expressions and Operators
Expressions are code constructs that produce a value. Operators perform operations on values.

Arithmetic Operators: +, -, *, /, % (Modulo). Used for numeric types.
Comparison Operators: ==, !=, <, >, <=, >=. Used for numeric and some other types (e.g., string), result is of type bool.
Logical Operators: && (AND), || (OR), ! (NOT). Used only for bool types. && and || are short-circuiting.
Assignment Operator: = (Assigns the value on the right to the location on the left; ownership/copy rules apply).
Unary Operators:
-expr: Numeric negation.
!expr: Logical NOT (bool).
&expr: Take an immutable reference (operand must be an l-value).
&mut expr: Take a mutable reference (operand must be a mutable l-value).
*expr: Dereference (operand must be a pointer or reference, loads the value at the address it points to).
Member Access: expr.member_name (Struct fields, Enum variants with associated data, Methods).
Index Access: expr[index] (Arrays, Pointers, Slices).
Function Call: function_name(arguments) or expression(arguments) (if the expression evaluates to a function pointer/reference).
<!-- end list -->
```
let a = 10;
let b = 5;
let c = a + b * 2; // Operator precedence applies
let is_greater = a > b && a != 0;
let is_not_active = !is_active;

let mut arr: [int; 5] = [1, 2, 3, 4, 5];
let first_element = arr[0];
arr[1] = 10;

let address: &int = &a; // Take the address of a (immutable reference)
let value_a = *address; // Load the value at the address
```
11. Comments
Use comments to document your code.

// Single-line comment
/* Multi- line comment */
Conclusion
This document provides an introduction to the basic syntax, types, variables, functions, control flow, and most importantly, the ownership system of the CNT programming language. CNT's focus on memory safety means adherence to ownership and borrowing rules is required, but these rules help you write safer and more reliable code by catching errors at compile-time.

More advanced topics (traits, generics, advanced pattern matching, FFI - Foreign Function Interface) may be added in the future. When coding with CNT, paying close attention to compiler errors will greatly help you understand the language's rules.

Start coding with CNT and experience memory safety!
