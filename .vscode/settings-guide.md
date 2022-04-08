<!-- https://member.ipmu.jp/yuji.tachikawa/style.css -->
<link href="style.css" rel="stylesheet"></link>

# Library path & compiler flag
## c_cpp_properties.json
- `includePath`
- `cppStandard`

## tasks.json
- `DCMAKE_TOOLCHAIN_FILE`
- compiler flag `-std=c++`
- compiler flag `-I`
- compiler flag `-L`

# CMakeLists.txt
- Replace `iris` with your project name.
- Replace `IRIS` with your capitalized project name.
- `target_compile_features`

# Doxyfile
- The following items have been overwritten:
```
PROJECT_NAME           = iris
PROJECT_NUMBER
PROJECT_BRIEF
PROJECT_LOGO
INPUT                  = README.md \
                         include/iris
EXAMPLE_PATH           = examples
EXAMPLE_RECURSIVE      = YES
HTML_EXTRA_STYLESHEET  = customdoxygen.css
HTML_COLORSTYLE_HUE    = 20
MACRO_EXPANSION        = YES
```
