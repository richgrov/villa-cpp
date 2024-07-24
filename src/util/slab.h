#ifndef VKAD_UTIL_SLAB_H_
#define VKAD_UTIL_SLAB_H_

#include <algorithm>
#include <utility>
#include <vector>

namespace vkad {

static constexpr int kInvalidSlabKey = -1;

template <class T> class Slab {
public:
   explicit Slab(size_t initial_capacity) : next_available_(kInvalidSlabKey) {
      objects_.reserve(initial_capacity);
   }

   ~Slab() {
      bool in_use[objects_.size()];
      std::fill_n(in_use, objects_.size(), true);

      int next_available = next_available_;
      while (next_available != kInvalidSlabKey) {
         in_use[next_available] = false;
         next_available = get_storage(next_available).next();
      }

      for (int i = 0; i < objects_.size(); ++i) {
         if (in_use[i]) {
            release(i);
         }
      }
   }

   [[nodiscard]] T &get(const int index) {
      return get_storage(index).value();
   }

   template <class... Args> int emplace(Args &&...args) {
      if (next_available_ == kInvalidSlabKey) {
         int key = objects_.size();
         objects_.emplace_back();
         get_storage(key).store_value(std::forward<Args>(args)...);
         return key;
      }

      int key = next_available_;
      auto &storage = get_storage(key);
      next_available_ = storage.next();
      storage.store_value(std::forward<Args>(args)...);
      return key;
   }

   void release(const int key) {
      auto &storage = get_storage(key);
      storage.call_value_destructor();
      storage.store_next(next_available_);
      next_available_ = key;
   }

private:
   struct Storage {
      static constexpr std::size_t kSize = std::max<std::size_t>({sizeof(int), sizeof(T)});
      static constexpr std::size_t kAlign = std::max<std::size_t>({alignof(int), alignof(T)});
      alignas(kAlign) unsigned char storage[kSize];

      int next() {
         auto ptr = reinterpret_cast<int *>(&storage);
         return *ptr;
      }

      void store_next(const int next) {
         auto ptr = reinterpret_cast<int *>(&storage);
         *ptr = next;
      }

      template <class... Args> void store_value(Args &&...args) {
         new (&storage) T(std::forward<Args>(args)...);
      }

      T &value() {
         auto ptr = reinterpret_cast<T *>(&storage);
         return *ptr;
      }

      void call_value_destructor() {
         auto ptr = reinterpret_cast<T *>(&storage);
         ptr->T::~T();
      }
   };

   Storage &get_storage(const int index) {
      return objects_[index];
   }

   std::vector<Storage> objects_;
   int next_available_;
};

} // namespace vkad

#endif // !VKAD_UTIL_SLAB_H_