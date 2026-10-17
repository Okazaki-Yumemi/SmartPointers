#ifndef UNIQUE_PTR
#define UNIQUE_PTR

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

#endif // UNIQUE_PTR
