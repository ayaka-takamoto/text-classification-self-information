#ifndef SUFFIX_ARRAY_MATCHER_H
#define SUFFIX_ARRAY_MATCHER_H

#include <vector>
#include <string>

std::vector<int> createSA(const std::string &text);

struct SuffixArrayMatcher {
    const char *text;
    const std::vector<int> &sa;

    SuffixArrayMatcher(const std::string &textRef, const std::vector<int> &saRef);

    void print() const;

    // 1文字を追加して一致区間 [start, end) の下限/上限を更新
    int patternSP(char pattern, size_t start, size_t end, size_t offset) const;
    int patternEP(char pattern, size_t start, size_t end, size_t offset) const;

    // 文字列パターン版（必要に応じて使用）
    int patternSP(const char *pattern, size_t start, size_t end, size_t offset) const;
    int patternEP(const char *pattern, size_t start, size_t end, size_t offset) const;

private:
    // pattern と suffix の辞書順比較（pattern が尽きるまで比較する）
    int comparePatternWithSuffix(const char *pattern, const char *suffix) const;
};

#endif // SUFFIX_ARRAY_MATCHER_H