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
class SharedPtr {
  private:
    // Add whatever you want to add.
    T* ptr;
    int* cnt;
  public:
    // Constructors
    SharedPtr();                         // Default constructor
    explicit SharedPtr(T* p);                     // Constructor from raw pointer
    SharedPtr(const SharedPtr& tmp);     // Copy constructor
    SharedPtr(SharedPtr&& tmp) noexcept;          // Move constructor
    SharedPtr(UniquePtr<T>& tmp);
    SharedPtr(UniquePtr<T>&& tmp) ;
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
SharedPtr<T>:: SharedPtr(){
  ptr = nullptr;
  cnt = nullptr;
}

template <typename T>
SharedPtr<T>:: SharedPtr(T* p){
  ptr = p;
  if(p){
    cnt = new int(1);
  }else{
    cnt = nullptr;
  }
}

template <typename T>
SharedPtr<T>:: SharedPtr(const SharedPtr& tmp){
  if(tmp.ptr != nullptr){
    ptr = tmp.ptr;
  }else{
    ptr = nullptr;
  }

  if(tmp.cnt != nullptr){
    cnt = tmp.cnt;
    (*cnt) ++;
  }else{
    cnt = nullptr;
  }
}

template <typename T>
SharedPtr<T>:: SharedPtr(SharedPtr&& tmp) noexcept:ptr(tmp.ptr),cnt(tmp.cnt) {
  tmp.ptr = nullptr;
  tmp.cnt = nullptr;
}

template <typename T>
SharedPtr<T>::SharedPtr(UniquePtr<T>& tmp): ptr(nullptr), cnt(nullptr) {
  if (tmp.get() != nullptr) {
      cnt = new int(1);
      ptr = tmp.release();
  }
}


template <typename T>
SharedPtr<T>:: SharedPtr(UniquePtr<T>&& tmp) :ptr(tmp.get()){
  if(ptr != nullptr){
    cnt = new int(1);
  }else{
    cnt = nullptr;
  }
  tmp.release();
}

template <typename T>
SharedPtr<T>:: ~SharedPtr(){
  if(cnt == nullptr){
    ptr = nullptr;
    return;
  }
  //最后一个
  if(*cnt == 1){
    delete ptr;
    delete cnt;
  }else{
    ptr = nullptr;
    (*cnt) --; 
  }
}

template <typename T>
SharedPtr<T>& SharedPtr<T>::operator=(const SharedPtr& temp){
  if (this == &temp) {
      return *this;
  }

  T* new_ptr = temp.ptr;
  int* new_cnt = temp.cnt;

  // 先取得新对象的共享所有权
  if (new_cnt != nullptr) {
      ++(*new_cnt);
  }

  // 再释放旧的所有权
  reset();

  // 最后更新成员
  ptr = new_ptr;
  cnt = new_cnt;

  return *this;
}

template <typename T>
SharedPtr<T>& SharedPtr<T>::operator=(SharedPtr&& temp) noexcept{
  if(this == & temp){
    return *this;
  }

  T* new_ptr = temp.ptr;
  int* new_cnt = temp.cnt;

  //先拿走所有权
  temp.ptr = nullptr;
  temp.cnt = nullptr;

  //释放
  reset();
  ptr = new_ptr;
  cnt = new_cnt;

  return *this;
}

template <typename T>
int SharedPtr<T>:: use_count()const{
  if(cnt == nullptr){
    return 0;
  }
  return *cnt;
}

template <typename T>
T* SharedPtr<T>:: get()const{
  return ptr;
}

template <typename T>
SharedPtr<T>::operator bool() const {
  return ptr != nullptr;
}

template <typename T>
T& SharedPtr<T>:: operator*()const {
  return *ptr;
}

template <typename T>
T* SharedPtr<T>:: operator->()const {
  return ptr;
}

template <typename T>
void SharedPtr<T>:: reset(){
  if(cnt != nullptr && *cnt == 1){
    delete ptr;
    delete cnt;
    ptr = nullptr;
    cnt = nullptr;
    return;
  }
  ptr = nullptr;
  if(cnt != nullptr) (*cnt) --;
  cnt = nullptr;
}

template <typename T>
void SharedPtr<T>:: reset(T* p){
  if(ptr == p){
    return;
  }
  this ->reset();
  if(p == nullptr){
    ptr = p;
    return;
  }
  cnt = new int(1);
  ptr = p;
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