

#ifndef KELBON_PREPROCESSOR_HPP
#define KELBON_PREPROCESSOR_HPP

#if _DEBUG
#define on_debug(expression) expression
#else
#define on_debug(expression)
#endif

#define TEST(name) void name##test()

#endif // !KELBON_PREPROCESSOR_HPP

