#ifndef __SINGLETON__
#define __SINGLETON__

#include <QtGlobal> // pour Q_DISABLE_COPY

#define DECLARE_SINGLETON(Class)            \
    Q_DISABLE_COPY(Class)                   \
    Class(Class&&) = delete;                \
    Class& operator=(Class&&) = delete;     \
public:                                     \
    static Class& instance()                \
    {                                       \
        static Class _instance;             \
        return _instance;                   \
    }                                       \
private:                                    \
    Class() = default;

#endif
