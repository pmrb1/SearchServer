#pragma once

#include <iostream>
#include <vector>

template <typename PageIterator>
class IteratorRange {
public:
    IteratorRange(PageIterator begin, PageIterator end) : first_(begin), last_(end), size_(distance(first_, last_)) {}

    PageIterator begin() const { return first_; }

    PageIterator end() const { return last_; }

    size_t size() const { return size_; }

private:
    PageIterator first_, last_;
    size_t size_;
};

template <typename PageIterator>
std::ostream& operator<<(std::ostream& out, const IteratorRange<PageIterator>& range) {
    for (PageIterator it = range.begin(); it != range.end(); ++it) {
        out << *it;
    }
    return out;
}

template <typename PageIterator>
class Paginator {
public:
    Paginator() = default;

    void Init(PageIterator begin, PageIterator end, size_t page_size) {
        for (size_t left = std::distance(begin, end); left > 0;) {
            const size_t current_page_size = std::min(page_size, left);
            const PageIterator current_page_end = std::next(begin, current_page_size);

            pages_.push_back({begin, current_page_end});

            left -= current_page_size;
            begin = current_page_end;
        }
    }

    bool IsInitialized() { return !pages_.empty(); }

    auto begin() const { return pages_.begin(); }

    auto end() const { return pages_.end(); }

    size_t size() const { return pages_.size(); }

private:
    std::vector<IteratorRange<PageIterator>> pages_;
};

template <typename AllPagesContainer>
auto Paginate(const AllPagesContainer& pages_contatiner, size_t page_size) {
    Paginator<decltype(std::begin(pages_contatiner))> paginator;
    paginator.Init(begin(pages_contatiner), end(pages_contatiner), page_size);

    return paginator.IsInitialized() ? paginator : throw std::invalid_argument("Paginator was not initialzed");
}