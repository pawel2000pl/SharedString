#pragma once

#include <cstddef>
#include <cstdint>
#include <algorithm>
#include <iterator>
#include <compare>
#include <list>
#include <type_traits>
#include <initializer_list>


template<typename T, typename CharType = char>
struct is_like_string {
private:
    template<typename U>
    static auto test(int) -> std::integral_constant<bool, 
        std::is_same<typename std::remove_cv<typename std::remove_reference<decltype(std::declval<const U>().at(0))>::type>::type, CharType>::value && 
        std::is_integral<decltype(std::declval<const U>().size())>::value &&
        std::is_same<typename std::remove_cv<typename std::remove_pointer<decltype(std::declval<const U>().data())>::type>::type, CharType>::value &&
        std::is_same<decltype(U(std::declval<const CharType*>(), std::declval<std::size_t>())), U>::value &&
        std::is_signed<decltype(std::declval<U>().compare(std::declval<std::size_t>(), std::declval<std::size_t>(), std::declval<const CharType*>()))>::value
    >;

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
        object->data = (CharType*)(object+1);
        return object;
    }


    static SharedStringData* create(const CharType* data, std::size_t length=npos) {
        SharedStringData* object = create((std::size_t)0);
        object->data = const_cast<CharType*>(data);
        object->count = length == npos ? strlen(data)+1 : length;
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


    static void remove_reference(SharedStringData* object) {
        if (!(--(object->references_count))) dispose(object);
    }


    bool is_mutable() const {
        return (void*)data == (void*)(this+1);
    }


    std::size_t count;
    std::size_t references_count;
    CharType* data;

};


template<typename CharType = char>
class SharedString {

    public:

        using DataStruct = SharedStringData<CharType>;
        constexpr static const std::size_t npos = DataStruct::npos;


        class ElementProxy {
            public:
                ElementProxy(SharedString* p, std::size_t i) : parent(p), index(i) {}
                ElementProxy(const ElementProxy&) = default;
                ElementProxy(ElementProxy&&) = default;

                ElementProxy& operator=(CharType value) {
                    parent->make_mutable();
                    parent->data_ptr[index] = value;
                    return *this;
                }

                operator CharType() const {
                    return parent->data_ptr[index];
                }

            private:
                SharedString* parent;
                std::size_t index;

        };


        SharedString() : SharedString(32) {}


        explicit SharedString(std::size_t size) {
            this->data_struct = DataStruct::create(size)->add_reference();
            this->count = 0;
            this->data_ptr = data_struct->data;
        }


        SharedString(const std::initializer_list<CharType>& init_list)
         : SharedString(std::begin(init_list), std::end(init_list), true) {}


        template<typename InputIt, typename std::iterator_traits<InputIt>::difference_type* = nullptr>
        SharedString(const InputIt& first, const InputIt& last, bool=false) {
            std::size_t size = std::distance(first, last);
            this->data_struct = DataStruct::create(size+1)->add_reference();
            this->count = size;
            this->data_ptr = data_struct->data;
            std::size_t i = 0;
            auto it = first;
            for (i=0;i<size;i++,it=std::next(it))
                this->data_ptr[i] = *it;
            this->data_ptr[i++] = 0;
        }


        SharedString(CharType* ptr, std::size_t count = npos) {
            if (count == npos) count = strlen(ptr);
            this->data_struct = DataStruct::create(count+1)->add_reference();
            this->count = count;
            this->data_ptr = data_struct->data;
            std::size_t i = 0;
            for (i=0;i<count;i++)
                this->data_ptr[i] = ptr[i];
            this->data_ptr[i++] = 0;
        }


        SharedString(const CharType* ptr, std::size_t count = npos) {
            if (count == npos) count = strlen(ptr);
            this->data_struct = DataStruct::create(ptr, count)->add_reference();
            this->count = count;
            this->data_ptr = data_struct->data;
        }


        template<typename T, typename std::enable_if<is_like_string<T, CharType>::value, void>::type* = nullptr>
        SharedString(const T& other) : SharedString(other.data(), other.size()) {}


        SharedString(CharType* begin, CharType* end)
         : SharedString(begin, end-begin) {}


        SharedString(const CharType* begin, const CharType* end) {
            this->data_struct = DataStruct::create(begin, end)->add_reference();
            this->count = end - begin;
            this->data_ptr = this->data_struct->data;
        }


        SharedString(DataStruct* data_struct, CharType* data_ptr, std::size_t count) {
            this->data_struct = data_struct->add_reference();
            this->count = count;
            this->data_ptr = data_ptr;
        }

        SharedString(DataStruct* data_struct, CharType* data_ptr, CharType* data_end)
         : SharedString(data_struct, data_ptr, data_end - data_ptr) {}


        SharedString(const SharedString& other) {
            this->data_struct = other.data_struct->add_reference();
            this->count = other.count;
            this->data_ptr = other.data_ptr;
        }


        SharedString(SharedString&& other) {
            this->data_struct= other.data_struct;
            this->count = other.count;
            this->data_ptr = other.data_ptr;
            other.data_struct = NULL;
            other.count = 0;
            other.data_ptr = NULL;
        }


        ~SharedString() {
            if (data_struct) DataStruct::remove_reference(data_struct);
        }


        bool is_mutable() const {
            return data_struct->references_count == 1 && data_struct->is_mutable();
        }


        void make_mutable() {
            if (is_mutable()) return;
            detach(count);
        }


        bool is_reserved(std::size_t expected) const {
            return is_mutable() && (data_ptr + expected <= data_struct->data + data_struct->count);
        }


        void reserve(std::size_t allocate_count=0) {
            if (is_reserved(allocate_count)) return;
            detach(allocate_count);
        }


        void detach(std::size_t allocate_count=0) {
            DataStruct* new_data_struct = DataStruct::create(std::max<std::size_t>(count+1, allocate_count));
            for (std::size_t i=0;i<count;i++)
                new_data_struct->data[i] = data_ptr[i];
            new_data_struct->data[count] = 0;
            data_ptr = new_data_struct->data;
            DataStruct::remove_reference(data_struct);
            data_struct = new_data_struct->add_reference();
        }


        void resize(std::size_t new_size, CharType filler = 0) {
            reserve(new_size);
            for (std::size_t i=count;i<new_size;i++)
                data_ptr[i] = filler;
            count = new_size;
        }


        std::size_t capacity() const {
            return is_mutable() ? data_struct->count - (data_ptr - data_struct->data) : count;
        }


        void append(const CharType* str, std::size_t add_count=npos) {
            if (add_count == npos) add_count = strlen(str);
            std::size_t new_count = count + add_count;
            if (!is_reserved(new_count))
                reserve(new_count * 2);
            for (std::size_t i=count,j=0;j<add_count;i++,j++)
                data_ptr[i] = str[j];
            count = new_count;
        }


        template<class T>
        typename std::enable_if<is_like_string<T>::value, void>::type
        append(const T& str) {
            append(str.data(), str.size());
        }


        void push_back(const CharType chr) {
            std::size_t new_count = count + 1;
            if (!is_reserved(new_count))
                reserve(new_count * 2);
            data_ptr[count] = chr;
            count = new_count;
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


        const CharType* begin() const {
            return data_ptr;
        }


        const CharType* end() const {
            return data_ptr+count;
        }


        CharType* begin() {
            make_mutable();
            return data_ptr;
        }


        CharType* end() {
            make_mutable();
            return data_ptr+count;
        }


        SharedString substr(std::size_t position, std::size_t char_count=npos) const {
            position = std::min(position, count);
            char_count = std::min(char_count, count - position);
            return SharedString(data_struct, data_ptr + position, char_count);
        }


        CharType at(std::size_t position) const {
            return data_struct->data[position];
        }


        const CharType& operator[](std::size_t position) const {
            return data_ptr[position];
        }


        ElementProxy operator[](std::size_t position) {
            return ElementProxy(this, position);
        }


        CharType* data() const {
            return data_ptr;
        }


        const CharType* c_str() {
            CharType* zero_ptr = data_ptr + count;
            if (data_struct->data + data_struct->count <= zero_ptr || *zero_ptr) {
                push_back((CharType)0);
                count--;
            }
            return data();
        }


        std::size_t references_count() const {
            return data_struct->references_count;
        }


        bool is_moved() const {
            return data_struct == NULL;
        }


        int compare(std::size_t pos, std::size_t length, const CharType* data) const {
            std::size_t loop_end = std::min(count, length);
            for (std::size_t i=pos;i<loop_end;i++) {
                int diff = (int)data_ptr[i] - (int)data[i];
                if (diff) return (diff > 0) ? 1 : -1;
            }
            return count == length ? 0 : count < length ? 1 : -1;
        }


        int compare(const CharType* data, std::size_t length=npos) const {
            if (length == npos) length = strlen(data);
            return compare(0, length, data);
        }


        template<class T>
        typename std::enable_if<is_like_string<T, CharType>::value, int>::type compare(const T& other) const {
            return compare(0, other.size(), other.data());
        }


        template<class T>
        static typename std::enable_if<is_like_string<T, CharType>::value && !std::is_same<T, SharedStringData<CharType>>::value, SharedString>::type
        smart_concat(const SharedStringData<CharType>& a, const T& b) {
            std::size_t a_size = a.size();
            std::size_t b_size = b.size();
            std::size_t count = a_size + b_size;
            if (a.is_reserved(count)) {
                CharType *a_data = a.data();
                CharType *b_data = b.data();
                for (std::size_t i=0,j=a_size;i<b_size;i++,j++)
                    a_data[j] = b_data[i];
                return SharedString(a.data_struct, a_data, count);
            } else
                return dummy_concat(a, b);
        }


        template<class T, class U>
        static typename std::enable_if<is_like_string<T, CharType>::value && is_like_string<U, CharType>::value, SharedString>::type
        dummy_concat(const T& a, const U& b) {
            std::size_t a_size = a.size();
            std::size_t b_size = b.size();
            std::size_t count = a_size + b_size;
            CharType *a_data = a.data();
            CharType *b_data = b.data();
            SharedString result(count);
            result.count = count;
            std::size_t i = 0;
            for (std::size_t j=0;j<a_size;j++)
                result.data_ptr[i++] = a_data[j];
            for (std::size_t j=0;j<b_size;j++)
                result.data_ptr[i++] = b_data[j];
            return result;
        }


        SharedString concat(const SharedStringData<CharType>& a, const SharedStringData<CharType>& b) {
            return smart_concat(a, b);
        } 


        template<class T>
        static typename std::enable_if<is_like_string<T, CharType>::value && !std::is_same<T, SharedStringData<CharType>>::value, SharedString>::type
        concat(const SharedStringData<CharType>& a, const T& b) {
            return smart_concat(a, b);
        } 

        
        template<class T>
        static typename std::enable_if<is_like_string<T, CharType>::value && !std::is_same<T, SharedStringData<CharType>>::value, SharedString>::type
        concat(const T& a, const SharedStringData<CharType>& b) {
            return dummy_concat(a, b);
        } 


        void clear() {
            count = 0;
        }


        bool is_placed_in(std::size_t position, const CharType* needle, std::size_t length=npos) const {
            if (length == npos) length = strlen(needle);
            if (position + length > count) return 0;
            CharType* data_pos = data_ptr + position;
            for (std::size_t i=0;i<length;i++)
                if (data_pos[i] != needle[i]) return false;
            return true;
        }


        template<class T>
        typename std::enable_if<is_like_string<T, CharType>::value, bool>::type is_placed_in(std::size_t position, const T& needle) const {
            return is_placed_in(position, needle.data(), needle.size());
        }


        std::size_t find(const CharType* needle, std::size_t start_position=0, std::size_t length=npos) const {
            if (length == npos) length = strlen(needle);
            if (length > count) return npos;
            std::size_t end_position = count - length + 1;
            for (std::size_t i=start_position;i<end_position;i++)
                if (is_placed_in(i, needle, length)) return i;
            return npos;
        }


        template<class T>
        typename std::enable_if<is_like_string<T, CharType>::value, std::size_t>::type find(const T& needle, std::size_t start_position=0) const {
            return find(needle.data(), start_position, needle.size());
        }


        std::size_t rfind(const CharType* needle, std::size_t start_position=npos, std::size_t length=npos) const {        
            if (length == npos) length = strlen(needle);
            start_position = (start_position == npos || start_position > count - length) ? count - length : start_position;
            if (length == 0) return start_position;
            if (length > count) return npos;
            if (!count) return 0;
            for (std::size_t i=start_position+1;i-->0;)
                if (is_placed_in(i, needle, length)) return i;    
            return npos;
        }


        template<class T>
        typename std::enable_if<is_like_string<T, CharType>::value, std::size_t>::type rfind(const T& needle, std::size_t start_position=0) const {
            return rfind(needle.data(), start_position, needle.size());
        }


        bool contains(const CharType* needle, std::size_t length=npos) const {
            return find(needle, 0, length) != npos;
        }


        template<class T>
        typename std::enable_if<is_like_string<T, CharType>::value, bool>::type contains(const T& needle) const {
            return contains(needle.data(), needle.size());
        }


        bool starts_with(const CharType* needle, std::size_t length=npos) const {
            return is_placed_in(0, needle, length);
        }


        template<class T>
        typename std::enable_if<is_like_string<T, CharType>::value, bool>::type starts_with(const T& needle) const {
            return starts_with(needle.data(), needle.size());
        }


        bool ends_with(const CharType* needle, std::size_t length=npos) const {
            return is_placed_in(count - length, needle, length);
        }


        template<class T>
        typename std::enable_if<is_like_string<T, CharType>::value, bool>::type ends_with(const T& needle) const {
            return ends_with(needle.data(), needle.size());
        }


        std::list<SharedString<CharType>> split(const CharType* separator, std::size_t separator_length=npos, std::size_t limit=npos) const {
            std::list<SharedString<CharType>> result;
            if (separator_length == npos) separator_length = strlen(separator);
            if (separator_length) {
                std::size_t result_count = 0;
                std::size_t pos = 0;
                while (pos <= count) {
                    std::size_t next_pos = (result_count++ >= limit) ? count : find(separator, pos, separator_length);
                    if (next_pos > count) next_pos = count;
                    result.emplace_back(data_struct, data_ptr + pos, next_pos - pos);
                    pos = next_pos + separator_length;
                }
            }
            return result;
        }


        template<typename T, typename std::enable_if<is_like_string<T, CharType>::value, void>::value* = nullptr>
        std::list<SharedString<CharType>> split(const T& separator, std::size_t limit=npos) const {
            return split(separator.data(), separator.size(), limit);
        }


        SharedString& operator=(const SharedString& other) {
            if (&other != this) {
                if (data_struct) DataStruct::remove_reference(data_struct);
                data_ptr = other.data_ptr;
                count = other.count;
                data_struct = other.data_struct->add_reference();
            }
            return *this;
        }
                
        
        SharedString& operator=(SharedString&& other) {
            if (&other != this) {
                if (data_struct) DataStruct::remove_reference(data_struct);
                data_ptr = other.data_ptr;
                count = other.count;
                data_struct = other.data_struct;
                other->data_struct = NULL;
            }
            return *this;
        }


        template<class T, typename std::enable_if<is_like_string<T>::value, void>::type* = nullptr>
        auto operator <=> (const T& t) const {
            return this->compare(t.data(), t.size()) <=> 0;
        }


        auto operator <=> (const CharType* u) const {
            return this->compare(u) <=> 0;
        }

        
        template<class T, typename std::enable_if<is_like_string<T>::value, void>::type* = nullptr>
        bool operator == (const T& t) const {
            return this->compare(t.data(), t.size()) == 0;
        }


        bool operator == (const CharType* u) const {
            return this->compare(u) == 0;
        }


        template<class T, typename std::enable_if<is_like_string<T>::value, void>::type* = nullptr>
        SharedString operator+(const T& x) const {
            SharedString result(*this);
            result.append(x.data(), x.size());
            return result;
        }

                
        SharedString operator+(const CharType* x) const {
            SharedString result(*this);
            result.append(x);
            return result;
        }
                

        SharedString operator+(const CharType x) const {
            SharedString result(*this);
            result.push_back(x);
            return result;
        }


    private:

        CharType* data_ptr;
        std::size_t count;
        DataStruct* data_struct;

};

