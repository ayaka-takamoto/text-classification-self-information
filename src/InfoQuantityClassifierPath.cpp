#include "InfoQuantityClassifierPath.h"
#include "SuffixArrayMatcher.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <limits>
#include <cmath>
#include <algorithm>
#include <filesystem> 

const std::string dataDirName("../data/"); // 元コードの定数（必要なら使用）

// ===================== I/O =====================


std::vector<TestData> readTestData(const std::string &filename) {
    std::ifstream ifs(filename);
    if (!ifs) {
        throw std::runtime_error("Failed to open test file: " + filename);
    }
    std::vector<TestData> res;
    std::string line;
    while (std::getline(ifs, line)) {
        if (line.empty()) continue;
        auto pos = line.find(',');
        if (pos == std::string::npos) continue;
        int cid = std::stoi(line.substr(0, pos));
        std::string text = line.substr(pos + 1);
        res.push_back({cid, text});
    }
    return res;
}

std::map<int, std::string> readTrainData(const std::string &filename, const std::string& separator) {
    std::ifstream ifs(filename);
    if (!ifs) {
        throw std::runtime_error("Failed to open train file: " + filename);
    }
    std::map<int, std::string> res; // classID -> concatenated text
    std::string line;
    while (std::getline(ifs, line)) {
        if (line.empty()) continue;
        auto pos = line.find(',');
        if (pos == std::string::npos) continue;
        int cid = std::stoi(line.substr(0, pos));
        std::string text = line.substr(pos + 1);
        if (res[cid].empty()) {
            res[cid] = text;
        } else {
            if (!separator.empty()) res[cid] += separator;
            res[cid] += text;
        }
    }
    return res;
}

// ===================== 情報量 =====================

double maxInfo(const int dataLen, const int strLen) {
    if (dataLen <= 0 || strLen <= 0) return 0.0;
    return strLen * std::log2(static_cast<double>(dataLen) * 8.0);
}

double calcInfo(const int dataLen, const int freq, const int strLen) {
    if (strLen <= 0) return 0.0;
    if (freq <= 0) return maxInfo(dataLen, strLen);
    // 元コードの既定式（候補差し替えがあればここを変更）
    return -std::log2((double)freq / dataLen);
    // 代替候補：
    // return -std::log2(static_cast<double>(freq) / static_cast<double>(std::max(1, dataLen)));
}

// ===================== 推定（最適分割 DP） =====================

double estimation(const std::string &data,
                  const std::vector<int> &sa,
                  const std::string &sample,
                  std::vector<std::string> &usedSubstrings) {
    const int n = static_cast<int>(sample.size());
    if (n == 0) return 0.0;

    std::vector<double> dp(n + 1, 0.0); // dp[i] = sample[i:] の最小コスト
    const SuffixArrayMatcher matcher(data, sa);

    for (int i = n - 1; i >= 0; --i) {
        // まず 1 文字を素直に送るコストで初期化
        dp[i] = std::log2(static_cast<double>(data.size()) * 8.0) + dp[i + 1];

        size_t sp = 0, ep = sa.size();
        size_t offset = 0;

        // sample[i..j] を 1 文字ずつ伸ばして頻度を調べる
        for (int j = i; j < n && sp < ep; ++j, ++offset) {
            const char c = sample[j];
            sp = matcher.patternSP(c, sp, ep, offset);
            ep = matcher.patternEP(c, sp, ep, offset);
            const int freq = static_cast<int>(ep - sp);
            if (freq <= 0) break;

            const int seg_len = j - i + 1;
            const double cost = calcInfo(static_cast<int>(data.size()), freq, seg_len) + dp[j + 1];
            if (cost < dp[i]) {
                dp[i] = cost;
                // 参考メモ（最終経路とは一致しない可能性あり）
                usedSubstrings.push_back(sample.substr(i, seg_len));
            }
        }
    }
    return dp[0];
}

 
// 経路（nextPos/segLen）を返す版：可視化で使用
double estimationWithPath(const std::string& data,
                          const std::vector<int>& sa,
                          const std::string& sample,
                          std::vector<int>& nextPos,
                          std::vector<int>& segLen) {
    const int n = static_cast<int>(sample.size());
    if (n == 0) { nextPos.clear(); segLen.clear(); return 0.0; }

    std::vector<double> dp(n + 1, 0.0);
    nextPos.assign(n + 1, -1);
    segLen.assign(n + 1, 0);

    const SuffixArrayMatcher matcher(data, sa);

    for (int i = n - 1; i >= 0; --i) {
        dp[i]      = std::log2(static_cast<double>(data.size()) * 8.0) + dp[i + 1];
        nextPos[i] = i + 1;
        segLen[i]  = 1;

        size_t sp = 0, ep = sa.size();
        size_t offset = 0;
        for (int j = i; j < n && sp < ep; ++j, ++offset) {
            const char c = sample[j];
            sp = matcher.patternSP(c, sp, ep, offset);
            ep = matcher.patternEP(c, sp, ep, offset);
            const int freq = static_cast<int>(ep - sp);
            if (freq <= 0) break;
            const int len  = j - i + 1;
            const double cost = calcInfo(static_cast<int>(data.size()), freq, len) + dp[j + 1];
            if (cost < dp[i]) {
                dp[i]      = cost;
                nextPos[i] = j + 1;
                segLen[i]  = len;
            }
        }
    }
    return dp[0];
}

std::string renderWithSeparator(const std::string& text,
                                const std::vector<int>& nextPos,
                                const std::vector<int>& segLen,
                                const std::string& vizSep) {
    std::string out;
    out.reserve(text.size() + text.size() / 2);
    for (int i = 0; i < static_cast<int>(text.size()); ) {
        int len = (i < static_cast<int>(segLen.size())) ? segLen[i] : 1;
        if (len <= 0) len = 1;
        out.append(text, i, len);
        int nxt = (i < static_cast<int>(nextPos.size()) && nextPos[i] > i && nextPos[i] <= static_cast<int>(text.size()))
                  ? nextPos[i]
                  : i + len;
        i = nxt;
        if (i < static_cast<int>(text.size()))
            out += vizSep;
    }
    return out;
}

// ===================== デバッグ補助 =====================

void printVectorToFile(const std::vector<int> &vec, const std::string &filename) {
    std::ofstream ofs(filename);
    for (size_t i = 0; i < vec.size(); ++i) {
        ofs << vec[i] << (i + 1 == vec.size() ? '\n' : ' ');
    }
}

// ===================== 実験ドライバ =====================

void runSingleDataset(const std::string &datasetName,
                      const std::string &dataDir,
                      const std::string &resultDir,
                      const std::string &separator,
                      const std::string &vizSep,
                      bool dumpAllClassSegments,
                      OutputMainDetail mainDetail) {
    const std::string train_path  = dataDir + datasetName + "_TRAIN.txt";
    const std::string test_path   = dataDir + datasetName + "_TEST.txt";
    const std::string result_path = resultDir + datasetName + "_path.txt";

    // 学習読み込み
    const auto train_map = readTrainData(train_path, separator);

    // クラスごとに SA 構築
    std::unordered_map<int, std::string>     class_text;
    std::unordered_map<int, std::vector<int>> class_sa;
    class_text.reserve(train_map.size());
    class_sa.reserve(train_map.size());
    for (const auto &kv : train_map) {
        class_text[kv.first] = kv.second;
        class_sa[kv.first]   = createSA(kv.second);
    }

    // （オプショナル）クラス別の分割出力ファイルを開く
    std::unordered_map<int, std::ofstream> seg_out; // cid → stream
    if (dumpAllClassSegments) {
        for (const auto& kv : class_text) {
            const int cid = kv.first;
            const std::string seg_path = resultDir + datasetName + "_seg_c" + std::to_string(cid) + ".txt";
            std::ofstream ofs_seg(seg_path);
            if (!ofs_seg) {
                throw std::runtime_error("Failed to open segment file: " + seg_path);
            }
            seg_out.emplace(cid, std::move(ofs_seg));
        }
    }

    // テスト読み込み
    const auto tests = readTestData(test_path);

    // 推定＆ログ出力
    std::ofstream ofs(result_path);
    if (!ofs) throw std::runtime_error("Failed to open result file: " + result_path);

    int correct = 0;
    for (const auto &s : tests) {
        double best_score = std::numeric_limits<double>::infinity();
        int best_cid = -1;

        // （必要なら）全クラス分を事前に計算して、同時にクラス別ファイルに出力
        if (dumpAllClassSegments) {
            for (const auto& kv : class_text) {
                const int cid = kv.first;
                const std::string& data = kv.second;
                const std::vector<int>& sa = class_sa.at(cid);

                std::vector<int> nextPos, segLen;
                const double score = estimationWithPath(data, sa, s.text, nextPos, segLen);
                const std::string segged = renderWithSeparator(s.text, nextPos, segLen, vizSep);

                auto it = seg_out.find(cid);
                if (it != seg_out.end()) {
                    it->second << "orig: " << s.text
                               << "\tseg: "  << segged
                               << "\tscore: " << score << "\n";
                }

                if (score < best_score) { best_score = score; best_cid = cid; }
            }
        } else {
            // 従来どおり：まず最良クラスだけ決める
            for (const auto& kv : class_text) {
                const int cid = kv.first;
                const std::string& data = kv.second;
                const std::vector<int>& sa = class_sa.at(cid);
                std::vector<std::string> used;
                const double score = estimation(data, sa, s.text, used);
                if (score < best_score) { best_score = score; best_cid = cid; }
            }
        }

        const int is_ok = (best_cid == s.classID) ? 1 : 0;
        correct += is_ok;

        std::string segmented;
        {
            const std::string& data = class_text.at(best_cid);
            const std::vector<int>& sa = class_sa.at(best_cid);
            std::vector<int> nextPos, segLen;
            (void) estimationWithPath(data, sa, s.text, nextPos, segLen);
            segmented = renderWithSeparator(s.text, nextPos, segLen, vizSep);
        }

        ofs << "correct_class: " << s.classID
            << "\tresult_class: " << best_cid
            << "\t" << is_ok;
        
        if (mainDetail == OutputMainDetail::Seg) {
            // 分割可視化：必要なときだけ経路を計算
            const std::string& data = class_text.at(best_cid);
            const std::vector<int>& sa = class_sa.at(best_cid);
            std::vector<int> nextPos, segLen;
            (void) estimationWithPath(data, sa, s.text, nextPos, segLen);
            const std::string segmented = renderWithSeparator(s.text, nextPos, segLen, vizSep);
            ofs << "\tseg: " << segmented;
        } else if (mainDetail == OutputMainDetail::Score) {
            // 既に持っている best_score を出すだけ（経路計算は不要）
            ofs << "\tscore: " << best_score;
        } else {
            // None：追加情報なし
        }
        ofs << "\n";

    }

    ofs << correct << "/" << tests.size() << "\n";
    ofs.setf(std::ios::fixed); ofs.precision(6);
    ofs << (tests.empty() ? 0.0 : static_cast<double>(correct) / static_cast<double>(tests.size())) << "\n";
    for (auto& kv : seg_out) kv.second.close();
}

void runExperiment(const std::string &fileListPath,
                   const std::string &dataDir,
                   const std::string &resultDir,
                   const std::string& separator,
                   const std::string& vizSep,
                   bool dumpAllClassSegments,
                   OutputMainDetail mainDetail) {
    std::ifstream ifs(fileListPath);
    if (!ifs) throw std::runtime_error("Failed to open file list: " + fileListPath);

    std::string name;
    while (std::getline(ifs, name)) {
        if (name.empty()) continue;
        std::cerr << "[RUN] " << name << std::endl;
        runSingleDataset(name, dataDir, resultDir, separator, vizSep, dumpAllClassSegments, mainDetail);
    }
}

// ===================== main =====================

int main() {
    try {
        const std::string list     = "file_name_list.txt";
        const std::string dataDir  = "../data/";
        const std::string resultDir= "../result/";
        
        runExperiment(list, dataDir, resultDir,
                      /*separator*/"",
                      /*vizSep*/"|",
                      /*dumpAllClassSegments*/true,
                      /*mainDetail*/OutputMainDetail::Score);
    } catch (const std::exception &e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}