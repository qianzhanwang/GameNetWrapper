#ifndef _H_PATTERN_SINGLETON_H_
#define _H_PATTERN_SINGLETON_H_

template <typename T>
class TSingleton
{
public:
    TSingleton() {}
    virtual ~TSingleton() {}

    static T& GetInstance()
    {
        if (s_pInstance == nullptr)
        {
            s_pInstance = new T;
        }
        return *s_pInstance;
    }

protected:
    static T* s_pInstance;

private:
    TSingleton<T>* operator = (const TSingleton<T>&);
};

template <typename T>
T* TSingleton<T>::s_pInstance = nullptr;

#endif //_H_PATTERN_SINGLETON_H_