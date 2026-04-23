# Linear Regression


$$
\[
(X^T X)\theta = X^T y
\]
$$

if $\(X^T X\)$ is invertible, we multiply both sides by \((X^T X)^{-1}\) and get:

$$
\[
\theta^* = (X^T X)^{-1} X^T y
\]
$$

This is called the **normal equation**.

### Meaning of the terms
- $\(X\)$: design matrix of size $\(n \times (d+1)\)$
- $\(y\)$: target vector of size $\(n \times 1\)$
- $\(\theta\)$: parameter vector of size $\((d+1) \times 1\)$


## Q1. What if $\(n \gg 1\)$?
**Answer:**  
If the number of samples is very large, computing \(X^T X\) becomes expensive in time and memory. In that case, iterative methods like gradient descent are preferred over the normal equation.

## Q2. What if $\(d \gg 1\)$?
**Answer:**  
If the number of features is very large, then $\(X^T X\)$ is a large $\((d+1)\times(d+1)\)$ matrix. Its inversion becomes computationally costly, so the normal equation becomes inefficient.

## Q3. What if $\(X^T X\)$ is non-invertible?
**Answer:**  
Then the normal equation cannot be applied directly. This usually happens because of linearly dependent or highly correlated features. We can use the pseudoinverse, remove redundant features, or apply regularization.

## Q4. What if the relation between $\(X\)$ and $\(y\)$ is not linear?
**Answer:**  
Then ordinary linear regression will not model the data well. We may use polynomial features, basis expansion, or a nonlinear model instead.



# Gradient Descent

## Q1. How does the choice of learning rate affect training when it is too small, too large, or appropriately chosen?

## Answer

### 1. Learning rate too small
Training becomes very slow because parameter updates are tiny. The model may take many iterations to converge.

### 2. Learning rate too large
Training becomes unstable because updates overshoot the minimum. The loss may oscillate or even diverge.

### 3. Learning rate just right
Training is both stable and reasonably fast. The model moves efficiently toward a minimum and converges well.


# Logistic Regression

<img width="1320" height="569" alt="image" src="https://github.com/user-attachments/assets/7d94e424-7df4-4a39-af1c-63a049c25b10" />

## Q1. Why is sigmoid used in logistic regression instead of tanh or other functions?

Sigmoid is used in logistic regression because the model must output a **probability** for binary classification, and sigmoid maps any real value to the range $\[(0,1)\]$

So it is suitable for representing $\[P(y=1 \mid x)\]$

The sigmoid function is

$$
\[
\sigma(z)=\frac{1}{1+e^{-z}}
\]
$$

It is also used because logistic regression assumes that the **log-odds** are linear in the input:
$\[\log\left(\frac{p}{1-p}\right)=w^T x+b\]$

This makes sigmoid the natural choice.

### Why not tanh?
The tanh function outputs values in $\[(-1,1)\]$

so it does not directly represent probabilities.

### Why not other functions?
- **Linear**: output is unbounded
- **ReLU**: output is not limited to (0,1)
- **Step function**: not differentiable and does not give smooth probability outputs

So sigmoid is preferred because it is smooth, differentiable, and naturally maps outputs to probabilities.

## Q2. Why do we use log-likelihood?

We use log-likelihood because it converts a product of probabilities into a sum: $\[L(\theta)=\prod_{i=1}^{n} p(y_i\mid x_i;\theta)\]$

$\[\log L(\theta)=\sum_{i=1}^{n}\log p(y_i\mid x_i;\theta)\]$

This makes differentiation easier, improves numerical stability, and does not change the maximizer since log is monotonic.


## Q3. Can we use perceptron instead of sigmoid?

Not in logistic regression. Logistic regression needs sigmoid because it outputs values in (0,1), so they can be interpreted as probabilities.

A perceptron uses a hard threshold and gives only class labels:

$$
\[
\hat y=
\begin{cases}
1, & z \ge 0 \\
0, & z < 0
\end{cases}
\]
$$

So it is not smooth, not differentiable, and does not provide probabilities. Therefore, it is a different model, not logistic regression. But if there are two classes, it is somewhat useful. In case of multiclass, one hot encoding can be used with perceptrons.


# Parameter Estimation


## Q1. What is the frequentist and the Bayesian approach?
In the **frequentist approach**, the parameter is treated as a **fixed but unknown value**.
The data is random, and inference is done by estimating that fixed parameter from the observed data.

In the **Bayesian approach**, the parameter is treated as an **unknown random variable**.
We start with a **prior** distribution, observe data, and update it to a **posterior** using Bayes' rule:

$$p(\theta \mid D) \propto p(D \mid \theta)\, p(\theta)$$



## Q2. Do we use MLE in the frequentist approach? Why?
Yes. In the frequentist approach, **MLE (Maximum Likelihood Estimation)** is commonly used because it chooses the parameter value that makes the observed data most likely.

$$\hat{\theta}\_{\text{MLE}}=\arg\max\_{\theta} p(D \mid \theta)$$

This fits the frequentist view because the parameter is assumed fixed, and we want the best estimate of that fixed value.


## Q3. What do we use in the Bayesian approach?
In the Bayesian approach, we use the **posterior distribution**: $p(\theta \mid D)$

Common Bayesian summaries are:

### MAP estimate
$$\hat{\theta}\_{\text{MAP}}=\arg\max\_{\theta} p(\theta \mid D)$$

### Posterior mean
$$\mathbb{E}\[\theta \mid D]$$

So, unlike frequentist inference, Bayesian inference uses both the **likelihood** and the **prior**.

## Summary
- **Frequentist:** parameter is fixed; often uses **MLE**
- **Bayesian:** parameter is random; uses **posterior distribution**, **MAP**, or **posterior mean**


## Q4. When is the frequentist approach better, and when is the Bayesian approach better?

### Frequentist approach is better when:
- there is a large amount of data,
- prior information is unavailable or unreliable,
- a simpler and faster method is needed,
- classical hypothesis testing or confidence intervals are required.

### Bayesian approach is better when:
- data is limited,
- prior knowledge is available and useful,
- uncertainty must be modeled explicitly,
- a full posterior distribution over parameters is needed,
- the problem involves hierarchical or complex probabilistic modeling.

Frequentist methods are usually preferred for large-data, simpler, standard inference settings. Bayesian methods are preferred when data is small, prior knowledge matters, and uncertainty estimation is important.

## Q5. Why do we use conjugate priors? How do they help?

A **conjugate prior** is a prior that gives a posterior in the same distribution family as the prior.

In Bayesian inference, $\[ p(\theta \mid D) \propto p(D \mid \theta)\, p(\theta)\]$

Using a conjugate prior helps because:

1. **The posterior is easy to derive**  
   It often has a closed-form expression.

2. **Updating is simple**  
   We only need to update the hyperparameters.

3. **Computation is faster**  
   It avoids difficult integration or approximation methods.

4. **Interpretation is easier**  
   The prior often acts like pseudo-counts or prior observations.

### Example
For Bernoulli data, if $\[ \theta \sim \mathrm{Beta}(\alpha,\beta)\]$

then after observing s successes and f failures, $\[\theta \mid D \sim \mathrm{Beta}(\alpha+s,\beta+f)\]$


---

# Deep Learning

## Q1. Why do neural networks need nonlinear activation functions, and how do different activation functions compare in practice?

### Answer
We use **nonlinear activation functions** because stacking only linear layers still gives a **linear function** overall. So without nonlinearity, a deep neural network would behave like a single linear model and could not learn complex patterns.

If

$$
h_1 = W_1 x + b_1,\qquad h_2 = W_2 h_1 + b_2
$$

then

$$
h_2 = W_2(W_1x+b_1)+b_2 = (W_2W_1)x + (W_2b_1+b_2)
$$

which is still linear in $$x$$.

So nonlinear activations allow the network to model:
- curved decision boundaries,
- complex feature interactions,
- hierarchical patterns.

### Common nonlinear activation functions

#### Sigmoid

$$
\sigma(x)=\frac{1}{1+e^{-x}}
$$

**Use:** binary output probability, logistic regression, some gates in LSTM/GRU.  
**Problem:** saturates for large positive or negative values, causing **vanishing gradients**.

#### Tanh

$$
\tanh(x)=\frac{e^x-e^{-x}}{e^x+e^{-x}}
$$

**Use:** hidden states in older RNNs, sometimes hidden layers.  
**Advantage:** zero-centered.  
**Problem:** still suffers from saturation and vanishing gradients.

#### ReLU

$$
\mathrm{ReLU}(x)=\max(0,x)
$$

**Use:** default hidden activation in many deep networks.  
**Advantages:** simple, fast, helps gradient flow better than sigmoid/tanh.  
**Problem:** neurons can die if they always output 0.

#### Leaky ReLU

$$
f(x)=
\begin{cases}
x, & x>0\\
\alpha x, & x\le 0
\end{cases}
$$

**Use:** hidden layers when dead ReLU is a concern.  
**Advantage:** keeps a small gradient for negative inputs.


#### Softmax

$$
\mathrm{softmax}(z_i)=\frac{e^{z_i}}{\sum_j e^{z_j}}
$$

**Use:** output layer for multiclass classification.  
**Purpose:** converts logits into class probabilities summing to 1.

### Which activation is better for which purpose?
- **Hidden layers in deep nets:** ReLU, Leaky ReLU, GELU
- **Binary classification output:** sigmoid
- **Multiclass classification output:** softmax
- **Older RNN hidden states:** tanh
- **Gates in LSTM/GRU:** sigmoid

### Example of a terrible activation function
A random bad choice would be the **step function**:

$$
f(x)=
\begin{cases}
1, & x\ge 0\\
0, & x<0
\end{cases}
$$

It performs terribly as an activation function because:
- it is **not differentiable** at 0,
- its gradient is 0 almost everywhere,
- gradient-based learning cannot update weights properly.

Another poor choice is a **constant function**, like

$$
f(x)=1
$$

because it gives the same output for every input, so the network cannot learn useful distinctions.


## Q2. What is a multilayer perceptron, and why is it called a fully connected neural network?

### Answer
An **MLP (Multilayer Perceptron)** is a feedforward neural network made of layers of neurons where each neuron in one layer is connected to every neuron in the next layer.

A typical layer computes:

$$
z^{(l)} = W^{(l)} a^{(l-1)} + b^{(l)}
$$

$$
a^{(l)} = \phi\big(z^{(l)}\big)
$$

where:
- $$W^{(l)}$$ = weight matrix,
- $$b^{(l)}$$ = bias vector,
- $$\phi$$ = activation function.

It is called **fully connected** because all units of one layer connect to all units of the next layer.

### Main properties
- works on fixed-size vector input,
- learns nonlinear mappings,
- widely used for tabular data and as building blocks inside larger models.


## Q3. Where is the softmax function used in a neural network, and what is its purpose?

### Answer
Softmax is usually applied at the **output layer** in a **multiclass classification** problem.

If the final layer produces logits

$$
z_1,z_2,\dots,z_C
$$

then softmax converts them into probabilities:

$$
\hat y_i = \frac{e^{z_i}}{\sum_{j=1}^{C} e^{z_j}}
$$

### Why do we use it?
- it converts arbitrary real-valued scores into probabilities,
- each output lies in $$\[0,1]$$,
- all class probabilities sum to 1:
- $ \sum_{i=1}^{C}\hat y_i = 1 $

So softmax is used **after the final linear layer** and **before or together with cross-entropy loss**.


## Q4. What do the forward pass and backward pass do?

### Reformulated question
What is computed in the forward pass and backward pass of a neural network?

### Answer

### Forward pass
In the **forward pass**, the input is passed through the network to compute:
- intermediate activations,
- final output,
- loss.

For each layer:

$$
z^{(l)} = W^{(l)} a^{(l-1)} + b^{(l)}
$$

$$
a^{(l)} = \phi(z^{(l)})
$$

At the end, we compute the prediction and the loss:

$$
L(\hat y, y)
$$

So the forward pass answers:
**“Given the current parameters, what does the model predict, and how wrong is it?”**

### Backward pass
In the **backward pass**, we compute gradients of the loss with respect to parameters using the chain rule:

$$
\frac{\partial L}{\partial W^{(l)}},\qquad \frac{\partial L}{\partial b^{(l)}}
$$

This tells us how each weight and bias should change to reduce the loss.

Then an optimizer such as gradient descent updates the parameters:

$$
\theta \leftarrow \theta - \eta \nabla_\theta L
$$

So the backward pass answers:
**“How should the parameters change to reduce the loss?”**

---

## Q5.

#### Q1. What is the true risk in supervised learning?
**Answer:**  
The true risk is the expected loss over the actual data-generating distribution:

$$
J^*(\theta)=\mathbb{E}\_{(x,y)\sim p_{\mathrm{data}}} \big\[L(f(x;\theta),y)\big]
$$

#### Q2. What is empirical risk?
**Answer:**  
Empirical risk is the average loss over the finite training set:

$$
J(\theta)=\frac{1}{m}\sum_{i=1}^{m} L\big(f(x^{(i)};\theta),y^{(i)}\big)
$$

#### Q3. Why do we minimize empirical risk instead of true risk?
**Answer:**  
Because the true data-generating distribution $$p_{\mathrm{data}}$$ is unknown. We only have the training samples.

#### Q4. Why is optimization not the same as learning?
**Answer:**  
Because optimization only reduces training loss, while learning means achieving low error on unseen data too.

---

## Q6. Why are optimization and learning not the same thing? When are they the same?
/ Why is minimizing the training objective different from learning a good model, and under what condition do they become nearly the same?

### Answer
**Optimization** means minimizing the chosen objective on the training data, usually the empirical loss:

$$
J(\theta)=\frac{1}{m}\sum_{i=1}^{m}L\big(f(x^{(i)};\theta),y^{(i)}\big)
$$

**Learning** means obtaining a model that generalizes well to unseen data, that is, has low true risk: 

$$
J^*(\theta)=\mathbb{E}\_{(x,y)\sim p_{\mathrm{data}}} \big\[L(f(x;\theta),y)\big]
$$

So they are not the same because:
- optimization focuses on **training performance**,
- learning focuses on **generalization**.

A model may optimize training loss extremely well and still overfit.

### When are they the same or nearly the same?
They become nearly the same when:
- the training set is large and representative,
- overfitting is controlled,
- empirical risk is a good approximation of true risk.

Then minimizing empirical loss also tends to minimize true loss.


## Q7. What are the main optimization difficulties in deep learning?

### Answer

### 1. Local minima
The loss function may have many minima. Gradient descent can get trapped in a local minimum instead of the global minimum.

### 2. Saddle points
A saddle point is a point where the gradient is zero, but it is not a minimum in all directions. Training can slow down or get stuck near such points.

### 3. Vanishing gradients
In deep networks, gradients can become extremely small as they are backpropagated through many layers. Then early layers learn very slowly.

### 4. Exploding gradients
Gradients can also become extremely large, causing unstable updates and divergence.

### 5. Non-convex loss surface
Deep neural network losses are usually non-convex, so optimization is harder than in convex problems.

### 6. Poor conditioning
If the loss surface is steep in some directions and flat in others, optimization becomes slow and unstable.


## Q8. What does it mean for a function to be convex, and why is convexity useful in optimization?

### Answer
A function $$f:X\to \mathbb{R}$$ is **convex** if for any $$x,x' \in X$$ and any $$0\le \lambda \le 1$$,

$$
f(\lambda x + (1-\lambda)x') \le \lambda f(x) + (1-\lambda)f(x')
$$


The line segment joining two points on the graph lies **above or on** the graph of the function.

### Why is it useful?
If a loss function is convex:
- every local minimum is also a global minimum,
- optimization is easier,
- gradient descent has much better theoretical behavior.

In deep learning, losses are usually **not convex**, which is why optimization is harder.


## Q9. Under what conditions is mini-batch stochastic gradient descent preferable to full batch gradient descent or single-sample SGD?

### Answer
Mini-batch SGD uses a small batch of samples at each update.

### Compared to batch gradient descent
It often outperforms full batch GD when:
- the dataset is large,
- full gradient computation is too slow,
- faster updates are needed,
- hardware such as GPUs can process batches efficiently.

### Compared to pure SGD
It often outperforms one-sample SGD when:
- single-sample gradients are too noisy,
- more stable gradient estimates are needed,
- vectorized computation is beneficial.

### Summary
Mini-batch SGD is usually preferred because it gives a good tradeoff between:
- speed,
- stability,
- memory efficiency,
- hardware utilization.



## Q10. How do the main hyperparameters batch size, learning rate, number of epochs, and weight decay affect training?

### Answer

### 1. Batch size
Batch size is the number of training examples used per update.

#### Small batch size
- noisier gradients,
- more updates per epoch,
- can improve generalization,
- slower hardware utilization.

#### Large batch size
- smoother gradient estimates,
- faster parallel computation,
- may require more memory,
- can sometimes generalize worse.


### 2. Learning rate
Learning rate controls step size in parameter updates: 

$$
\theta \leftarrow \theta - \eta \nabla_\theta L
$$

#### Too small
- very slow training,
- may take too long to converge.

#### Too large
- overshooting,
- oscillation,
- divergence.

#### Well chosen
- stable and fast convergence.


### 3. Number of epochs
An epoch means one full pass through the training set.

#### Too few epochs
- underfitting,
- model has not learned enough.

#### Too many epochs
- risk of overfitting,
- training loss decreases but validation performance may worsen.

So epochs control how long the model trains.


### 4. Weight decay
Weight decay is a regularization term that penalizes large weights, often equivalent to L2 regularization: 

$$
L_{\mathrm{total}} = L_{\mathrm{data}} + \lambda \|W\|_2^2
$$

#### Effect
- discourages overly large weights,
- reduces overfitting,
- can improve generalization.

#### Too much weight decay
- may underfit,
- may prevent the model from learning enough.



## Very short summary

- **Nonlinear activations** allow neural nets to learn complex functions.
- **MLP** is a feedforward fully connected neural network.
- **Softmax** is used at the output layer for multiclass probabilities.
- **Forward pass** computes outputs and loss; **backward pass** computes gradients.
- **Optimization** reduces training loss; **learning** means generalizing well.
- **Main optimization challenges:** local minima, saddle points, vanishing/exploding gradients, non-convexity.
- **Convexity** makes optimization easier because local minima are global minima.
- **Mini-batch SGD** balances speed and stability.
- **Batch size, learning rate, epochs, weight decay** strongly affect convergence and generalization.

So the prior and posterior stay in the same family.

We use conjugate priors mainly because they make Bayesian inference analytically simple, computationally efficient, and easy to update.
