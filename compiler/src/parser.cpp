/* Author: Vrushang Anand */
/* Last Date Modified: Jun 23, 2026 */
/* Source file for the Parser for a custom compiler for the ML Inference
 * Ecosystem */

#include "parser.hpp"

void Layer::initWeights(long rows, long cols)
{
    this->weights.rows = rows;
    this->weights.cols = cols;
}

void Layer::initBiases(long rows, long cols)
{
    this->biases.rows = rows;
    this->biases.cols = cols;
}

void Layer::setActFunc(ActFunc func)
{
    this->actfunc = func;
}
