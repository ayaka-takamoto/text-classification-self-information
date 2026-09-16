#ifndef INFO_QUANTITY_CLASSIFIER_H
#define INFO_QUANTITY_CLASSIFIER_H

#include <string>
#include <vector>
#include <map>

struct TestData {
    int classID;
    std::string text;
};

// ★ 追加：メイン出力の詳細レベル
enum class OutputMainDetail {
    None,   // 追加情報なし（seg も score も出さない）
    Seg,    // seg: <分割> を出す（デフォルト）
    Score   // score: <数値> を出す（seg の代わり）
};

// 入出力
std::vector<TestData> readTestData(const std::string &filename);
std::map<int, std::string> readTrainData(const std::string &filename,
                                         const std::string &separator = "");

// 情報量
double maxInfo(const int dataLen, const int strLen);
double calcInfo(const int dataLen, const int freq, const int strLen);

// 推定（最適分割 DP）
double estimation(const std::string &data,
                  const std::vector<int> &sa,
                  const std::string &sample,
                  std::vector<std::string> &usedSubstrings);

// 経路（可視化用）
double estimationWithPath(const std::string& data,
                          const std::vector<int>& sa,
                          const std::string& sample,
                          std::vector<int>& nextPos,
                          std::vector<int>& segLen);

std::string renderWithSeparator(const std::string& text,
                                const std::vector<int>& nextPos,
                                const std::vector<int>& segLen,
                                const std::string& vizSep = "|");

// 実験ドライバ
void runSingleDataset(const std::string &datasetName,
                      const std::string &dataDir,
                      const std::string &resultDir,
                      const std::string &separator = "",
                      const std::string &vizSep = "|",
                      bool dumpAllClassSegments = false,
                      OutputMainDetail mainDetail = OutputMainDetail::Seg);

void runExperiment(const std::string &fileListPath,
                   const std::string &dataDir,
                   const std::string &resultDir,
                   const std::string &separator = "",
                   const std::string &vizSep = "|",
                   bool dumpAllClassSegments = false,
                   OutputMainDetail mainDetail = OutputMainDetail::Seg);

#endif // INFO_QUANTITY_CLASSIFIER_H