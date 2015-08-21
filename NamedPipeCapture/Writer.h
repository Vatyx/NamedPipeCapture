#pragma once
#ifndef WRITER_H_DEFINED
#define WRITER_H_DEFINED
#include <memory>
#include <utility>

class Writer
{
public:
   friend void DoWrite(Writer& obj, std::pair<std::unique_ptr<char[]>, std::size_t>&& sendObj);
   template<typename T>
   Writer(T x) : self_(std::make_shared<model<T>>(std::move(x))) {}
private:
   struct processor_t
   {
      virtual ~processor_t() {}
      virtual void Write_(std::pair<std::unique_ptr<char []>, std::size_t>&&) = 0;
   };
   template <typename T>
   struct model : processor_t
   {
      model(T x) : data_(std::move(x)) {}
      void Write_(std::pair<std::unique_ptr<char []>, std::size_t>&& sendObj) override
      {
         DoWrite(data_, std::move(sendObj));
      }
      T data_;
   };
   std::shared_ptr<processor_t> self_;
};
inline void DoWrite(Writer& obj, std::pair<std::unique_ptr<char []>, std::size_t>&& sendObj)
{
   obj.self_->Write_(std::move(sendObj));
}
#endif