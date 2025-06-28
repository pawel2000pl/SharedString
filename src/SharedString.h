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
struct SharedStringData {

    SharedStringData() = default;
    SharedStringData(const char* adata, const std::size_t acount) : data{(char*)adata}, count{acount} {}
    SharedStringData(const SharedStringData&) = delete;
    SharedStringData(SharedStringData&&) = delete;

    virtual ~SharedStringData() {}

    SharedStringData* add_reference() {
        references_count++;
        return this;
    }

    std::size_t remove_reference(void* object) {
        if (owner == object) owner = NULL;
        return --references_count;
    }

    bool set_owner(void* new_owner) {
        if (owner) return owner == new_owner;
        if (!new_owner) return false;
        owner = new_owner;
        return true;
    }

    char* const data = NULL;
    const std::size_t count = 0;
    std::size_t references_count = 0;
    void* owner = 0;

};


template<typename CharType, std::size_t allocated_count>
struct SharedStringDataConstBuf : public SharedStringData<CharType> {

    SharedStringDataConstBuf() : SharedStringData<CharType>(data_buf, allocated_count) {}

    char data_buf[allocated_count];

};


template<typename CharType>
struct SharedStringDataConstPtr : public SharedStringData<CharType> {

    SharedStringDataConstPtr(const char* ptr) : SharedStringData<CharType>(ptr, strlen(ptr)) {
        this->owner = this; // modifications not allowed
    }

    SharedStringDataConstPtr(const char* ptr, std::size_t count) : SharedStringData<CharType>(ptr, count) {
        this->owner = this; // modifications not allowed
    }

    SharedStringDataConstPtr(const char* begin, char* end) : SharedStringData<CharType>(begin, end - begin) {
        this->owner = this; // modifications not allowed
    }

};


template<typename CharType>
struct SharedStringDataDynamicPtr : public SharedStringData<CharType> {

    SharedStringDataDynamicPtr(std::size_t count) : SharedStringData<CharType>(new CharType[count], count) {}

    ~SharedStringDataDynamicPtr() override {
        delete [] this->data;
    }

};



template<typename CharType = char>
class SharedString {

    public:

        using DataStruct = SharedStringData<CharType>;
        constexpr static const std::size_t npos = std::size_t(-1);


        template<std::size_t allocate=32>
        SharedString(std::integral_constant<std::size_t, allocate>) {
            data_struct = (new SharedStringDataConstBuf<CharType, allocate>())->add_reference();
            count = 0;
            data_ptr = data_struct->data;
        }


        SharedString(const std::initializer_list<CharType>& init_list) {
            std::size_t size = init_list.size();
            data_struct = (new SharedStringDataDynamicPtr<CharType>(size))->add_reference();
            count = size;
            data_ptr = data_struct->data;
            std::size_t i = 0;
            for (const auto chr : init_list)
                data_ptr[i++] = chr;
        }


        template<typename InputIt, typename std::iterator_traits<InputIt>::difference_type* = nullptr>
        SharedString(InputIt& first, InputIt& last) {
            std::size_t size = std::distance(first, last);
            data_struct = (new SharedStringDataConstPtr<CharType>(size))->add_reference();
            count = size;
            data_ptr = data_struct->data;
            auto it = first;
            std::size_t i = 0;
            for (std::size_t i=0;i<size;i++,it=std::next(it)) {
                data_ptr[i] = *it;
            }
        }


        SharedString(const char* ptr) {
            data_struct = (new SharedStringDataConstPtr<CharType>(ptr))->add_reference();
            count = data_struct->count;
            data_ptr = data_struct->data;
        }


        SharedString(const char* ptr, std::size_t count) {
            data_struct = (new SharedStringDataConstPtr<CharType>(ptr, count))->add_reference();
            this->count = count;
            data_ptr = data_struct->data;
        }


        template<typename T, typename std::enable_if<is_like_string<T>::value, void>::type* = nullptr>
        SharedString(const T& other) : SharedString(other.data(), other.size()) {}


        SharedString(const char* begin, const char* end) {
            data_struct = (new SharedStringDataConstPtr<CharType>(begin, end))->add_reference();
            count = end - begin;
            data_ptr = data_struct->data;
        }


        SharedString(const std::size_t allocate) {
            data_struct = (new SharedStringDataDynamicPtr<CharType>(allocate))->add_reference();
            count = 0;
            data_ptr = data_struct->data;
        }


        SharedString(DataStruct* data_struct, std::size_t offset, std::size_t count) {
            this->data_struct = data_struct->add_reference();
            this->count = count;
            this->data_ptr = data_struct->data + offset;
        }


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
            if (data_struct && !data_struct->remove_reference(this)) delete data_struct;
        }


        bool is_mutable() const {
            return data_struct->owner == this;
        }


        void make_mutable() {
            if (is_mutable()) return;
            if (data_struct->set_owner(this)) return;
            reserve(count, true);
        }


        void reserve(std::size_t allocate_count=0, bool make_me_owner=false) {
            if (allocate_count <= (data_struct->set_owner(this) ? data_struct->count : count)) return;
            DataStruct* new_data_struct = new SharedStringDataDynamicPtr<CharType>(std::max<std::size_t>(count, allocate_count));
            if (make_me_owner) new_data_struct->owner = this;
            for (std::size_t i=0;i<count;i++)
                new_data_struct->data[i] = data_ptr[i];
            data_ptr = new_data_struct->data;
            if (!data_struct->remove_reference(this)) delete data_struct;
            data_struct = new_data_struct->add_reference();
        }


        SharedString* detach() {
            reserve();
            return this;
        }


        void push_back(const CharType* str, std::size_t add_count=1) {
            std::size_t new_count = count + add_count;
            if (!data_struct->set_owner(this) || new_count > data_struct->count)
                reserve(new_count * 2, true);
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
