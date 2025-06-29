#pragma once

#include <cstddef>
#include <cstdint>
#include <algorithm>
#include <iterator>
#include <list>
#include <type_traits>
#include <initializer_list>

template<typename T>
struct is_like_string {
private:
    template<typename U>
    static auto test(int) -> decltype(std::declval<U>().at(), std::declval<U>().size(), std::declval<U>().data(), std::true_type());

    template<typename>
    static std::false_type test(...);

public:
    static constexpr const bool value = decltype(test<T>(0))::value;
};


template<typename CharType>
std::size_t strlen(const CharType* chr) {
    const CharType* it = chr-1;
    while (*(++it));
    return it - chr;
}


template<typename CharType>
struct SharedStringData final {

    constexpr static const std::size_t npos = (std::size_t)(-1);

    SharedStringData() = delete;
    ~SharedStringData() = delete;


    static SharedStringData* create(std::size_t length) {
        std::size_t alloc_size = sizeof(SharedStringData) + length * sizeof(CharType);
        SharedStringData* object = (SharedStringData*) (new std::int8_t[alloc_size]);
        object->count = length;
        object->references_count = 0;
        object->owner = NULL;
        object->data = (CharType*)(object+1);
        return object;
    }


    static SharedStringData* create(const CharType* data, std::size_t length=npos) {
        SharedStringData* object = create((std::size_t)0);
        object->data = const_cast<CharType*>(data);
        object->owner = object; // modifications not allowed
        return object;
    }


    static SharedStringData* create(const CharType* begin, const CharType* end) {
        return create(begin, end-begin);
    }


    static void dispose(SharedStringData* object) {
        delete [] ((std::int8_t*)object);
    }


    SharedStringData* add_reference() {
        references_count++;
        return this;
    }


    static void remove_reference(SharedStringData* object, void* owner) {
        if (object->owner == owner) owner = NULL;
        if (!(--(object->references_count))) dispose(object);
    }


    bool set_owner(void* new_owner) {
        if (owner) return owner == new_owner;
        if (!new_owner) return false;
        owner = new_owner;
        return true;
    }


    std::size_t count;
    std::size_t references_count;
    void* owner;
    CharType* data;

};


template<typename CharType = char>
class SharedString {

    public:

        using DataStruct = SharedStringData<CharType>;
        constexpr static const std::size_t npos = DataStruct::npos;


        explicit SharedString(std::size_t size) {
            data_struct = DataStruct::create(size)->add_reference();
            count = 0;
            data_ptr = data_struct->data;
        }


        SharedString(const std::initializer_list<CharType>& init_list) {
            std::size_t size = init_list.size();
            data_struct = DataStruct::create(size)->add_reference();
            count = size;
            data_ptr = data_struct->data;
            std::size_t i = 0;
            for (const auto chr : init_list)
                data_ptr[i++] = chr;
        }


        template<typename InputIt, typename std::iterator_traits<InputIt>::difference_type* = nullptr>
        SharedString(InputIt& first, InputIt& last) {
            std::size_t size = std::distance(first, last);
            data_struct = DataStruct::create(size)->add_reference();
            count = size;
            data_ptr = data_struct->data;
            auto it = first;
            std::size_t i = 0;
            for (std::size_t i=0;i<size;i++,it=std::next(it)) {
                data_ptr[i] = *it;
            }
        }


        SharedString(const CharType* ptr, std::size_t count = npos) {
            if (count) count = strlen(ptr);
            data_struct = DataStruct::create(ptr, count)->add_reference();
            this->count = count;
            data_ptr = data_struct->data;
        }


        template<typename T, typename std::enable_if<is_like_string<T>::value, void>::type* = nullptr>
        SharedString(const T& other) : SharedString(other.data(), other.size()) {}


        SharedString(const char* begin, const char* end) {
            data_struct = DataStruct::create(begin, end)->add_reference();
            count = end - begin;
            data_ptr = data_struct->data;
        }


        SharedString(DataStruct* data_struct, std::size_t offset, std::size_t count) {
            this->data_struct = data_struct->add_reference();
            this->count = count;
            this->data_ptr = data_struct->data + offset;
        }


        SharedString() : SharedString(32) {}


        SharedString(const SharedString& other) {
            this->data_struct = other.data_struct->add_reference();
            this->count = other.count;
            this->data_ptr = other.data_ptr;
        }


        SharedString(SharedString&& other) {
            this->data_struct= other.data_struct;
            this->count = other.count;
            this->data_ptr = other.data_ptr;
            if (this->data_ptr->owner == &other) this->data_ptr->owner = this;
            other.data_struct = NULL;
            other.count = 0;
            other.data_ptr = NULL;
        }


        ~SharedString() {
            if (data_struct) DataStruct::remove_reference(data_struct, this);
        }


        bool is_mutable() const {
            return data_struct->owner == this;
        }


        void make_mutable() {
            if (data_struct->set_owner(this)) return;
            reserve(count);
        }


        void reserve(std::size_t allocate_count=0) {
            if (allocate_count <= data_struct->count && data_struct->set_owner(this)) return;
            DataStruct* new_data_struct = DataStruct::create(std::max<std::size_t>(count, allocate_count));
            new_data_struct->owner = this;
            for (std::size_t i=0;i<count;i++)
                new_data_struct->data[i] = data_ptr[i];
            data_ptr = new_data_struct->data;
            DataStruct::remove_reference(data_struct, this);
            data_struct = new_data_struct->add_reference();
        }


        std::size_t capacity() const {
            return data_struct->count;
        }


        void push_back(const CharType* str, std::size_t add_count=1) {
            std::size_t new_count = count + add_count;
            if (!data_struct->set_owner(this) || new_count > data_struct->count)
                reserve(new_count * 2);
            for (std::size_t i=count,j=0;j<add_count;i++,j++)
                data_ptr[i] = str[j];
            count = new_count;
        }


        void push_back(const CharType chr) {
            push_back(&chr, 1);
        }


        CharType pop_back() {
            if (__glibc_unlikely(!count)) return 0;
            return data_ptr[--count];
        }


        std::size_t size() const noexcept {
            return count;
        }


        std::size_t length() const noexcept {
            return count;
        }


        SharedString substr(std::size_t position, std::size_t char_count=npos) const {
            std::size_t begin = std::min(position, count-1);
            std::size_t end = std::min(position + char_count, count);
            return SharedString(data_struct, data_ptr - data_struct->data + begin, end - begin);
        }


        CharType at(std::size_t position) const {
            return data_struct->data[position + position];
        }


        CharType& operator[](std::size_t position) const {
            return data_ptr[position];
        }


        CharType* data() const {
            return data_ptr;
        }


        const CharType* c_str() {
            CharType* zero_ptr = data_ptr + count;
            if (data_struct->data + data_struct->count <= zero_ptr || *zero_ptr) {
                push_back((char)0);
                count--;
            }
            return data();
        }


        std::size_t references_count() const {
            return data_struct->references_count;
        }


        int compare(const CharType* data, std::size_t length=npos) const {
            if (length == npos) length = strlen(data);
            std::size_t loop_end = std::min(count, length);
            for (std::size_t i=0;i<loop_end;i++) {
                int diff = (int)data_ptr[i] - (int)data[i];
                if (diff) return (diff > 0) ? 1 : -1;
            }
            return count == length ? 0 : count < length ? -1 : 1;
        }


        template<class T>
        typename std::enable_if<is_like_string<T>::value, int>::type compare(const T& other) const {
            return compare(other.data(), other.size());
        }


        template<class T>
        static typename std::enable_if<is_like_string<T>::value && !std::is_same<T, SharedStringData<CharType>>::value, SharedString>::type
        concat(const SharedStringData<CharType>& a, const T& b) {
            SharedString result(a);
            result.push_back(b.data(), b.size());
            return result;
        }


        template<class T>
        static typename std::enable_if<is_like_string<T>::value && !std::is_same<T, SharedStringData<CharType>>::value, SharedString>::type
        concat(const T& a, const SharedStringData<CharType>& b) {
            std::size_t a_size = a.size();
            std::size_t b_size = b.size();
            std::size_t count = a_size + b_size;
            SharedString result(count);
            result.count = count;
            std::size_t i = 0;
            for (std::size_t j=0;j<a_size;j++)
                result.data_ptr[i++] = a[j];
            for (std::size_t j=0;j<b_size;j++)
                result.data_ptr[i++] = b[j];
            return result;
        }


        void clear() {
            count = 0;
        }


        bool is_placed_in(std::size_t position, const char* needle, std::size_t length=npos) const {
            if (length == npos) length = strlen(needle);
            if (position + length > count) return 0;
            char* data_pos = data_ptr + position;
            for (std::size_t i=0;i<length;i++)
                if (data_pos[i] != needle[i]) return false;
            return true;
        }


        template<class T>
        typename std::enable_if<is_like_string<T>::value, bool>::type is_placed_in(std::size_t position, const T& needle) const {
            return is_placed_in(position, needle.data(), needle.size());
        }


        std::size_t find(const char* needle, std::size_t start_position=0, std::size_t length=npos) const {
            if (length == npos) length = strlen(needle);
            if (length < count) return npos;
            std::size_t end_position = count - length;
            for (std::size_t i=start_position;i<end_position;i++)
                if (is_placed_in(i, needle, length)) return i;
            return npos;
        }


        template<class T>
        typename std::enable_if<is_like_string<T>::value, std::size_t>::type find(const T& needle, std::size_t start_position=0) const {
            return find(needle.data(), start_position, needle.size());
        }


        std::size_t rfind(const char* needle, std::size_t start_position=npos, std::size_t length=npos) const {
            if (length == npos) length = strlen(needle);
            if (start_position > count) start_position = count;
            if (start_position < length) return npos;
            start_position -= length;
            std::size_t end_position = count - length;
            for (std::size_t i=start_position;i!=(std::size_t)(-1);i--)
                if (is_placed_in(i, needle, length)) return i;
            return npos;
        }


        template<class T>
        typename std::enable_if<is_like_string<T>::value, std::size_t>::type rfind(const T& needle, std::size_t start_position=0) const {
            return rfind(needle.data(), start_position, needle.size());
        }


        bool contains(const char* needle, std::size_t length=npos) const {
            return find(needle, 0, (length==npos)?strlen(needle):length) != npos;
        }


        template<class T>
        typename std::enable_if<is_like_string<T>::value, bool>::type contains(const T& needle) const {
            return contains(needle.data(), needle.size());
        }


        bool starts_with(const char* needle, std::size_t length=npos) const {
            return is_placed_in(0, needle, (length==npos)?strlen(needle):length);
        }


        template<class T>
        typename std::enable_if<is_like_string<T>::value, bool>::type starts_with(const T& needle) const {
            return starts_with(needle.data(), needle.size());
        }


        bool ends_with(const char* needle, std::size_t length=npos) const {
            return is_placed_in(count - length, needle, (length==npos)?strlen(needle):length);
        }


        template<class T>
        typename std::enable_if<is_like_string<T>::value, bool>::type ends_with(const T& needle) const {
            return ends_with(needle.data(), needle.size());
        }



        std::list<SharedString<CharType>> split(const char* separator, std::size_t separator_length=npos, std::size_t limit=(std::size_t)(-1)) const {
            std::list<SharedString<CharType>> result;
            if (separator_length == npos) separator_length = strlen(separator);
            if (separator_length) {
                std::size_t result_count = 0;
                char* start_position = data_ptr;
                const char* data_end = data_ptr + count - separator_length;
                for (char* data_it = data_ptr; data_it < data_end; ) {
                    std::size_t i;
                    for (i=0;i<separator_length;i++)
                        if (separator[i] != data_it[i]) break;
                    if (i==separator_length) {
                        result.emplace_back(start_position, data_it+1);
                        data_it += separator_length;
                        start_position = data_it;
                        if (result_count++ >= limit) break;
                    } else data_it++;
                }
                result.emplace_back(start_position, data_ptr + count);
            }
            return result;
        }



    private:

        CharType* data_ptr;
        std::size_t count;
        DataStruct* data_struct;

};
