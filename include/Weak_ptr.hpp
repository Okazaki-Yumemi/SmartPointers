#ifndef WEAK_PTR
#define WEAK_PTR
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
    SharedPtr(const WeakPtr<T>& tmp);

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
SharedPtr<T>::SharedPtr(const WeakPtr<T>& tmp) {
    if (tmp.ctrl != nullptr && tmp.ctrl->strong_cnt > 0) {
        ctrl = tmp.ctrl;
        ++ctrl->strong_cnt;
    } else {
        ctrl = nullptr;
    }
}

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



template <typename T>
class WeakPtr {
  private:
    // Add whatever you want to add.
    ControlBlock<T>* ctrl = nullptr;

    void release_owner() noexcept;

    template<typename U>
    friend class SharedPtr;


  public:
    // Constructors
    WeakPtr();                         // Default constructor
    WeakPtr(const SharedPtr<T>& temp); // Constructor from SharedPtr
    WeakPtr(const WeakPtr& temp);      // Copy constructor
    WeakPtr(WeakPtr&& temp) noexcept;           // Move constructor

    // Destructor
    ~WeakPtr();

    // Assignment operators
    WeakPtr& operator=(const WeakPtr<T>& temp);      // Copy assignment operator
    WeakPtr& operator=(const SharedPtr<T>& temp); // Assignment from SharedPtr
    WeakPtr& operator=(WeakPtr&& temp) noexcept;           // Move assignment operator

    // Basic operations
    int use_count() const;     // Get the number of shared owners
    SharedPtr<T> lock() const; // Get a SharedPtr to the managed object
    bool expired() const;      // Check if the managed object was deleted
    void reset();        // Reset to empty state
    explicit operator bool() const;

    // Utility functions
    void swap(WeakPtr& other); // Swap with another WeakPtr
};

// Non-member swap function
template <typename T>
void swap(WeakPtr<T>& lhs, WeakPtr<T>& rhs);

template <typename T>
void WeakPtr<T>:: release_owner() noexcept{
    if(ctrl == nullptr ){
        return;
    }

    ControlBlock<T>* curr_ctrl = ctrl;

    ctrl = nullptr;

    curr_ctrl -> weak_cnt --;
    
    //自己是最后一个control的
    if(curr_ctrl->weak_cnt == 0){
        delete curr_ctrl;
    }
}

template <typename T>
WeakPtr<T>:: WeakPtr(){
    // do nothing
}

template <typename T>
WeakPtr<T>::WeakPtr(const SharedPtr<T>& temp){
    if(temp.ctrl == nullptr){
        ctrl = nullptr;
    }else{
        ControlBlock<T>* curr_ctrl = temp.ctrl;
        curr_ctrl -> weak_cnt++;
        this->ctrl = curr_ctrl;
    }
}

template  <typename T>
WeakPtr<T>::WeakPtr(const WeakPtr& temp){
    if(temp.ctrl == nullptr){
        ctrl = nullptr;
    }else{
        ControlBlock<T>* curr_ctrl = temp.ctrl;
        curr_ctrl->weak_cnt ++;
        this->ctrl = curr_ctrl;
    }
}

template <typename T>
WeakPtr<T>::WeakPtr(WeakPtr&& temp) noexcept:ctrl(temp.ctrl) {
    temp.ctrl = nullptr;
}

template <typename T>
WeakPtr<T>:: ~WeakPtr(){
    this->release_owner();
}

template<typename T>
WeakPtr<T>& WeakPtr<T>::operator=(const WeakPtr<T>& temp){
    if(this == &temp){
        return *this;
    }

    //Get
    ControlBlock<T>* new_block = temp.ctrl;
    if(new_block != nullptr){
        new_block -> weak_cnt ++;
    }
    release_owner();
    ctrl = new_block;

    return *this;
}

template <typename T>
WeakPtr<T>& WeakPtr<T>::operator=(const SharedPtr<T>& temp){
    if(temp.ctrl == nullptr){
        release_owner();
        this->ctrl = nullptr;
        return *this;
    }
    ControlBlock<T>* new_block = temp.ctrl;
    new_block->weak_cnt ++;
    release_owner();
    this->ctrl = new_block;
    return *this;
}

template <typename T>
WeakPtr<T>&  WeakPtr<T>::operator=(WeakPtr<T>&& temp) noexcept{
    if(this == &temp){
        return *this;
    }

    ControlBlock<T>* new_block = temp.ctrl;
    temp.ctrl = nullptr;
    release_owner();
    ctrl = new_block;
    return *this;
}

template<typename T>
int WeakPtr<T>::use_count() const{
    if(ctrl == nullptr){
        return 0;
    }else{
        return ctrl->strong_cnt;
    }
}

template<typename T>
SharedPtr<T> WeakPtr<T>::lock() const{
    SharedPtr<T> result;

    if(ctrl != nullptr && ctrl->strong_cnt > 0){
        result.ctrl = this->ctrl;
        ++(ctrl->strong_cnt);
    }
    return result;
}

template<typename T>
bool WeakPtr<T>::expired()const{
    if(ctrl == nullptr){
        return true;
    }else if(ctrl->ptr == nullptr){
        return true;
    }

    return false;
}

template<typename T>
void WeakPtr<T>::reset(){
    this->release_owner();
}


template<typename T>
void WeakPtr<T>::swap(WeakPtr<T>& other){
    if(this == &other){
        return;
    }
    ControlBlock<T>* this_block = ctrl;
    ControlBlock<T>* other_block = other.ctrl;

    this->ctrl = other_block;
    other.ctrl = this_block;
}

template <typename T>
void swap(WeakPtr<T>& lhs, WeakPtr<T>& rhs){
    lhs.swap(rhs);
}

template <typename T>
WeakPtr<T>::operator bool() const{
    return !this->expired();
}

#endif // WEAK_PTR
