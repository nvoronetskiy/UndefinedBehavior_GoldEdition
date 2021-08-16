

#ifndef KELBON_PREPROCESSOR_HPP
#define KELBON_PREPROCESSOR_HPP

#if _DEBUG
#define on_debug(expression) do { expression } while(false)
#else
#define on_debug(expression) do { } while(false)
#endif

#endif // !KELBON_PREPROCESSOR_HPP

