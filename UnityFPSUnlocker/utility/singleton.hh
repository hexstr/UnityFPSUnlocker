#ifndef SINGLETON_HEADER
#define SINGLETON_HEADER

template <typename T>
class Singleton {
public:
    Singleton(const Singleton&) = delete;
    void operator=(const Singleton&) = delete;

    static T& GetInstance() {
        static T instance;
        return instance;
    }

protected:
    Singleton() {}
    virtual ~Singleton() {}
};

#endif // singleton.hh