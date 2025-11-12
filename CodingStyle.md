# C++ Coding Standard

This document outlines the C++ coding standards and best practices observed in the codebase. Adhering to these guidelines ensures consistency, readability, and maintainability.

---

#### **1. File and Code Structure**

1.1. **Header Comments**:

**C++ File Naming**:
* Use `.cc` for implementation files and `.hh` for headers.
* C++ file names should use `PascalCase` (e.g., `SdkConfig.cc`, `HttpServer.hh`).
* Avoid snake_case in C++ filenames to distinguish them clearly from C modules.

1.2. Every `.cc` and `.hh` file must begin with a standard copyright and license header.

```cpp
/*
 * Copyright (C) 2025 PINGGY TECHNOLOGY PRIVATE LIMITED
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
```

1.3. **Include Order**:
Includes should be grouped in the following order to improve clarity and avoid hidden dependencies:
1.  The corresponding header for the implementation file (e.g., `SdkConfig.cc` includes `SdkConfig.hh`).
2.  Project-local headers (e.g., `<utils/StringUtils.hh>`).
3.  Third-party library headers (e.g., `nlohmann_json.hh`).
4.  Standard library headers (e.g., `<memory>`, `<vector>`).

1.4. **Namespaces**:

All code should be encapsulated within a project-specific namespace (e.g., `sdk`). Avoid `using namespace` in header files.

```cpp
namespace sdk
{
// ... code ...
} // namespace sdk
```

1.5. **C Files**:
For pure C modules, the following conventions apply:
* **File Extensions**: Use `.c` for implementation files and `.h` for header files.
* **File Names**: Use `snake_case` for all file names (e.g., `network_utils.c`, `http_parser.h`).
* **Function and Variable Names**: Use `snake_case` consistently for all function and variable names.
* **Constants and Macros**: Continue to use `UPPER_SNAKE_CASE`.
* **Struct and Type Names**: Use `PascalCase` for struct and typedef names.
* The same indentation, brace, and include-order rules as C++ apply.

---

#### **2. Naming Conventions**

2.1. **Types (Classes, Structs, Enums)**: Use `PascalCase`.
    *   `class SDKConfig`
    *   `struct HeaderMod`
    *   `enum class Action`
    *   **Abstract Classes**: Pure virtual (abstract) classes should be declared using `abstract class` instead of `class` for clarity.
        ```cpp
        abstract class SdkEventHandler: virtual public pinggy::SharedObject
        {
            // ... pure virtual or virtual methods ...
        };
        ```

2.2. **Functions and Methods**: Use `PascalCase`.
    *   **Public/Protected**: Use `PascalCase` (e.g., `GetMode`, `SetArguments`).
    *   **Private**: Use `camelCase` (e.g., `validate`, `resetArguments`).

2.3. **Variables**:
    *   **Member Variables**:
        *   **Naming**: Use `PascalCase` for public/protected members and `camelCase` for private members.
        *   **Alignment**: The variable name must start at column 33. If the data type is too long for the name to start on the same line, the name should be placed on the next line, also starting at column 33.
            ```cpp
            struct MyStruct
            {
                bool                        myFlag;
                int                         count;
                std::vector<tString>        items;
                VeryLongTypeNameThatIsTooLong
                                            *myPointer;
            };
            ```
    *   **Local Variables**: Use `camelCase`.
        *   `auto cmds = ShlexSplitString(args);`
        *   `tString whitelist = "w:";`
    *   **Note**: The distinction between public/private member variables is an ideal to strive for. Existing code primarily uses `camelCase` for all member variables. New code should follow the public/private distinction.

2.4. **Constants and Macros**: Use `UPPER_SNAKE_CASE`.
    *   `#define MAX_RECONNECTION_TRY 20`

2.5. **Enum Members**: Use `PascalCase` if it is public other wise use `camelCase`
    *   `HeaderMod::Action::Add`
    *   `HeaderMod::Action::Unknown`

2.6. **Type Aliases**: Prefix with a `t`.
    *   `tString` (likely an alias for `std::string`)

---

#### 3. Function Definition and Declaration

3.1. **Function Signature Formatting**:
Function definitions and declarations must follow a specific layout for consistency:
*   The return type is placed on its own line.
*   The function name (including the class scope) starts at column 0 on the next line.

```cpp
// Declaration in a .hh file
class MyClass
{
public:
    tString
    MyFunction(int argument);

    const tInt
    ASingleLineFunction()       { return something; }

    const tInt
    ASingleLineFunctionWithLongName()
                                { return something; }
};

// Definition in a .cc file
tString
MyClass::MyFunction(int argument) // Multi-line body
{
    // ... function body ...
}
```

#### **4. Formatting and Style**

4.1. **Braces**: Use the "One True Brace Style" (1TBS). The opening brace goes on the same line as the statement.

```cpp
void
SDKConfig::validate()
{
    if (!serverAddress) {
        serverAddress = NewUrlPtr("a.pinggy.io:443");
    } else {
        serverAddress = NewUrlPtr(serverAddress);
    }
}
```

4.2. **Indentation**: Use 4 spaces for indentation. Do not use tabs.

4.3. **Pointers and References**:
* Place the asterisk (`*`) or ampersand (`&`) **next to the variable name**, not the type.
    * `Correct`: `tString *myString;`
    * `Incorrect`: `tString* myString;`
* **References should generally be avoided.** Their use will be considered **on a case-by-case basis**.
    * Prefer smart pointers or passing by value when possible, as they make ownership and lifetime clearer.
* **Exception for Function Return Types**: For function definitions where the return type is on its own line, the pointer or reference symbol may stay with the type on the preceding line.
```cpp
const tString *
SDKConfig::GetSomePointer();
```
4.4. **`const` Correctness**: Use `const` wherever possible for member functions that do not modify object state and for variables that should not be changed.

```cpp
const tString
SDKConfig::GetHeaderManipulations() const;
```

---

#### **5. C++ Features and Idioms**

5.1. **Smart Pointers and Factories**:
*   **Concrete Classes**: For any instantiable class or struct that will be managed by a `shared_ptr`, use the `DefineMakeSharedPtr(ClassName)` macro after the class definition. This creates the necessary pointer type aliases and a `NewClassNamePtr()` factory function.
*   **Abstract Classes**: For abstract classes, use the `DeclareSharedPtr(ClassName)` macro instead. This provides the pointer type aliases without creating a factory function, as abstract classes cannot be instantiated directly.

5.2. **Enums**: Use `enum class` for strongly-typed, scoped enumerations instead of plain `enum`.

5.3. **Type Inference**: Use `auto` to simplify variable declarations where the type is clear from the initializer, especially with iterators and complex template types.

```cpp
for (auto cmd : cmds) {
    // ...
}
```

4.4. **Macros**: Use macros sparingly. For simple, repeated code blocks, function-like macros with a `do-while(0)` wrapper are acceptable.

```cpp
#define TO_JSON_STR(x, y)       \
    do {                        \
        json j = x;             \
        y = j.dump();           \
    } while(0)
```
**Recommendation**: For new code, consider replacing such macros with inline functions or templates to improve type safety and debugging.

4.5. **String Manipulation**: Utilize helper functions from `<utils/StringUtils.hh>` for common operations like splitting and joining strings.

4.6. **Error Handling**: Use exceptions for reporting configuration or runtime errors. Define custom exception classes (e.g., `SdkConfigException`) for clarity.

---

#### **5. Code Example**

The following snippet demonstrates several of the rules above:

```cpp
// 2.2: PascalCase method name
// 4.3: Use of auto
void
SDKConfig::SetArguments(tString args)
{
    // 2.3: camelCase local variable
    auto cmds = ShlexSplitString(args);

    // 4.3: Range-based for loop with auto
    for (auto cmd : cmds) {
        auto vals = SplitString(cmd, ":", 1);
        // 3.1: 1TBS for if statement
        if (vals.size() < 2 || vals[0].empty())
            continue;

        auto cmdType = cmd.at(0);
        switch(cmdType) {
            case 'a':
                {
                    // ... logic ...
                }
                break;
            // ... other cases
        }
    }
}
```
