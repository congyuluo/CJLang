## Introduction

Welcome to CJLang! CJLange is a programming language designed by myself (Congyu Luo, Jeff)
for fun. I built this language in C using knowledge learned from the book "Crafting Interpreters" 
by Robert Nystrom. Part of my compiler uses code modified from the original book's code.

CJLang uses syntax partially borrowed from Robert Nystrom's Lox language. It uses a single-pass
pratt parser in compiling the source code. All bytecode is then stored in a single chunk for runtime.
The VM uses a stack-based architecture in running its bytecode.

CJLang supports local & global variables, functions, variable type checking, print statements, recursion, 
mathematical computations, and logical expressions.

I did this project purely as a hobby during my summer break. Since this language has not been comprehensively tested by 
professionals, please do not use this language in any type of production environment. I won' t be responsible if you 
decide to code your newest plane's flight control system in CJLang, lol.

## How to run code

Run the compiled compiler & VM sourcecode with your CJLang sourcecode file as program argument.

## CJLang Documentation

### Supported value types

```python
String_Value = "Hello World";
Number_Value = 1;
Boolean_Value = True;
```

### Global variable declaration

```python
x = "Hello World";
```

When a variable is declared in the global scope, it will be stored to global hashmap of the VM.

Global variable could also be declared in a higher scope using `Global` keyword.

```python
def test(){
  Global x = "Hello World";
}

test();
print x;

>> Hello World
```

### Local variable declaration

```python
def test(){
  x = "Hello World";
}
```

When a variable is declared in a scope higher than global scope, Eg: In a function, it will be stored in the VM's stack. 

Local & Global variables are allowed to have same names.

```python
def test(){
  x = "Hello World";
}

x = 1;
test();
print x;

>> 1
```

### Accessing variables

When accessing a variable in a local scope, VM will first search for a variable by its name in the local scope. If such 
name does not exist in the local scope, the VM will then search in the global scope. In other words, local scope will be
prioritized.

Accessing local variable in local scope:

```python
def test(){
  x = "Hello World";
  print x;
}

x = 1;
test();

>> Hello World
```

Accessing global variable in local scope:

```python
def test(){
  print x;
}

x = 1;
test();

>> 1
```

When trying to access a variable which does not exist in both local & global scope, or have not been created, an error 
will be raised.

```python
print x;
>> Variable with name 'x' does not exist in global scope.
```

Since local variables are popped from the stack once its scope has finished, only global variables can be accessed in the global scope.

```python
x = 1;
print x;

>> 1
```
Incorrect usage:

```python
def test(){
  x = 1;
}

test();
print x;

>> Variable with name 'x' does not exist in global scope.
```

### Type checking

`type()` function can be used to check the type of a variable during runtime. Type function returns with a string containing
the type of the input value.

```python
value = "Hello World";
print type(value);

>> OSTR_TYPE

value = 1;
print type(value);

>> NMBR_TYPE

value = True;
print type(value);

>> BOOL_TYPE
```

### Print statements

`print` statement can be used to print out the content of a variable:

```python
x = "Hello ";
y = "World";

print x;
print y;

>> Hello World
```

`lprint` statement prints the content of a variable with a `\n` (newline) at the end.

```python
x = "Hello ";
y = "World";

lprint x;
lprint y;

>> Hello 
>> World
```

### Evaluating boolean expressions

CJLang's logical operators include `!`, `==`, `!=`, `<`, `>`, `<=`, `>=`, and they will result in a boolean type value.

Comparison across value type is supported:
```python
a = 1;
b = "Hello World";
print a == b;

>> False
```
Note that `<`, `>`, `<=`, `>=` only support number type values:
```python
a = 1;
b = 10;
print a < b;

>> True

b = "Hello World";
print a < b;

>> Cannot perform binary operation on values of different types.
```

The negation operator `!` can only be used on boolean type values:
```python
a = True;
print !a;

>> False

a = 1;
print !a;

>> Error at '!': Expect expression.
```

### Evaluating numerical expressions

CJLang's numerical operators include `+`, `-`, `*`, `/`, `^`, (`+=`, `-=`, `*=`, `/=`, `^=`). Each of these numerical operators take two number type 
values and results in a single number type value.

```python
a = (1 + 1) - 2 * 1 / 0.5 * -0.5;
print a;

>> 4
```

### String Operations

CJLang provides `+`, (`+=`) for concatenating two strings and `len()` function for checking length of string values.

```python
a = "Hello ";
a += "World";

lprint a;

>> Hello world

print len(a);

>> 11
```

### Grouping

During operation, a single line containing multiple expressions will be evaluated according to their precedence order.

`=` << `or` << `and` << `==`,`!=` << `<`,`>`,`<=`,`>=` << `+`,`-` << `*`,`/` << `!`,`-` << `.`,`()` 

```python
x = False or True and 1 == (1 + 1) * (1 / 2);
print x;

>> True;
```

### If, Else conditions

`if` statement can be used to craft the control flow of your program:

```python
if (True) {
    print "Hello World";
}

>> Hello World
```

Sample `if`, `else` statement:

```python
if (False){
    print "Hello World";
} else {
    print 1;
}

>> 1
```

### While loop

The `while` loop executes infinitely until its exit condition has been reached:

```python
x = 1;

while (x < 5) {
    lprint x;
    x += 1;
}

>> 1
>> 2
>> 3
>> 4
```

### For loop

The `for` loop can be used to execute a piece of script for a number of times. It contains a exit condition and an 
increment operation. Note that CJLang's for loop does not initiate a variable. Therefore, the exit condition is based on
pre-existing variables.

```python
i = 0;

for (i < 5; i+=1;) {
    lprint i;
}

>> 0
>> 1
>> 2
>> 3
>> 4
```

Sample 2D loop:

```python
i = 0;

for (i < 2; i+=1;) {
    
    j = 0;

    for (j < 2; j+=1;) {
        print i;
        print ", ";
        lprint j;
    }
}

>> 0, 0
>> 0, 1
>> 1, 0
>> 1, 1
```

### Functions

Functions are supported in CJLang, without the need to declare return type or void. CJLang supports a single variable as
a function's `return` variable. Like python, `def` keyword is used to define a function.

```python
def increment(x) {
    return x + 1;
}

print increment(1);

>> 2
```

When a `return` variable is seeked from a function without `return` variable, a `None` variable will be gained.

```python
def incrementx() {
    x += 1;
}

x = 1;

lprint incrementx();

>> None

print x;

>> 2
```

### Recursion

Recursion is supported in CJLang

Sample recursive fibonacci code:

```python
def fib(n) {
    if (n <= 1) {
        return n;
    }
    return fib(n - 1) + fib(n - 2);
}

lprint fib(30);

>> 832040
```
