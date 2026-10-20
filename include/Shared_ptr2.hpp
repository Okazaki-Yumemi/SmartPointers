#ifndef SHARED_PTR
#define SHARED_PTR
#include <utility>

template <typename T>
class UniquePtr {
  private:
    // Add whatever you want to add.
    T* ptr;
  public:
    // Constructors
    UniquePtr();                          // Default constructor
    explicit UniquePtr(T* p);             // Constructor from raw pointer
    UniquePtr(UniquePtr& other) = delete; // Copy constructor is deleted
    UniquePtr(UniquePtr&& other) noexcept;         // Move constructor

    // Destructor
    ~UniquePtr();

    // Assignment operators
    UniquePtr& operator=(UniquePtr& other) = delete; // Copy assignment operator is deleted
    UniquePtr& operator=(UniquePtr&& other) noexcept; // Move assignment operator

    // Basic operations
    T* get() const;                 // Get the raw pointer
    explicit operator bool() const; // Check if the pointer is not null
    T& operator*() const;           // Dereference operator
    T* operator->() const;          // Arrow operator
    void reset();                   // Reset to empty state
    void reset(T* p);               // Reset with a new raw pointer
    T* release();                   // Release ownership of the managed object
};


template <typename T>
UniquePtr<T>::UniquePtr(){
  ptr = nullptr;
}

template <typename T>
UniquePtr<T>::UniquePtr(T* p){
  ptr = p;
}

// no copy.

template <typename T>
UniquePtr<T>::UniquePtr(UniquePtr&& other) noexcept :ptr(other.ptr){
    other.ptr = nullptr;
}

template <typename T>
UniquePtr<T>::~UniquePtr(){
  delete ptr;
  ptr = nullptr;
}

// no copy

template <typename T>
UniquePtr<T>& UniquePtr<T>::operator=(UniquePtr&& other) noexcept{
  if (this != &other) {
    delete ptr;
    ptr = other.ptr;
    other.ptr = nullptr;
  }
  return *this;
}

template <typename T>
T* UniquePtr<T>::get() const{
  return ptr;
}

template <typename T>
UniquePtr<T>::operator bool() const {
  return ptr != nullptr;
}

template <typename T>
T& UniquePtr<T>:: operator* ()const{
  return *ptr;
}

template <typename T>
T* UniquePtr<T>:: operator ->() const{
  return ptr;
}

template <typename T>
void UniquePtr<T>:: reset(){
  delete ptr;
  ptr = nullptr;
}

template <typename T>
void UniquePtr<T>:: reset(T* p){
  if(ptr == p){
    return;
  }
  delete ptr;
  ptr = p;
}

template <typename T>
T* UniquePtr<T>:: release(){
  T* return_ptr = ptr;
  ptr = nullptr;
  return return_ptr;
}

// make_unique function
template <typename T, typename... Args>
UniquePtr<T> make_unique(Args&&... args){
  return UniquePtr<T>(new T(std::forward<Args>(args)...));
}



template <typename T>
struct ControlBlock{
    T* ptr;
    int strong_cnt;
    int weak_cnt;

    explicit ControlBlock(T *p){
        ptr = p;
        strong_cnt = 1;
        weak_cnt = 1;
    }
};

template<typename T>
class WeakPtr;


template <typename T>
class SharedPtr {
  private:
    // Add whatever you want to add.
    ControlBlock<T>* ctrl = nullptr;

    template<typename U>
    friend class WeakPtr;
    
    void release_owner() noexcept;

  public:
    // Constructors
    SharedPtr();                         // Default constructor
    explicit SharedPtr(T* p);                     // Constructor from raw pointer
    SharedPtr(const SharedPtr& tmp);     // Copy constructor
    SharedPtr(SharedPtr&& tmp) noexcept; // Move constructor
    SharedPtr(UniquePtr<T>& tmp);
    SharedPtr(UniquePtr<T>&& tmp);
    // How about constructor from WeakPtr?

    // Destructor
    ~SharedPtr();

    // Assignment operators
    SharedPtr& operator=(const SharedPtr& temp);     // Copy assignment operator
    SharedPtr& operator=(SharedPtr&& temp) noexcept; // Move assignment operator

    // Basic operations
    int use_count() const;       // Get the number of shared owners
    T* get() const;              // Get the raw pointer
    explicit operator bool() const;       // Check if the pointer is not null
    T& operator*() const;        // Dereference operator
    T* operator->() const;       // Arrow operator
    void reset();          // Reset to empty state
    void reset(T* p);      // Reset with a new raw pointer
};


template <typename T>
void SharedPtr<T>:: release_owner() noexcept{
    if(ctrl == nullptr){
        return;
    }

    ControlBlock<T>* curr_ctrl = ctrl;
    ctrl = nullptr;

    curr_ctrl -> strong_cnt --;
    if(curr_ctrl->strong_cnt == 0){
        T* p = curr_ctrl->ptr;
        curr_ctrl ->ptr = nullptr;
        delete p;

        curr_ctrl-> weak_cnt --;

        if(curr_ctrl -> weak_cnt == 0){
            delete curr_ctrl;
        }
    }

}

//do nothing
template <typename T>
SharedPtr<T>:: SharedPtr(){}


template<typename T>
SharedPtr<T>:: SharedPtr(T* p){
    if(p != nullptr){
        ctrl = new ControlBlock(p);
    }else{
        ctrl = nullptr;
    }
}

template<typename T>
SharedPtr<T>:: SharedPtr(const SharedPtr& tmp){
    // do nothing
    if(tmp.ctrl == nullptr){
        ctrl = nullptr;
    }else{
        this->ctrl = tmp.ctrl;
        this->ctrl->strong_cnt ++;
    }
}

template <typename T>
SharedPtr<T>:: SharedPtr(SharedPtr&& tmp)noexcept{
    //右值构造
    this->ctrl = tmp.ctrl;
    tmp.ctrl = nullptr;
}

template <typename T>
SharedPtr<T>::SharedPtr(UniquePtr<T>& tmp){
    if(tmp.get() == nullptr){
        ctrl = nullptr;
        return;
    }
    ControlBlock<T>* block = new ControlBlock<T>(tmp.get());
    tmp.release();
    ctrl = block;
}

template <typename T>
SharedPtr<T>::SharedPtr(UniquePtr<T>&& tmp):SharedPtr(tmp) {}

template <typename T>
SharedPtr<T>:: ~SharedPtr(){
    this->release_owner();
}

template <typename T>
SharedPtr<T>& SharedPtr<T>::operator=(const SharedPtr& temp){
    if(this == & temp){
        return *this;
    }

    //取得新的
    ControlBlock<T>* new_block = temp.ctrl;
    if(new_block != nullptr){
        new_block->strong_cnt ++;
    }
    release_owner();
    ctrl = new_block;

    return *this;
}

template <typename T>
SharedPtr<T>& SharedPtr<T>::operator=(SharedPtr&& temp)noexcept{
    if(this == & temp){
        return *this;
    }

    ControlBlock<T>* new_block = temp.ctrl;

    temp.ctrl = nullptr;

    release_owner();
    ctrl = new_block;

    return *this;
}

template <typename T>
int SharedPtr<T>:: use_count()const{
    if(ctrl == nullptr){
        return 0;
    }else{
        return ctrl->strong_cnt;
    }
}

template <typename T>
T* SharedPtr<T>:: get() const{
    if(ctrl != nullptr){
        return ctrl->ptr;
    }else{
        return nullptr;
    }
    
}

template <typename T>
SharedPtr<T>::operator bool() const{
    if(ctrl == nullptr){
        return false;
    }
    return (ctrl -> ptr) != nullptr;
}

template <typename T>
T& SharedPtr<T>:: operator *() const{
    return *(ctrl->ptr);
}

template <typename T>
T* SharedPtr<T>:: operator-> ()const{
    return ctrl->ptr;
}

template <typename T>
void SharedPtr<T>:: reset(){
    release_owner();
}

template <typename T>
void SharedPtr<T>:: reset(T* p){
    
    if( get() == p){
        return;
    }
    SharedPtr<T> temp(p);
    *this = std::move(temp);
}


template <typename T, typename... Args>
SharedPtr<T> make_shared(Args&&... args){
  return SharedPtr<T>(new T(std::forward<Args>(args)...));
}


// Then implement the make_shared function
// I think I'd better leave you to design the prototype :)

// template <?>
//? make_shared(?);

#endif // SHARED_PTR