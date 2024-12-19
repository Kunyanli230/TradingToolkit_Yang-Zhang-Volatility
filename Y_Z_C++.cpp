
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

// Function to calculate ATR
std::vector<double> calculateATR(const std::vector<double>& high, const std::vector<double>& low, const std::vector<double>& close, int period) {
    std::vector<double> atr(high.size(), 0.0);
    for (size_t i = 1; i < high.size(); ++i) {
        double tr1 = high[i] - low[i];
        double tr2 = std::abs(high[i] - close[i - 1]);
        double tr3 = std::abs(low[i] - close[i - 1]);
        double tr = std::max({tr1, tr2, tr3});
        atr[i] = (i < period) ? tr : (atr[i - 1] * (period - 1) + tr) / period;
    }
    return atr;
}

// Function to calculate AVSO
std::vector<double> calculateAVSO(const std::vector<double>& src, int length, int smoothingLength) {
    std::vector<double> avso(src.size(), 0.0);
    std::vector<double> mean(src.size(), 0.0);
    std::vector<double> stddev(src.size(), 0.0);

    for (size_t i = length; i < src.size(); ++i) {
        double sum = 0.0, sqSum = 0.0;
        for (size_t j = i - length; j < i; ++j) {
            sum += src[j];
            sqSum += src[j] * src[j];
        }
        mean[i] = sum / length;
        stddev[i] = std::sqrt((sqSum / length) - (mean[i] * mean[i]));
        double zScore = (src[i] - mean[i]) / stddev[i];
        avso[i] = (i == length) ? zScore : (avso[i - 1] * (smoothingLength - 1) + zScore) / smoothingLength;
    }
    return avso;
}

// Main UT Bot Strategy Function
void utBotStrategy(
    const std::vector<double>& close,
    const std::vector<double>& high,
    const std::vector<double>& low,
    int atrPeriod, double keyValue, int avsoLength, int avsoSmoothingLength
) {
    auto atr = calculateATR(high, low, close, atrPeriod);
    auto avso = calculateAVSO(close, avsoLength, avsoSmoothingLength);

    std::vector<bool> buy(close.size(), false);
    std::vector<bool> sell(close.size(), false);
    std::vector<double> trailingStop(close.size(), 0.0);

    for (size_t i = 1; i < close.size(); ++i) {
        double prevStop = trailingStop[i - 1];
        double atrValue = atr[i] * keyValue;

        if (close[i] > prevStop) {
            trailingStop[i] = std::max(prevStop, close[i] - atrValue);
        } else {
            trailingStop[i] = std::min(prevStop, close[i] + atrValue);
        }

        buy[i] = (close[i] > trailingStop[i]) && (avso[i] > 0);
        sell[i] = (close[i] < trailingStop[i]) && (avso[i] < 0);
    }

    // Print Results
    for (size_t i = 0; i < close.size(); ++i) {
        std::cout << "Day " << i + 1 << ": Close=" << close[i] 
                  << ", ATR=" << atr[i] << ", AVSO=" << avso[i]
                  << ", Buy=" << buy[i] << ", Sell=" << sell[i] << std::endl;
    }
}

int main() {
    std::vector<double> close = {100, 102, 101, 103, 104, 102, 100, 99, 101};
    std::vector<double> high = {101, 103, 102, 104, 105, 103, 101, 100, 102};
    std::vector<double> low = {99, 101, 100, 102, 103, 101, 99, 98, 100};

    utBotStrategy(close, high, low, 10, 1.0, 20, 5);
    return 0;
}
