# Word Embeddings and Related NLP Ideas

This note explains the main ideas around representing text for machine learning, especially **word embeddings**, **Word2Vec**, and the transition from simple count-based methods to neural sequence models.

The goal is to explain **what each idea means**, **why it was introduced**, and **when it is a good choice**.

---

## 1. Why text is harder than images or tabular data

Neural networks want **numbers** as input.  
Images are already numeric grids of pixel values.  
Text is different: words are symbolic, discrete, and highly context-dependent.

A few major difficulties are:

- **Same word, different meaning**  
  Example: *park* in “parallel park” vs *park* in “walk in the park”.

- **Typos and spelling variation**  
  Example: “lake” vs “lale”.

- **Paraphrases / synonyms**  
  Example: “Obama speaks to the media” and “The president greets the press” may mean similar things with very different words.

- **Word order matters**  
  “Parts are interchangeable but not similar” is not the same as  
  “Parts are similar but not interchangeable”.

So, a good text representation should not just store identity of words, but should capture **similarity, context, and meaning**.

### Good choice scenario
This framing is useful whenever you are designing an NLP system and need to justify why naive numeric encodings are not enough.

---

## 2. Why one-hot vectors are not enough

A very naive approach is to give each unique word in the vocabulary a separate index and represent it with a **one-hot vector**.

If the vocabulary size is \(|V|\), then each word gets a vector of length \(|V|\) where:

- one position is 1
- all other positions are 0

For example, if vocabulary size is 100,000, every word gets a 100,000-dimensional vector.

### Problems with one-hot encoding

1. **Huge dimensionality**  
   The vector length equals vocabulary size.

2. **No similarity information**  
   “apple” and “orange” are both fruits, but their one-hot vectors are orthogonal.  
   So contextually or semantically similar words are **not close** under one-hot encoding.

3. **Very sparse**  
   Mostly zeros, which is inefficient.

A useful intuition is this: if two words behave similarly in language, we want the model to feel that similarity. One-hot cannot do that.

### Good choice scenario
One-hot is okay only for **toy examples**, very small vocabularies, or when teaching the mechanics of lookup tables and matrix multiplication.

---

## 3. A helpful intuition: representing things as vectors

Before thinking about words, it helps to think about representing **anything** as a vector.

Imagine a person described by a few numerical traits. Then that person can be represented as a point in space. Two similar people will have vectors pointing in similar directions.

This leads to two core ideas:

1. **Things can be represented as vectors**
2. **Similarity between vectors can be computed**

The most common similarity measure here is **cosine similarity**.

\[
\cos(\theta)=\frac{x \cdot y}{\|x\|\|y\|}
\]

If the cosine value is larger, the vectors are more aligned, so the items are more similar.

### Why this matters for words
If words are represented as dense vectors, then semantically related words can end up near each other.

### Good choice scenario
This explanation is a good choice when introducing embeddings to beginners, especially if they are confused about why vectors are useful at all.

---

## 4. Bag of Words (BoW)

A classic document representation is **Bag of Words**.

Here, a document is represented by counts of words:

- how many times “great” appears
- how many times “movie” appears
- how many times “boring” appears
- etc.

### Strengths
- Simple
- Easy to implement
- Works surprisingly well for some classification tasks

### Weaknesses
- Very high dimensional
- Ignores word order
- Does not capture semantic similarity
- Documents with no exact shared words may still be very similar in meaning, but BoW misses that

### Good choice scenario
Bag of Words is still a good baseline for:
- simple sentiment classification
- spam detection
- quick text prototypes
- low-resource educational settings

If interpretability and simplicity matter more than semantic richness, BoW can still be a good first model.

---

## 5. Language modeling

A **language model** assigns probabilities to sequences of words.

For a sentence \(x_1, x_2, \dots, x_T\),

\[
P(x_1, x_2, \dots, x_T)
=
P(x_1)P(x_2|x_1)P(x_3|x_1,x_2)\cdots P(x_T|x_1,\dots,x_{T-1})
\]

A very practical interpretation is:

> Given the current text, predict the next word.

For example, in a phrase like:

> “The quick brown fox jumps over ...”

the model tries to estimate probabilities for all words in the vocabulary and assign the highest score to the most likely next word.

### Important practical point
The output is not just one word.  
The model produces a **probability distribution over the whole vocabulary**.

So if vocabulary size is 1 million, the output layer is effectively 1 million-dimensional.

### Good choice scenario
Language modeling is a good choice when you need:
- next-word prediction
- autocomplete
- text generation
- scoring whether a sentence sounds natural

---

## 6. n-gram language models

Before neural methods became dominant, one common idea was the **n-gram model**.

An n-gram is a chunk of \(n\) consecutive words.

Examples:
- unigram: one word
- bigram: two consecutive words
- trigram: three consecutive words

A bigram model assumes:

\[
P(x_t|x_1,\dots,x_{t-1}) \approx P(x_t|x_{t-1})
\]

A trigram model assumes dependence on the previous two words, and so on.

### Strengths
- Conceptually simple
- Fast to explain
- Can work decently with enough counts

### Weaknesses
- Larger \(n\) gives more context but causes a combinatorial explosion in the counting table
- Small \(n\) misses long-range context
- Rare or unseen sequences become a big problem

A famous sentence like:

> “Colorless green ideas sleep furiously.”

is grammatically valid but statistically strange.  
This shows that language is more than local word frequency.

### Good choice scenario
n-grams are a good choice for:
- introductory NLP courses
- very lightweight predictive text systems
- rule-heavy or low-compute pipelines
- understanding why neural methods were needed

---

## 7. Why embeddings are needed

Word embeddings solve two big problems at once:

1. They convert words into **numerical inputs**
2. They place similar words near each other in vector space

Instead of vocabulary-sized one-hot vectors, each word gets a much smaller dense vector, for example:

- 50 dimensions
- 100 dimensions
- 300 dimensions

These dense vectors can encode many latent properties at once.  
Not in a manually labeled way, but in a learned way.

For example, a word vector may implicitly reflect tendencies related to:
- person vs object
- singular vs plural
- gendered usage
- royalty
- actions
- semantic field

### Good choice scenario
Embeddings are a good choice whenever:
- you want semantic similarity
- exact word overlap is not enough
- your downstream model needs dense numeric input
- you want transferability to many later tasks

---

## 8. What word embeddings are

A **word embedding** is a dense vector representation of a word that captures information from how that word appears in context.

A useful informal principle is:

> Similar words appear in similar contexts.

If a mysterious word appears in sentences like:

- “I love drinking Underberg after a meal.”
- “Underberg is quite strong.”
- “A few bottles of Underberg made me drunk.”

then even if you do not know the dictionary meaning, the context strongly suggests it is some kind of alcoholic drink.

That is the basic intuition behind learned word vectors.

### Good choice scenario
This idea is a good choice in:
- unknown-word meaning inference
- search and retrieval
- recommendation systems
- clustering vocabulary by usage

---

## 9. Semantic similarity and cosine similarity

Once words become vectors, we can compare them using cosine similarity.

If:
- “good” and “nice” are close
- “cat” and “kitten” are close
- “dog” is near them but a bit farther
- “house” is farther away

then the vector space is capturing useful semantic structure.

### Key point
Higher cosine similarity usually means the words are used in more similar ways.

But similarity is task-dependent:

- Sometimes you want **interchangeability**
- Sometimes you want **relatedness**
- Sometimes you want **functional similarity**

That is why training setup and window size matter.

### Good choice scenario
Cosine similarity is a good choice for:
- nearest-neighbor word lookup
- synonym suggestion
- document retrieval using averaged embeddings
- analogy probing

---

## 10. Word analogies

A famous property of learned embeddings is that vector arithmetic sometimes captures meaningful relations.

A classic example is:

\[
\text{king} - \text{man} + \text{woman} \approx \text{queen}
\]

This does **not** mean the vectors literally contain a single “royalty neuron” or “gender neuron”, but it shows that certain relational patterns become linear in the embedding space.

### Why this is interesting
It suggests embeddings capture structured regularities, not just rough similarity.

### Important caution
These analogies are impressive, but they are not perfect and should not be overinterpreted.

### Good choice scenario
Analogy tests are good for:
- sanity-checking learned embeddings
- educational demos
- comparing embedding quality informally

They are **not** the best standalone evaluation for a real application.

---

## 11. Word2Vec: the core idea

**Word2Vec** is a family of methods for learning word embeddings efficiently from raw text.

The two main training styles are:

- **CBOW (Continuous Bag of Words)**  
  Predict the center word from surrounding context

- **Skip-Gram**  
  Predict surrounding context words from the center word

Both rely on local context windows over running text.

### Good choice scenario
Word2Vec is a good choice when:
- you have a reasonably large corpus
- you want simple, fast, non-contextual embeddings
- you need a strong classical baseline
- compute is limited compared to modern large transformers

---

## 12. Sliding window idea

Training samples are produced with a **sliding window** over text.

Suppose the sentence is:

> “The quick brown fox jumps over ...”

A small window moves across the sentence and creates input-target pairs.

This is the core mechanism by which context becomes supervision.

A very important intuition is:

> Nearby words act as clues for each other.

### Good choice scenario
The sliding-window view is the right explanation when teaching how raw text becomes a training dataset without manual labels.

---

## 13. CBOW (Continuous Bag of Words)

In **CBOW**, the context words are used to predict the center word.

For example, with a small window:

- context: “cup”, “of”, “is”, “on”
- target: “coffee”

So the model learns:

> Given nearby words, which word best fits in the middle?

### Intuition
CBOW combines information from surrounding words and predicts the missing/center word.

### Strengths
- Often faster to train
- Can work well for frequent words
- Stable on large corpora

### Weaknesses
- May smooth over fine-grained distinctions
- Less focused on rare words than skip-gram in some settings

### Good choice scenario
CBOW is a good choice when:
- you want efficient training
- the corpus is large
- the vocabulary has many common words
- you want a solid practical baseline

---

## 14. Skip-Gram

In **Skip-Gram**, the center word is used to predict nearby context words.

For example, from the sentence:

> “A cup of coffee is on the table”

if the center word is “coffee” and the window size is 2, possible training pairs are:

- (coffee, cup)
- (coffee, of)
- (coffee, is)
- (coffee, on)

So one center word generates multiple training examples.

### Intuition
Skip-gram asks:

> If I know this word, what words are likely to appear around it?

### Strengths
- Often better for rare words
- Produces many training pairs
- Very influential historically

### Weaknesses
- Can be more expensive than CBOW in basic form

### Good choice scenario
Skip-gram is a good choice when:
- your corpus is not enormous
- rare word quality matters
- you want classical Word2Vec behavior

---

## 15. Word2Vec architecture intuition

The basic skip-gram architecture can be understood in a very concrete way.

### Step 1: one-hot input
The input word is represented as a one-hot vector of size \(|V|\).

### Step 2: projection to embedding space
Multiplying the one-hot vector by matrix \(W\) simply **selects one row** of \(W\).  
That selected row is the embedding of the input word.

This is why one-hot input is mainly a lookup trick.

If \(W \in \mathbb{R}^{|V| \times d}\), then:
- \(|V|\) = vocabulary size
- \(d\) = embedding dimension, maybe 50, 100, or 300

The hidden layer output is therefore the learned embedding vector.

### Step 3: project back to vocabulary space
Then another matrix \(W'\) projects the embedding back to a \(|V|\)-dimensional output.

### Step 4: softmax
A softmax turns those scores into probabilities across the full vocabulary.

### Why this becomes slow
If the vocabulary is very large, softmax over all words is expensive.  
That full-vocabulary output layer becomes the computational bottleneck.

### Good choice scenario
This explanation is a good choice when students are confused about:
- what the hidden layer really means
- why multiplying by a one-hot vector is basically a lookup
- why the expensive part is the output side, not the embedding lookup side

---

## 16. Cross-entropy loss

When the target context word is known, we compare:

- the model’s predicted probability distribution
- the true one-hot target distribution

This is done with **cross-entropy loss**.

If the correct target word has high predicted probability, the loss is low.

### Intuition
Cross-entropy punishes the model when it places probability mass on wrong words instead of the correct word.

### Good choice scenario
Cross-entropy is the standard choice when:
- exactly one class should be correct
- output is a probability distribution over vocabulary
- you are doing next-word or context-word prediction

---

## 17. Why plain softmax training is expensive

In the naive skip-gram setup, for every training pair, the model must compute scores for **every word in the vocabulary**.

That means:

- large output vector
- large softmax normalization
- slow training

If the vocabulary is huge, this becomes the bottleneck.

So even though the learned embedding is small, the prediction step is not small.

### Good choice scenario
This explanation is important whenever someone asks:
- “Why was Word2Vec considered efficient if there is still a huge vocabulary output?”
- “Where exactly is the bottleneck?”

---

## 18. Negative sampling

To avoid the cost of full softmax, Word2Vec often uses **negative sampling**.

Instead of asking:

> Which word among the entire vocabulary is the correct context word?

we ask a simpler binary question:

> Are these two words neighbors in context or not?

So each training example becomes a word pair with label:

- **1** for a true neighboring pair
- **0** for a randomly sampled non-neighboring pair

### Example
Positive pair:
- (coffee, cup) → 1

Negative pairs:
- (coffee, democracy) → 0
- (coffee, elephant) → 0
- (coffee, algebra) → 0

Now the model does not need full softmax over the whole vocabulary.

### Why sigmoid appears here
Since the target is binary (neighbor or not), we use a **sigmoid** output instead of full softmax.

This is a major speedup.

### Important intuition
If you trained only on positive pairs, the model could cheat by always predicting 1.  
So negative examples are necessary.

### Good choice scenario
Negative sampling is a good choice when:
- vocabulary is large
- you need efficient embedding training
- exact normalized probabilities over the whole vocabulary are unnecessary

---

## 19. Skip-Gram with Negative Sampling (SGNS)

This combination is often written as **SGNS**.

It brings together:

- **Skip-Gram**: center word predicts context words
- **Negative Sampling**: binary classification on sampled positive and negative pairs

This became one of the most influential practical ways to train word embeddings.

### Why SGNS works well
It balances:

- useful training signal from real context
- computational efficiency from sampling
- scalable learning on large corpora

### Good choice scenario
SGNS is a strong choice when:
- you want a proven classical embedding method
- training time matters
- corpus is large enough for distributional learning
- contextual transformers are overkill for the task

---

## 20. What is actually learned

During training, there are usually two matrices:

- **Embedding matrix** \(W\)
- **Context matrix** \(W'\)

Each word has:
- one representation when it appears as an input word
- another representation when it appears as a context word

After training, a common practice is to keep **\(W\)** as the learned word embeddings and discard \(W'\).

### Good choice scenario
This detail matters when implementing Word2Vec from scratch or reading older papers/code.

---

## 21. Window size matters

The context window size controls what kind of similarity the model learns.

A rough intuition:

- **Smaller window** → more syntactic / substitutable similarity  
  Words that can appear in similar local slots may become close.

- **Larger window** → broader semantic relatedness  
  Words from the same topic may become close.

So “good” and “bad” may sometimes appear in similar local contexts even though they are opposites. That can make them look similar to a local-context model.

### Good choice scenario
Smaller windows are a good choice when:
- you care about local word function
- POS-like or syntactic behavior matters

Larger windows are a good choice when:
- topic relatedness matters
- document-level semantics matter more

---

## 22. Document similarity and Word Mover’s Distance

Once words have embeddings, documents can also be compared semantically.

One elegant method is **Word Mover’s Distance (WMD)**.

It measures how much “travel” is needed in embedding space to move the words of one document to the words of another.

This helps when two documents mean similar things but use different words.

### Example intuition
A document about:

- “Obama speaks to the media in Illinois”

may be close to one about:

- “The president greets the press in Chicago”

even though they share few exact tokens.

### Good choice scenario
WMD is a good choice when:
- semantic document similarity matters
- exact token overlap is weak
- the dataset is not so huge that transport-based distance becomes too slow

---

## 23. X2Vec family

The same embedding idea extends beyond words.

Examples:
- **Word2Vec**: word embeddings
- **Doc2Vec**: document or paragraph embeddings
- **Node2Vec**: graph node embeddings
- **Item2Vec**: item embeddings for recommendation
- **Sent2Vec**: sentence embeddings

The common pattern is:

> Learn dense vectors from co-occurrence, neighborhood, or contextual structure.

### Good choice scenario
This family is useful whenever entities have relational context:
- products bought together
- graph nodes connected together
- sentences occurring in related contexts

---

## 24. Doc2Vec

**Doc2Vec** extends the embedding idea to larger text units like paragraphs or documents.

Instead of only learning vectors for words, it also learns vectors representing an entire paragraph/document.

### Applications
- document classification
- sentiment analysis
- recommendation
- information retrieval

### Good choice scenario
Doc2Vec is a good choice when:
- you need a fixed-length representation for variable-length text
- you want something simpler than modern transformer encoders
- you need document-level vector similarity

---

## 25. Problems with Word2Vec

Word2Vec is powerful, but it has important limitations.

### 1. One vector per word type
A word like **bank** gets only one vector, even though it can mean:
- river bank
- financial bank

So multiple meanings get mixed together.

### 2. Limited context
The model only sees words within a local context window.

It does not deeply model long-range compositional meaning.

### 3. Static representation
The word vector is fixed regardless of sentence context.

### Good choice scenario
Because of these limitations, plain Word2Vec is not the best choice when:
- word sense disambiguation is crucial
- context changes meaning heavily
- long-range dependencies matter

---

## 26. Bias and time dependence

Word embeddings can capture real statistical patterns in data, but that includes undesirable patterns too.

### Societal bias
If the training corpus contains stereotypes, the embeddings can reflect them.

### Time dependence
Meaning changes over time.  
Words that were related in one era may not be related in another.

So embeddings are not neutral, timeless truths. They are compressed summaries of usage patterns in the data.

### Good choice scenario
This caution is important in:
- hiring-related NLP
- social analysis
- historical corpora
- fairness-sensitive systems

---

## 27. Moving beyond fixed-size context: RNN motivation

Classical embeddings and n-grams still leave a problem:

- text sequences have variable length
- context can be long
- different positions in a sequence should often be processed with shared logic

A feed-forward network with separate parameters for every position would be inefficient and would not scale well to variable-length text.

This motivates the **Recurrent Neural Network (RNN)**.

---

## 28. RNN intuition

An **RNN** processes a sequence one timestep at a time while carrying a hidden state.

At each step, it uses:

- the current word embedding
- the previous hidden state

to produce:
- a new hidden state
- possibly an output

So the hidden state acts like a memory summary of what has been seen so far.

### Why parameter sharing matters
If every timestep had separate parameters, long sequences would require too many parameters and would generalize poorly.

RNNs fix this by using the **same parameters at every timestep**.

### Strengths
- Can handle variable-length sequences
- Natural sequential structure
- Shared parameters across time

### Weaknesses
- Hard to train for long dependencies compared to later models
- Modern transformers often outperform them

### Good choice scenario
RNNs are a good choice when:
- you want to teach sequence modeling foundations
- the task is small or moderate scale
- streaming/online sequence processing matters
- you want a simple recurrent baseline

---

## 29. When to choose what

## One-hot encoding
Choose it for:
- teaching
- toy examples
- very small vocabularies

Do **not** choose it for meaningful semantic NLP.

## Bag of Words
Choose it for:
- quick baselines
- interpretable text classification
- low-compute pipelines

Do **not** choose it when semantic similarity is central.

## n-gram language model
Choose it for:
- classical NLP understanding
- lightweight local prediction systems
- educational settings

Do **not** choose it when long context matters.

## Word2Vec CBOW
Choose it for:
- efficient embedding training
- large corpora
- frequent-word-heavy settings

## Word2Vec Skip-Gram / SGNS
Choose it for:
- strong classical embeddings
- rare-word sensitivity
- efficient training with large vocabulary

## Doc2Vec
Choose it for:
- document-level fixed vectors
- retrieval and classification

## RNN
Choose it for:
- sequential modeling basics
- variable-length input
- situations where recurrence is a natural fit

---

## 30. Final intuition

A compact way to remember the whole story is:

- **One-hot** tells you only identity
- **Bag of Words** tells you counts
- **n-grams** tell you local sequence counts
- **embeddings** tell you similarity in vector space
- **Word2Vec** learns those vectors from context
- **SGNS** makes that learning efficient
- **RNNs** move from fixed local windows to full sequential processing

Or in simpler words:

> first we counted words,  
> then we learned vectors for words,  
> then we learned models that process whole sequences.

That progression is one of the key historical paths in NLP.

---

## 31. Very short recap

- Text needs numerical representation.
- One-hot is sparse and has no notion of similarity.
- Bag of Words is simple but ignores order and meaning.
- n-grams use short context but scale poorly.
- Word embeddings place similar words close in vector space.
- Cosine similarity helps compare embeddings.
- Word2Vec learns embeddings from surrounding words.
- CBOW predicts target from context.
- Skip-Gram predicts context from target.
- Full softmax is expensive for large vocabularies.
- Negative sampling changes the task to binary neighbor vs non-neighbor prediction.
- SGNS is an efficient and important training method.
- Word2Vec is static and cannot fully handle multiple meanings of a word.
- RNNs extend the story to sequence modeling with shared parameters across timesteps.
