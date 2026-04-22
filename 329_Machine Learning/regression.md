### 1. Classification Loss Functions
These are used when you want the model to choose between discrete categories.

#### Binary Cross-Entropy (Log Loss)
* When to use: Binary classification (2 classes).
* Activation: Pair it with a Sigmoid activation in the final layer.
* Why: It penalizes confident but wrong predictions exponentially. It is much more effective than Mean Squared Error for classification because it keeps the gradient strong even when the sigmoid function starts to flatten out.
* Example: Is this deepfake video "Real" or "Fake"?

#### Categorical Cross-Entropy
* When to use: Multi-class classification (3+ classes), where each sample belongs to exactly one class.
* Activation: Pair it with a Softmax activation.
* Why: It treats the output as a probability distribution. It measures the "distance" between the predicted probability and the ground truth (which is usually a one-hot vector).
* Example: Identifying which Bangla character is written in an image.

#### Sparse Categorical Cross-Entropy
* When to use: Same as Categorical Cross-Entropy, but when your labels are integers (0, 1, 2) instead of one-hot vectors.
* Why: It is memory-efficient. If you have 10,000 classes, you don't want to store a 10,000-length vector for every single label.
* Example: Predicting the next word in a large-vocabulary language model.

---

### 2. Regression Loss Functions
These are used when your output is a continuous numerical value.

#### Mean Squared Error (MSE / L2 Loss)
* When to use: Predicting precise numerical values.
* Why: It squares the error ($(y - \hat{y})^2$). This makes it very sensitive to outliers. If the model is off by a little, the penalty is small; if it is off by a lot, the penalty is massive.
* Example: Predicting the exact frequency or amplitude in a Text-to-Speech (TTS) model.

#### Mean Absolute Error (MAE / L1 Loss)
* When to use: When your dataset contains many outliers or noisy data.
* Why: It takes the absolute difference ($|y - \hat{y}|$). Unlike MSE, the penalty grows linearly, not quadratically. This makes the model more robust and less likely to be "pulled" by a few bad data points.
* Example: Predicting the duration of a phoneme in speech synthesis where some samples might be abnormally long.

---

### Summary Selection Guide

| Task Type | Output Activation | Recommended Loss |
| :--- | :--- | :--- |
| Binary Classification | Sigmoid | Binary Cross-Entropy |
| Multi-class Classification | Softmax | Categorical Cross-Entropy |
| Regression (General) | Linear / None | Mean Squared Error (MSE) |
| Regression (Outliers) | Linear / None | MAE |

Given your work with NLP and Deepfake detection, you'll likely find yourself alternating between Binary Cross-Entropy (for the final fake/real decision) and Triplet Loss (if you're comparing facial features across frames). Does one of these feel more applicable to the specific deepfake architecture you are currently researching?
