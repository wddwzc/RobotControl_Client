#ifndef VQUEUE_H
#define VQUEUE_H

#ifndef VQUE_H
#define VQUE_H

#include <vector>

#define MAX_QUEUE_SIZE 150000

template<class T>
class VQue
{
public:
    VQue();
    ~VQue();

    void push(std::vector<T>&);
    void push(const T&);
    void clear();

    int size();
    T& at(const int index);
    T& operator[](const int i);

    void resize(const int _len);//定位模式下重新调整队列大小
    int get_capacity();

private:
    bool Init();

private:
    int len;
    int rear;
    int front;
    T* base;
    bool is_success_init;//标志是否成功进行初始化
private:
    int max_size;//队列的长度

    //单例，先不用
private:
    static VQue<T>* pDG;
public:
    static VQue<T>* GetInstance();

};


template<class T>
VQue<T>* VQue<T>::pDG = 0;

template<class T>
VQue<T>::VQue()
{
    is_success_init = Init();
}

template<class T>
VQue<T>::~VQue()
{
    if(is_success_init)
        delete[] base;
    base = 0;
    rear = len = front = 0;
}

template<class T>
VQue<T> *VQue<T>::GetInstance()
{
    if(pDG == 0)
        pDG = new VPointQue;
    return pDG;
}

template<class T>
bool VQue<T>::Init()
{
    base = new T[MAX_QUEUE_SIZE];
    if(!base)
        return false;
    rear = len = front = 0;
    max_size = MAX_QUEUE_SIZE;
    return true;
}

template<class T>
void VQue<T>::resize(const int _len)
{
    is_success_init = false;
    if(base)
        delete []base;
    base = new T[_len];
    if(base)
        is_success_init = true;
    rear = len = front = 0;
    max_size = _len;
}

template<class T>
void VQue<T>::push(std::vector<T> &contents)
{
    if(false == is_success_init)
        return;
    for(int i = 0; i < contents.size(); ++i)
    {
        base[rear] = contents[i];
        rear++;
        rear = rear % max_size;
        if(len < max_size)
            len++;
    }
}

template<class T>
void VQue<T>::push(const T &t)
{
    if(false == is_success_init)
        return;

    base[rear] = t;
    rear++;
    rear = rear % max_size;
    if(len < max_size)
        len++;
}

template<class T>
int VQue<T>::size()
{
    return len;
}

template<class T>
int VQue<T>::get_capacity()
{
    return max_size;
}

template<class T>
T& VQue<T>::at(const int index)
{
    if(index<0 || index>=len) return 0;
    return base[index];
}

template<class T>
T& VQue<T>::operator[](const int i)
{
    return base[i];
}

template<class T>
void VQue<T>::clear()
{
     rear = len = front = 0;
}


#endif // VQUE_H



#endif // VQUEUE_H
