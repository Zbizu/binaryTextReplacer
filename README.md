# About
This program is a simple replacer of text. It uses a very simple custom scripting language. All possible operations can be found in example input file.

# How to run
To see how to use this program, view `launcher.bat`.

# How to compile
The program should compile in Code::Blocks or MS Visual Studio as easily as "hello world" kind of programs. It doesn't use anything other than standard libraries.

If you run into issues in C::B, enable C++11.

# Script syntax
```
-- this is a comment
fileName: "example.php" --this file will be opened

-- this is how you can replace strings
replace: "aaa"
to: "bbb" -- if longer than replace it wont work

-- this is how you can replace values
replaceChars: "3"
at: "bbb="
to: "cc" -- if longer than replaceChars it wont work

-- this is how you can replace binary data
replace: "%%%%74 65 73 74" --test
to: "%%%%61 62 63 64" -- abcd
```
