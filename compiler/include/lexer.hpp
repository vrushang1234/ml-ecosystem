/* Author: Vrushang Anand */
/* Last Date Modified: Jun 23, 2026 */
/* Header file for the Lexer for a custom compiler for the ML Inference
 * Ecosystem */

#include <vector>

enum ActFunc { ReLU, Sigmoid, Softmax, LeakyReLU };

struct Weights {
  std::vector<std::vector<double>> weightsVector;
  long rows, cols;
};

struct Biases {
  std::vector<std::vector<double>> biasesVector;
  long rows, cols;
};

class Layer {
private:
  Weights weights;
  Biases biases;
  ActFunc actfunc;

public:
  Layer();
  void initWeights(long rows, long cols);
  void initBiases(long rows, long cols);
  void setActFunc(ActFunc func);
  ~Layer();
};

class Network {
private:
  std::vector<Layer> layers;
};
