# Word Embeddings and RNNs

## Why text needs special treatment
Text is not like images or tabular data. A sentence can have variable length, word order matters, and the same word can mean different things in different contexts. Similar meaning can also be expressed using very different words.

So a good text representation should capture:
- identity of words
- similarity between words
- effect of surrounding context
- sequence order

---

## Bag of Words
Bag of Words represents a document using word counts.

### Why it is useful
- simple
- easy to implement
- strong baseline for some document tasks

### Main problem
It ignores word order and meaning. Two texts can mean similar things even if they share few exact words.

### Good choice when
Use it for quick baselines like spam detection, topic classification, or simple sentiment analysis.

---

## Language modeling
A language model predicts the next word from previous words, or more generally assigns probabilities to text sequences.

Example idea:
- `I am very thirsty, I want to drink ...`
- likely completions: `water`, `juice`, etc.

The key point is that prediction depends on **context**.

### Good choice when
Use it for autocomplete, next-word prediction, text generation, and learning useful text representations.

---

## n-gram models
An n-gram model assumes the next word depends only on the previous `n-1` words.

- unigram: 1 word
- bigram: 2 words
- trigram: 3 words

### Strength
- simple and intuitive

### Weakness
- larger `n` gives more context but the count table grows very fast
- unseen phrases become a problem
- long-range context is missed

### Good choice when
Use it for teaching, lightweight baselines, or small classical NLP tasks.

---

## Why one-hot is not enough
A one-hot vector has length equal to vocabulary size. That makes it huge and sparse.

More importantly, it does not show similarity:
- `apple` and `orange` look completely unrelated
- `apple` and `democracy` also look unrelated in the same way

So one-hot only tells the model **which word**, not **how words are related**.

### Good choice when
Use one-hot only for toy examples or to explain lookup mechanics.

---

## Word embeddings
A word embedding is a dense learned vector for a word.

Instead of vocabulary-sized sparse vectors, each word gets a smaller vector such as 50, 100, or 300 dimensions.

### Why embeddings matter
They let similar words stay close in vector space. That means the model can learn that some words are semantically or contextually related.

### Cosine similarity
Similarity between embeddings is often measured using **cosine similarity**:
- higher cosine similarity means vectors point in similar directions
- this usually means similar usage/context

### Good choice when
Use embeddings when meaning, similarity, or transfer to downstream NLP tasks matters.

---

## Word2Vec intuition
Word2Vec learns word vectors from context.

Core idea:

> words appearing in similar contexts should get similar vectors

If an unknown word appears in contexts related to drinking, bottles, or being strong, the model can infer something about its meaning from usage alone.

### Good choice when
Use Word2Vec when you want efficient static word vectors from large unlabeled text.

---

## CBOW and Skip-gram

### CBOW
CBOW predicts the center word from surrounding context words.

### Good choice when
Use CBOW when training speed matters and you want a strong practical baseline.

### Skip-gram
Skip-gram predicts surrounding context words from the center word.

Example:
- center word: `coffee`
- nearby targets: `cup`, `of`, `is`, `on`

### Good choice when
Use skip-gram when rare-word representation matters more.

---

## Word2Vec architecture in simple terms
1. Start with a one-hot input word  
2. Multiply by an embedding matrix  
3. This effectively selects that word’s embedding  
4. Project back to vocabulary-sized scores  
5. Use softmax to get probabilities  

So the hidden layer is really the learned embedding.

### Important practical issue
The expensive part is the output side, because softmax over the whole vocabulary can be very costly.

---

## Cross-entropy loss
When the correct target word is known, the predicted probability distribution is compared with the true one-hot target using **cross-entropy**.

If the correct word gets high probability, loss becomes low.

### Good choice when
Use cross-entropy when one correct class should be selected from many vocabulary options.

---

## Negative sampling
To avoid expensive full softmax, Word2Vec often uses **negative sampling**.

Instead of predicting over the whole vocabulary, the model learns a simpler binary task:

- is this pair a real context pair? → `1`
- or a random non-neighbor pair? → `0`

So instead of one huge softmax, the model solves a few small binary decisions with sigmoid.

### Why it helps
- much faster
- works very well in practice
- avoids the full-vocabulary bottleneck

### Good choice when
Use skip-gram with negative sampling for efficient embedding learning on large corpora.

---

## Window size intuition
The context window controls what kind of similarity is learned.

- **small window** → more local/syntactic similarity
- **large window** → broader semantic/topic relatedness

### Good choice when
Use a small window for local substitution-like behavior, and a larger window for broader relatedness.

---

## Limits of Word2Vec
Word2Vec gives one fixed vector per word. That creates issues for polysemy.

Example:
- `bank` of a river
- `bank` for money

Both senses are merged into one vector.

It also relies on local context windows, so long-range meaning is limited.

---

## Why RNNs are needed
Classical embeddings and n-grams do not fully solve sequence modeling.

We still need a model that:
- handles variable-length input
- keeps track of order
- carries information forward through the sequence
- reuses the same logic at each position

That leads to the **Recurrent Neural Network (RNN)**.

---

## Core RNN idea
At time step `t`, the RNN uses:
- current input `x_t`
- previous hidden state `h_(t-1)`

to produce a new hidden state `h_t`.

The hidden state acts like a running summary of what has been seen so far.

In simple terms:
- current token gives new information
- previous hidden state carries past context
- new hidden state mixes both

---

## Why parameter sharing matters
RNNs use the **same weights at every time step**.

This is important because:
- model size does not grow with sequence length
- variable-length sequences can be handled
- every position is processed with the same rule

Without parameter sharing, sequence models would become too large and inefficient.

---

## Common RNN input-output patterns
- **one-to-many**: one input → sequence output  
  Example: image captioning

- **many-to-one**: sequence input → one output  
  Example: sentiment classification

- **many-to-many**: sequence input → sequence output  
  Example: sequence labeling or video captioning

- **sequence-to-sequence**: encode one sequence, decode another  
  Example: machine translation

---

## Character-level language modeling
Instead of words, the model predicts one character at a time and feeds the generated character back into itself.

### Good choice when
Use it when:
- vocabulary should stay very small
- spelling patterns matter
- unknown words are common
- text is noisy or code-like

---

## Backpropagation Through Time (BPTT)
RNNs are trained by unrolling them across timesteps and backpropagating through the sequence. This is called **Backpropagation Through Time**.

### Truncated BPTT
For long sequences, we often backpropagate through shorter chunks instead of the whole sequence.

### Good choice when
Use truncated BPTT when full-sequence training is too costly.

---

## Main weakness of vanilla RNNs
Vanilla RNNs struggle with long-range dependencies because gradients can:

- **vanish**: become too small
- **explode**: become too large

### Practical control
- exploding gradients → use **gradient clipping**
- vanishing gradients → often change the architecture

---

## LSTM intuition
LSTM is a gated version of RNN designed to preserve information better over long sequences.

It introduces:
- **cell state**
- **input gate**
- **forget gate**
- **output gate**

The main benefit is that information can flow through a more additive path, which usually improves gradient flow and long-term memory.

### Important nuance
LSTM does not magically remove all training problems, but it makes long-distance dependency learning much easier than plain RNN.

### Good choice when
Use LSTM when vanilla RNN forgets important earlier context.


# Vanilla RNN vs LSTM

## Vanilla RNN
At time step \(t\), a vanilla RNN uses:

- current input vector \(x_t\)
- previous hidden state \(h_{t-1}\)

These are concatenated, multiplied by a weight matrix, then passed through a nonlinearity to produce the next hidden state:

$$
h_t = \tanh\!\big(W [x_t ; h_{t-1}] + b\big)
$$

So \(h_t\) is the new summary of the past plus the current input.

### Limitation
Because information must pass through repeated matrix multiplications across time, vanilla RNNs often struggle with long-term dependencies.

---

## LSTM
An LSTM uses:

- current input \(x_t\)
- previous hidden state \(h_{t-1}\)
- previous cell state \(c_{t-1}\)

First, \(x_t\) and \(h_{t-1}\) are stacked and passed through one large linear layer.  
This produces four vectors:

- input gate: \(i_t\)
- forget gate: \(f_t\)
- output gate: \(o_t\)
- candidate gate: \(g_t\)

A compact form is:

$$
[i_t, f_t, o_t, g_t] = W [x_t ; h_{t-1}] + b
$$

Then nonlinearities are applied:

$$
i_t = \sigma(i_t), \qquad
f_t = \sigma(f_t), \qquad
o_t = \sigma(o_t), \qquad
g_t = \tanh(g_t)
$$

Usually these gate vectors have the same size as the hidden state.

---

## Cell state update
The LSTM cell state is updated by:

$$
c_t = f_t \odot c_{t-1} + i_t \odot g_t
$$

where \(\odot\) means elementwise multiplication.

Interpretation:

- \(f_t\): how much of the old cell state to keep
- \(i_t\): whether to write new information
- \(g_t\): what new information to write

---

## Hidden state update
After updating the cell state, the hidden state is:

$$
h_t = o_t \odot \tanh(c_t)
$$

So the output gate controls how much of the cell state is revealed.

---

## Main idea
Vanilla RNN keeps only a hidden state.  
LSTM keeps both a hidden state and a cell state, and uses gates to control forgetting, writing, and revealing information.

This gated additive update makes LSTMs much better at handling longer dependencies than vanilla RNNs.

If the forget gates were exactly 1, then:

$$c_t = c_{t-1} + \text{new stuff}$$

and

$$\frac{\partial c_t}{\partial c_{t-1}} = 1$$

So the gradient could pass backward unchanged along that path. That would be truly uninterrupted.

But in practice:

$$f_t \in (0, 1)$$

because it is produced by a sigmoid. So the gradient is actually multiplied by $f_t$ at each step. Thus the path is not perfectly constant unless the forget gate stays very close to 1.

So the better statement is:

- LSTM provides a much easier path for gradient flow
- It can preserve gradients for long durations
- But it does not guarantee a perfectly unchanged gradient forever

At each time step:

- The **forget gate** decides how much old information to keep
- The **input gate** decides how much new information to write
- The **output gate** decides how much of the cell content to expose as hidden state

Because the cell state mostly carries information forward directly, the backward gradient also has a cleaner route backward. That is the main reason LSTMs can remember information over much longer spans than vanilla RNNs.

---

## Does LSTM solve the vanishing gradient problem?

It **greatly reduces** it, but does **not completely eliminate** it. This is the correct answer.

### Why it helps

The cell-state path avoids repeated multiplication by a full recurrent matrix and repeated squashing nonlinearities in the long-term memory path. If the forget gate stays near 1, then:

$$\frac{\partial c_t}{\partial c_{t-1}} \approx 1$$

and gradients can survive for many steps. This is why LSTMs are much better than vanilla RNNs at learning long-term dependencies.

### Why it does not fully solve it

If the forget gates are consistently less than 1, say $f_t \approx 0.8$, then over many steps:

$$0.8^{100}$$

is tiny. So gradients can still vanish eventually.

Also, gradients going through the gates and through the hidden state still pass through sigmoids and tanh, which can saturate.

So LSTM **mitigates** vanishing gradients strongly, but does not mathematically abolish them in every circumstance.

---

## Does LSTM solve the exploding gradient problem?

**Not completely.** LSTM helps stability, but exploding gradients can still happen.

### Why exploding is less severe than in vanilla RNN

The cell-state path is more controlled. The additive structure is more stable than repeated multiplication by a recurrent matrix. Also, forget gates are bounded between 0 and 1, so along the direct cell-state path, you do not get uncontrolled growth from that term alone.

### But exploding can still happen

The overall network still contains:

- Matrix multiplications for the gates
- Hidden-to-hidden dependencies
- Backpropagation through many steps
- Output layers and other transformations

So the total gradient in the whole model can still become large. That is why in practice people still use:

- **Gradient clipping**
- **Careful initialization**
- **Proper learning rates**

with LSTMs.

So LSTM does **not** fully solve exploding gradients either.


---

## GRU
GRU is another gated recurrent model, simpler than LSTM in structure and often competitive in practice.

### Good choice when
Use GRU when you want something lighter than LSTM but stronger than vanilla RNN.

---

## Where each method is a good choice

### Bag of Words
- quick baseline document classification
- simple interpretable pipelines

### n-gram model
- short-context prediction
- teaching and classical baselines

### Word embeddings / Word2Vec
- semantic similarity
- search and retrieval
- recommendation
- downstream NLP initialization

### CBOW
- faster embedding training

### Skip-gram + negative sampling
- efficient large-corpus training
- better handling of rare words

### Vanilla RNN
- educational sequence baseline
- simple sequential tasks

### LSTM / GRU
- longer dependencies
- practical sequence modeling
- text generation, classification, captioning baselines

---

## Final takeaway
A useful progression is:

1. **Bag of Words / n-grams** → simple count-based baselines  
2. **Word embeddings / Word2Vec** → dense meaning-aware word vectors  
3. **RNNs** → sequence models with memory  
4. **LSTM / GRU** → better long-range sequence modeling  

The main idea is simple:

> in language, meaning usually depends on context, similarity, and sequence order — not just isolated words.


---

# Why do CNNs work well with images?

Images are not just random collections of numbers. They have **structure**:

- Nearby pixels are usually related.
- Small patterns like edges, corners, and textures matter.
- The same object feature can appear in different parts of the image.

A **Convolutional Neural Network (CNN)** is designed to use this structure efficiently.

## 1. Local patterns matter

In an image, a pixel by itself usually means very little. What matters is the pattern formed by **neighboring pixels**.

For example:

- an edge comes from a small change in nearby pixel values
- a corner comes from a particular local arrangement
- textures are repeated local patterns

CNNs use **small filters/kernels** that look at local regions such as `3x3` or `5x5`. This helps the network detect meaningful visual features.

---

## 2. The same feature can appear anywhere

A cat’s eye is still a cat’s eye whether it is near the top-left or the center of the image.

CNNs use the **same filter across the whole image**. This is called **weight sharing**.

That means:

- fewer parameters
- less memory needed
- better generalization
- ability to detect the same pattern in different locations

So instead of learning one “edge detector” for every possible position, the CNN learns one filter and slides it over the image.

---

## 3. CNNs build features hierarchically

CNNs often learn features in stages:

- early layers: edges, simple lines
- middle layers: corners, curves, textures, shapes
- deeper layers: object parts
- final layers: full objects

This matches how images are naturally organized: simple patterns combine into more complex ones.

---

## 4. They are parameter-efficient

Suppose you have a grayscale image of size `100 x 100`.

A fully connected neuron receiving this image as input would need:

`100 x 100 = 10,000` weights for just **one neuron**

If the next layer has 100 neurons, that is already:

`10,000 x 100 = 1,000,000` weights

That becomes very large very quickly.

A CNN filter of size `3x3` only needs:

`9` weights per channel

Even with many filters, this is much smaller than fully connected connections over the whole image.

So CNNs are much more practical for images.

---

# Why not use a single perceptron?

A **single perceptron** computes something like this:

\[
y = f(w_1x_1 + w_2x_2 + \cdots + w_nx_n + b)
\]

It creates only a **linear decision boundary**.

## Problems with a single perceptron for images

### 1. It treats all pixels independently
A single perceptron does not naturally understand that neighboring pixels are related.

It just sees a long list of numbers.

For example, a `28x28` image becomes a vector of length `784`. The perceptron does not know which pixels are next to each other unless we somehow force that structure into the design.

### 2. It cannot capture complex visual patterns well
Real image tasks are highly nonlinear.

To distinguish objects, the model must detect combinations of edges, textures, shapes, and arrangements. A single perceptron is too simple for that.

### 3. It is not translation-friendly
If the same object moves slightly in the image, the raw pixel positions change a lot. A single perceptron usually does not handle this nicely.

CNNs handle this better because the same filters are applied everywhere.

---

# What happens if I put a picture into a neural network?

A picture is just turned into numbers.

For example:

- grayscale image: a 2D matrix of intensities
- RGB image: three 2D matrices, one for red, green, and blue

A neural network does not “see” the image like a human. It only receives numeric values.

## Case 1: Put the image into a fully connected neural network

If you flatten the image, for example:

- `28 x 28` image  
becomes
- a vector of length `784`

Then the network processes it like any other input vector.

### What goes wrong?
- spatial structure is lost in the input representation
- the network must learn everything from scratch
- many parameters are needed
- training may be harder
- it may overfit more easily

It can still work for small simple datasets, but it is usually not the best choice for image tasks.

---

## Case 2: Put the image into a CNN

The CNN keeps the image structure and processes local regions first.

Instead of flattening immediately, it applies convolutions over the image.

This allows the network to:

- preserve spatial relationships
- detect local features
- combine features into more meaningful representations

That is why CNNs usually perform much better on images than plain fully connected networks.

---

# Intuition with an example

Suppose you want to detect whether an image contains the digit **7**.

A single perceptron would try to assign weights directly to all pixels and decide from one weighted sum.

But the digit **7** can be:

- slightly shifted
- thicker or thinner
- brighter or darker
- handwritten in many styles

A CNN first learns small useful patterns like:

- horizontal edges
- slanted strokes
- intersections

Then deeper layers combine those patterns into something that looks like a **7**.

This makes the model much better at recognizing the digit in many variations.

---

# In short

## Why do CNNs work well with images?
Because they exploit the natural structure of images:

- local connectivity
- shared patterns across space
- hierarchical feature learning
- fewer parameters

## Why not a single perceptron?
Because it is too simple:

- only linear
- ignores spatial relationships
- cannot capture complex image structure well

## What happens if I put a picture into a neural network?
The image becomes numbers. Then:

- in a plain neural network, it is usually flattened into a long vector
- in a CNN, its spatial structure is preserved and used effectively

---

# One-sentence summary

A CNN works well for images because it is built to recognize local visual patterns efficiently, while a single perceptron is too simple and ignores the spatial structure that makes images meaningful.


---

# Kernel function, overlap, convolution, and how CNNs know “which feature is where”

These ideas are closely connected, but the word **kernel** is used in more than one way in machine learning and signal processing. In CNNs, a **kernel** usually means a small learnable filter such as `3x3` or `5x5`.

---
## 1. What is a kernel in a CNN?

A kernel is a small matrix of weights. For example, a `3x3` kernel might look like this:

$$K = \begin{bmatrix} -1 & 0 & 1 \\ -1 & 0 & 1 \\ -1 & 0 & 1 \end{bmatrix}$$

This kernel is placed over a small patch of the image, and the network computes a weighted sum between the kernel and that patch. If the patch is

$$P = \begin{bmatrix} 2 & 3 & 7 \\ 1 & 4 & 8 \\ 0 & 2 & 9 \end{bmatrix}$$

then the response is:

$$(-1)(2) + 0(3) + 1(7) + (-1)(1) + 0(4) + 1(8) + (-1)(0) + 0(2) + 1(9)$$

A large positive or negative value means the patch matches the pattern the kernel is looking for. So a kernel is basically a **small pattern detector**.

---

## 2. What does “overlap” with the kernel mean?

When people say a patch “overlaps well” with a kernel, they usually mean:

- the patch has a pattern similar to what the kernel is designed or trained to detect
- so the weighted sum becomes large in magnitude

Mathematically, this is like a **dot product** between the kernel and the local image patch.

If the patch and kernel align well, the output is strong.

If they do not align well, the output is weak.

So from this overlap, the network extracts properties like:

- edges
- corners
- lines
- texture patterns
- color transitions
- small repeated motifs

For example:

- a vertical-edge kernel responds strongly where intensity changes from left to right
- a horizontal-edge kernel responds strongly where intensity changes from top to bottom

At deeper layers, the same idea continues, but now the “patches” are not raw pixels. They are patches of earlier **feature maps**, so the kernel can detect more abstract things like:

- eye-like shapes
- wheel-like curves
- text strokes
- object parts

---

## 3. How do we extract different properties from kernel overlap?

Different kernels learn to respond to different patterns.

### Early layers
Usually learn simple features like:

- vertical edges
- horizontal edges
- diagonal edges
- blobs
- local texture

### Middle layers
Learn combinations of earlier features:

- corners
- curves
- repeated textures
- simple shapes

### Deeper layers
Learn more semantic patterns:

- face parts
- windows
- wheels
- leaves
- letters
- object fragments

So the “different properties” come from the fact that **different kernels have different weights**.

One kernel may strongly activate on vertical edges. Another may activate on checkerboard-like texture. Another may activate on red-green contrast.

Each kernel produces one feature map, showing **where in the image that feature appears**.

---

## 4. Where does convolution come in?

Convolution is the operation that applies the kernel across the image.

The kernel is not used only once. It slides over the image:

- first top-left patch
- then slightly shifted patch
- then next patch
- and so on

At each location, it computes a weighted sum.

This produces an output map called a **feature map**.

So convolution is what lets the network ask:

> “Does this local pattern appear here?”
>  
> “Does it appear here?”
>  
> “Does it appear here?”

across the whole image.

### In practice
In deep learning, what is called “convolution” is often technically closer to **cross-correlation**, because the kernel is usually not flipped. But everyone still calls it convolution.

---

## 5. Why is convolution useful?

Because the same feature can appear anywhere.

A vertical edge is still a vertical edge whether it is:

- on the left side
- in the center
- on the right side

Instead of learning separate detectors for every image position, convolution uses the **same kernel everywhere**.

This gives two big benefits:

### Parameter sharing
The same kernel weights are reused across all locations.

This means:

- fewer parameters
- less memory
- better learning efficiency

### Translation sensitivity in a useful way
If a feature moves slightly, the same kernel can still detect it in the new location.

That makes CNNs much more suitable for images than a fully connected network.

---

## 6. How does the network know which feature is in which patch?

This is the key idea:

A convolution output is not just a number. It is a **2D map of numbers**.

Each position in that output map corresponds to a particular region of the input image.

So if a feature map has a strong activation at row 10, column 15, that means:

- the kernel detected its pattern strongly
- in the patch of the image corresponding to that spatial location

So the network knows both:

- **what feature** was detected  
  because it knows which kernel produced the activation

- **where it was detected**  
  because it knows the spatial position in the feature map

### Very important idea
Each channel says **what**
Each spatial location says **where**

So in a feature tensor:

$$H \times W \times C$$

- `H x W` tells where
- `C` tells what kind of feature

---

## 7. Then how does the network keep track of location through many layers?

Each deeper neuron sees a larger effective region of the original image. This is called the **receptive field**.

At first:

- one output cell may look at a tiny `3x3` patch

After several layers:

- one deeper output cell may correspond to a much larger region of the image

So the network gradually builds:

- local information first
- then broader context

This lets it detect things like:

- a short edge
- then a corner made of edges
- then a shape made of corners
- then an object part made of shapes

The spatial arrangement is preserved layer by layer, though sometimes at lower resolution.

---

## 8. What about pooling? Does the network lose location?

Yes, partially.

Operations like **max pooling** reduce resolution. For example, a `2x2` pooling layer might reduce a `32x32` map to `16x16`.

This helps with:

- reducing computation
- making features more robust to small shifts

But it also means exact location becomes less precise.

So CNNs often balance:

- keeping enough spatial information
- becoming robust to small translations

In tasks like segmentation or detection, architectures are designed to preserve or recover more detailed spatial information.

---

## 9. Example intuition

Suppose a kernel is an edge detector.

You slide it across the image.

- In a smooth region, response is small.
- At a vertical boundary, response becomes large.
- So the feature map lights up where vertical edges exist.

Now imagine 64 different kernels in the first layer.

Then the network creates 64 feature maps:

- one for vertical edges
- one for horizontal edges
- one for diagonal edges
- one for dark-to-light transitions
- one for texture
- etc.

So the network is building a stack of maps saying:

- where each kind of basic pattern appears

Then the next layer looks at those maps and asks:

- do certain edge combinations form a corner?
- do several corners and curves form an eye?
- do certain textures and contours form fur?

That is how feature extraction grows in complexity.

---

## 10. If the same kernel is used everywhere, how does it know left eye vs right eye?

The kernel itself does **not** know “left eye” or “right eye”.

The kernel only knows:

- “I see an eye-like pattern here.”

The **position in the feature map** tells where it was found.

Later layers combine:

- feature type
- spatial arrangement

So a later layer can learn things like:

- two eye-like activations side by side above a nose-like activation may indicate a face
- a wheel-like feature near the bottom of a car-shaped contour may indicate a car

So location is not stored in the kernel weights themselves. It is stored in the **activation positions** and how later layers combine them.

---

## 11. Why not just flatten the image and use a normal neural network?

Because then you lose the natural spatial structure.

Flattening turns an image into a long vector:

\[
x_1, x_2, x_3, \dots, x_n
\]

A dense layer can still learn from it, but:

- it must learn spatial relationships from scratch
- it uses many more parameters
- it does not naturally reuse the same pattern detector across locations

CNNs solve this by building locality and weight sharing directly into the architecture.

---

## 12. Summary in simple terms

A CNN kernel is a small learned pattern detector.

Convolution means:

- slide that detector over the image
- compute how strongly each patch matches it
- build a map of where that feature appears

From this process, the network learns:

- **what** kind of feature exists, from which kernel activated
- **where** it exists, from where the activation occurs in the feature map

Then deeper layers combine many local detections into larger, more meaningful patterns.

---

## One-line intuition

A kernel asks, at every small patch:  
**“Does this patch look like the pattern I care about?”**  
Convolution repeats that question everywhere, and the resulting feature map tells the network both **what was found** and **where it was found**.
