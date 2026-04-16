# Word Embeddings and RNNs — merged concise notes

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
