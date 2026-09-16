#include "SuffixArrayMatcher.h"
#include <algorithm>
#include <iostream>
#include <cstring>

// 素朴な Suffix Array 構築（辞書順ソート）
std::vector<int> createSA(const std::string &text) {
    const int n = static_cast<int>(text.size());
    std::vector<int> sa(n + 1);
    for (int i = 0; i <= n; ++i) sa[i] = i;

    auto cmp = [&text](int i, int j) {
        const char* a = text.c_str() + i;
        const char* b = text.c_str() + j;
        while (*a && *b) {
            if (*a < *b) return true;
            if (*a > *b) return false;
            ++a; ++b;
        }
        // 短い方が「小さい」
        return *a == '\0' && *b != '\0';
    };
    std::sort(sa.begin(), sa.end(), cmp);
    return sa;
}

SuffixArrayMatcher::SuffixArrayMatcher(const std::string &textRef, const std::vector<int> &saRef)
    : text(textRef.c_str()), sa(saRef) {}

void SuffixArrayMatcher::print() const {
    for (size_t i = 0; i < static_cast<int>(sa.size()); i++) {
        std::cout << text + sa[i] << std::endl;
    }
}

int SuffixArrayMatcher::comparePatternWithSuffix(const char *pattern, const char *suffix) const {
    for (int i = 0; pattern[i] != '\0'; i++) {
        if (pattern[i] < suffix[i]) return -1;
        if (pattern[i] > suffix[i]) return 1;
    }
    return 0; // pattern を使い切るまでは等しい（prefix一致）
}

// [start, end) の中で suffix[offset] >= pattern となる最初の位置
int SuffixArrayMatcher::patternSP(char pattern, size_t start, size_t end, size_t offset) const {
    size_t low = start, high = end;
    while (low < high) {
        const size_t mid = (low + high) >> 1;
        const size_t idx = static_cast<size_t>(sa[mid]) + offset;
        char c = text[idx];
        if (c < pattern) low = mid + 1;
        else high = mid;
    }
    return static_cast<int>(low);
}

// [start, end) の中で suffix[offset] > pattern となる最初の位置
int SuffixArrayMatcher::patternEP(char pattern, size_t start, size_t end, size_t offset) const {
    size_t low = start, high = end;
    while (low < high) {
        const size_t mid = (low + high) >> 1;
        const size_t idx = static_cast<size_t>(sa[mid]) + offset;
        char c = text[idx];
        if (c <= pattern) low = mid + 1;
        else high = mid;
    }
    return static_cast<int>(low);
}

int SuffixArrayMatcher::patternSP(const char *pattern, size_t start, size_t end, size_t offset) const {
    size_t sp = start, ep = end, off = offset;
    for (size_t i = 0; pattern[i] != '\0' && sp < ep; ++i, ++off) {
        sp = patternSP(pattern[i], sp, ep, off);
        ep = patternEP(pattern[i], sp, ep, off);
    }
    return static_cast<int>(sp);
}

int SuffixArrayMatcher::patternEP(const char *pattern, size_t start, size_t end, size_t offset) const {
    size_t sp = start, ep = end, off = offset;
    for (size_t i = 0; pattern[i] != '\0' && sp < ep; ++i, ++off) {
        sp = patternSP(pattern[i], sp, ep, off);
        ep = patternEP(pattern[i], sp, ep, off);
    }
    return static_cast<int>(ep);
}