# Topics from the Word Embeddings Slides  
_Based on the uploaded slide deck and the annotated notes._

## 1. Why text is hard for machine learning

Images are naturally numeric tensors, but text starts as symbols.  
A model cannot directly understand words like `king`, `apple`, or `bank`; it needs a numeric representation. The slides also emphasize that text is tricky because of:

- **Homonyms**: same surface word, different meanings  
  - _park_ in “parallel park” vs. _park_ in “walk in the park”
- **Typos**
- **Paraphrases / synonyms**
- **Word order**
  - “body pieces are not interchangeable” vs. “body pieces are interchangeable” mean very different things


### When this matters
This matters in almost every NLP system:
- search engines
- translation
- autocomplete
- summarization
- sentiment analysis
- question answering

A representation that only memorizes exact words will usually fail on synonymy, paraphrase, and context shifts.

---

## 2. Naive representation: one-hot vectors

One of the annotated pages stresses the “naive approach”: represent each vocabulary word as a **one-hot vector**. If the vocabulary has size \(|V|\), then each word gets a \(|V|\)-dimensional vector with one 1 and the rest 0s. The handwritten note directly points out that the vector size becomes the vocabulary size, and that this does **not** capture family/relation/similarity between words. 

The lecture slides also show that neural language models output a probability over the whole vocabulary, so vocabulary size is central to the computational cost.

### Why one-hot is bad
If `apple` and `orange` are both fruits, their one-hot vectors are still orthogonal.  
So the model sees them as completely different symbols unless it learns everything from scratch.

The annotations say essentially this in mixed Bangla-English: contextual similarity such as `apple` and `orange` cannot be expressed well with one-hot vectors. The notes also compare this to representing personality as numbers and then measuring similarity. 

### Good choice?
One-hot is okay only when:
- the vocabulary is tiny
- semantic similarity is unimportant
- you want a simple teaching example

### Bad choice?
Avoid it when:
- vocabulary is large
- you need semantic similarity
- you care about generalization to related words

---

## 3. Bag of Words (BoW)

The slide deck introduces **Bag of Words** for representing documents.  
It counts word occurrences but ignores structure. The slide explicitly lists two main drawbacks:

- **High dimensionality**
- **No semantic information** 

### Intuition
A document becomes a big count vector:
- how many times did “cat” appear?
- how many times did “movie” appear?
- how many times did “great” appear?

### Example scenario where BoW is a good choice
Bag of Words is a good choice when:
- you want a **quick baseline**
- interpretability is important
- the task depends mostly on **keyword presence**

Examples:
- spam detection
- simple sentiment classification on short reviews
- topic classification with limited resources

### When BoW is a poor choice
BoW struggles when:
- two documents mean the same thing but use different words
- word order matters
- you need semantic similarity

The “document similarity?” slide makes exactly this point by comparing:

- “Obama speaks to the media in Illinois”
- “The President greets the press in Chicago”

These documents may share little or no exact vocabulary, yet their meanings are related. 

---

## 4. Semantic similarity and cosine similarity

A central motivation in both files is:  
put words into vectors so that **similar words have similar vectors**, and measure this with **cosine similarity**. The slides explicitly say “Put words into vectors so we can measure the similarity between words” and “Use cosine similarity.”

The annotated article first explains this through a **personality vector** analogy:
- represent a person as a vector of numeric traits
- compare two people using cosine similarity
- more dimensions usually give a richer comparison

This is used to build intuition before moving to word vectors.

### Intuition
Cosine similarity measures the angle between two vectors:
- closer direction → more similar
- opposite direction → less similar

That is why `good` and `nice` can be close, while `bad` can point elsewhere in vector space, as shown in the slides. 

### Good choice?
Cosine similarity is a good choice when:
- vector magnitude is less important than direction
- comparing embeddings
- retrieving semantically similar words/documents

Examples:
- nearest-neighbor word lookup
- semantic search
- document retrieval
- clustering embeddings

---

## 5. What word embeddings are

The slide deck defines word embeddings as **vector representations of words that capture semantic relationships**. 

The annotated notes emphasize that a trained embedding is not manually designed feature engineering. Rather, the vector dimensions become **latent** properties learned from data. The notes also remark that although we cannot always interpret each dimension directly, the learned representation captures meaningful structure.

### Key idea
Instead of a sparse one-hot vector, each word gets a dense, low-dimensional vector like:
- `king`
- `queen`
- `man`
- `woman`

and those vectors reflect similarity and relation.

### Good choice?
Word embeddings are a good choice when:
- you need dense numerical input for neural models
- you want semantic similarity
- data contains many related words or paraphrases

Examples:
- search ranking
- recommendation
- intent classification
- language modeling
- downstream neural NLP pipelines

---

## 6. Why embeddings are better than one-hot

The slides say we need word embeddings because they provide:

- **Numerical input**
- **Similarity and distance information** 

The annotated notes reinforce this with the `king`, `man`, `woman` comparison and explicitly mention that trained embeddings capture closeness in a way one-hot cannot.

### Example
If a model learns that `cat` and `kitten` are close, then knowledge about one may transfer to the other.  
That is impossible with raw one-hot vectors unless the model separately learns everything.

### Good choice?
Embeddings are a strong default whenever:
- the vocabulary is moderate or large
- semantically related words should behave similarly
- you want a compact learned representation

---

## 7. Language modeling: predict the next word

The slides define **language modeling** as assigning probabilities to text and predicting the next word given the current text. They show the chain rule factorization of sequence probability.

The annotated notes pay special attention here. They describe next-token prediction using the example sentence “The quick brown fox jumps over …” and point out:

- the model predicts the next token
- the output is over the whole vocabulary
- if vocabulary size is very large, the output layer is also very large
- this motivates later efficiency improvements

These ideas are also supported by the article’s discussion of neural language models producing probabilities over all vocabulary words. 

### Intuition
Given:
> “I like cats because they look ...”

the model should give high probability to a word like `cute`.

### Good choice?
Language modeling is a good choice when:
- your goal is next-word prediction
- you want a generative model of text
- you want embeddings learned from raw text without manual labels

Examples:
- autocomplete keyboards
- speech recognition language models
- text generation systems
- pretraining for downstream tasks

---

## 8. n-gram language models

The slides introduce **n-gram models** as models that assume each word depends only on the previous \(n-1\) words. They count frequencies of n-grams and predict the next word from those counts.

They also point out the core tradeoff:
- larger \(n\) gives more context
- but the counting table grows rapidly and becomes expensive/sparse

### Intuition
- unigram: use just single-word frequencies
- bigram: depend on previous one word
- trigram: depend on previous two words

### Example scenario where n-grams are a good choice
n-grams are a good choice when:
- you need a simple baseline
- training data is limited
- interpretability matters
- deployment constraints are strict

Examples:
- tiny autocomplete prototypes
- classical ASR baselines
- lightweight educational/demo systems

### When n-grams are a poor choice
They are weak when:
- long-range context matters
- vocabulary is large
- you want semantic generalization

The Chomsky sentence discussion on the slide is there to provoke exactly that issue: a sentence can be grammatically plausible but statistically rare, or locally reasonable but globally odd.

---

## 9. Word2Vec: the main idea

The slides explicitly motivate Word2Vec with the principle:

> Similar words appear in similar contexts.

They use the “Underberg” example:
- “I love drinking Underberg after a meal.”
- “Underberg is quite strong.”
- “A few bottles of Underberg make me very drunk.”

From context alone, we can infer that `Underberg` is probably some alcoholic drink. This is distributional semantics in action. 

### Good choice?
Word2Vec is a good choice when:
- you have lots of unlabeled text
- you want efficient static word embeddings
- you care about semantic neighborhoods and analogies

Examples:
- pretraining embeddings for a classifier
- approximate semantic search
- recommendation-style sequence modeling
- exploratory analysis of word similarity

---

## 10. Two Word2Vec training styles: Skip-Gram and CBOW

The slide deck highlights the two classic architectures:

- **Skip-Gram**: use the center/target word to predict surrounding context words
- **CBOW (Continuous Bag of Words)**: use surrounding context words to predict the center/target word 

The recap slide also summarizes this cleanly. 

### 10.1 Skip-Gram

The slides show an example such as:

- sentence: “A cup of coffee is on the table”
- center word: `coffee`
- context words within a window: `cup`, `of`, `is`, `on`

Training samples become:
- (`coffee`, `cup`)
- (`coffee`, `of`)
- (`coffee`, `is`)
- (`coffee`, `on`) 

The annotations also describe skip-gram in mixed Bangla-English: given a center word, predict surrounding words; this creates multiple training examples from one window. They also note that the original softmax over all vocabulary words is expensive. 

#### Good choice scenario for Skip-Gram
Skip-Gram is often a good choice when:
- you care a lot about **word-level semantic quality**
- the dataset is reasonably large
- rare words matter more

Examples:
- building a domain lexicon from a large corpus
- learning embeddings from research papers
- mining semantic relations in specialized text

### 10.2 CBOW

CBOW reverses the direction:
- use the surrounding words to predict the center word

So from context words around `coffee`, predict `coffee`. The slides show this architecture directly. 

The annotations label this as “Continuous Bag of Words → CBOW” and contrast it with skip-gram. 

#### Good choice scenario for CBOW
CBOW is a good choice when:
- training speed matters
- the corpus is large
- you want a simpler and often faster approximation

Examples:
- fast embedding pretraining for a downstream model
- large-scale industrial preprocessing
- classroom demos where speed matters

---

## 11. Word2Vec architecture: one-hot → embedding → output probabilities

The slides walk through Skip-Gram as a small neural network:

1. **Input one-hot vector** for the target word  
2. Multiply by matrix **W** to select/get the word’s dense embedding  
3. Multiply by **W′** and apply **softmax** over the vocabulary  
4. Compare with the true context word using **cross-entropy loss**

One slide specifically asks what happens when a one-hot vector multiplies \(W\): it simply picks out the row for that target word. That row is the embedding of the word. 

The annotations say the same thing in a more intuitive way:
- one-hot input goes through a projection
- the hidden layer is the embedding vector
- then it is projected back to vocabulary size
- cross-entropy loss is used
- gradients update the embedding matrix 

### Why this is important
This explains where embeddings come from:
they are not magic tables; they are **learned parameters** inside a prediction model.

### Good choice?
This architecture is good pedagogically when:
- teaching how embeddings are learned
- explaining why one-hot can still lead to dense vectors
- connecting NLP with standard neural network training

---

## 12. Softmax bottleneck and why it is expensive

The annotated pages pay special attention to a computational bottleneck:
to predict a word over a vocabulary of size \(|V|\), the model needs an output score for every word, then softmax over all of them. The notes explicitly mention that when vocab is very large, this becomes time-consuming.

The article also states that the “project to output vocabulary” step is expensive because it must be done for huge numbers of training samples.

### Why it hurts
If \(|V| = 100,000\), each training step needs scoring across 100,000 candidates.

### Good choice?
Full softmax is still reasonable when:
- vocabulary is small
- you need exact normalized probabilities
- efficiency is not the top concern

### Poor choice?
Avoid full softmax when:
- vocabulary is huge
- training corpus is massive
- you mainly want embeddings, not exact next-word probabilities

---

## 13. Negative sampling

This is one of the most important annotated ideas.

The article explains that instead of predicting the exact next/context word among all vocabulary words, we change the task:

- given an **input word** and an **output word**
- predict whether they are true neighbors (**1**) or not (**0**)

This avoids the expensive full-vocabulary softmax. 

The annotated notes summarize this very directly:
- don’t softmax across every token
- use **sigmoid**
- create binary labels 0/1
- add **negative samples**
- otherwise the model could trivially predict 1 for everything and learn nothing

The article then explains exactly that loophole and how negative examples fix it. 

### Intuition
Positive example:
- (`coffee`, `cup`) → 1

Negative example:
- (`coffee`, `democracy`) → 0

Now the model learns which word pairs belong together.

### Good choice scenario
Negative sampling is a very good choice when:
- vocabulary is large
- training speed matters
- your main goal is high-quality embeddings

Examples:
- Word2Vec on large corpora
- industrial-scale embedding training
- recommendation systems using item-context pairs

---

## 14. SGNS: Skip-Gram with Negative Sampling

The article names the combination explicitly:
- **Skip-Gram**
- **Negative Sampling**
- together → **SGNS** 

This is the classic practical Word2Vec recipe.

### Why it became popular
Because it balances:
- semantic quality
- speed
- simplicity

### Good choice scenario
Use SGNS when:
- you want static embeddings
- you have plenty of raw text
- you need something lighter than modern transformers

Examples:
- building embeddings for Bangla news articles
- training embeddings for a university corpus
- bootstrap features for a downstream classifier

---

## 15. Analogy structure in embedding space

The annotated article highlights the famous analogy behavior:

`king - man + woman ≈ queen`

and explains that the nearest vector by cosine similarity is often `queen`.

This does **not** mean embeddings understand human language perfectly.  
It means certain relations become approximately linear in the learned vector space.

### Intuition
Some semantic relations are represented as directions:
- man → woman
- king → queen

### Good choice?
Analogy-style reasoning is useful for:
- qualitative evaluation of embeddings
- exploratory analysis
- teaching vector arithmetic intuition

### Not a strong production metric
Do not rely only on analogies to evaluate embeddings for real tasks.

---

## 16. Window size and what type of similarity you get

The article notes that **window size** changes what “similarity” means:

- smaller windows tend to capture **interchangeability**
- larger windows tend to capture broader **relatedness** 

### Examples
Small window:
- `doctor` and `physician` may be close

Larger window:
- `doctor` and `hospital` may become closer as related concepts

### Good choice scenario
Choose a smaller window when:
- synonym-like similarity matters
- substitution behavior matters

Choose a larger window when:
- topic/association matters
- related concepts are more useful than near-synonyms

---

## 17. Document similarity and Word Mover’s Distance

The slides introduce **Word Mover’s Distance (WMD)** as a way to measure similarity between documents using distances in word embedding space. 

### Intuition
Instead of exact word overlap, ask:
how much “movement” is needed to transform the words in one document to words in another in embedding space?

### Good choice scenario
WMD is a good choice when:
- exact word overlap is low
- semantic similarity matters
- documents are short to medium length

Examples:
- semantic retrieval
- duplicate question detection
- clustering short documents with paraphrasing

### Less ideal when
It can be computationally heavier than simpler cosine-based document embeddings.

---

## 18. X2Vec family: beyond words

The slides generalize the idea into “X2vec”:
- Word2Vec
- Doc2Vec
- Node2Vec
- Item2Vec
- Sent2Vec

### Core idea
You can often learn embeddings for any entity if you can define a notion of **context** or **co-occurrence**.

### Example scenarios
- **Doc2Vec**: represent a paragraph/document for classification or retrieval
- **Node2Vec**: graph node embeddings from graph neighborhoods
- **Item2Vec**: recommendation from co-purchased or co-viewed items
- **Sent2Vec**: sentence-level representation

---

## 19. Limitations of Word2Vec and static embeddings

The slides explicitly mention several limitations:

1. **Multiple meanings collapse into one vector**
   - `bank` of a river vs. `bank` as a financial institution
2. **Limited context**
   - only local context window is used
3. **Bias**
   - embeddings can capture societal biases
4. **Time dependence**
   - meaning changes over time 

### Why this matters
A static embedding gives one vector per word type, not per usage.

### Example scenario where Word2Vec is not a good choice
Avoid relying only on Word2Vec when:
- context-specific meaning matters
- ambiguity is common
- fairness concerns are central
- domain meaning shifts over time

Examples:
- legal NLP
- medical NLP
- conversational assistants
- modern QA systems

---

## 20. Bridge to sequence models: RNNs

After static word embeddings, the slides transition to **RNNs**.  
The recap slide says RNNs:
- have an internal state (memory)
- can handle arbitrary sequences of inputs
- are trained with backpropagation through time

The later slides motivate parameter sharing across timesteps because otherwise:
- too many parameters are needed
- long sequences become hard to train
- variable-length sequences are awkward 

### Why this transition matters
Static embeddings give you a vector per word.  
RNNs try to model the **sequence**, not just isolated word relationships.

### Good choice scenario
RNNs are a good choice when:
- order matters
- you need sequence modeling
- inputs have variable length

Examples:
- text classification over sequences
- sequence labeling
- older-generation language models
- speech and time-series modeling

---

## 21. Special notes from the annotations (Bangla + English mixed)

A few especially important annotation-driven takeaways from the handwritten notes:

### a) “One-hot is too sparse”
The notes stress that one-hot vectors become huge and do not encode similarity.  
This is one of the biggest motivations for embeddings.

### b) “Think in terms of similarity”
The notes repeatedly relate word vectors to personality vectors and cosine similarity.  
That is a very good intuition: embeddings are useful because similar things become numerically close. 

### c) “The hidden layer is the embedding”
In the Skip-Gram architecture notes, the middle projection is essentially the learned dense representation.

### d) “Softmax over full vocab is the bottleneck”
This is one of the most emphasized handwritten remarks, and it directly motivates negative sampling. 

### e) “Need negative samples”
The notes explain that if you only train on positive neighbors, the model can cheat.  
So negative examples are necessary to make the task meaningful.

---

## 22. Quick “which method should I choose?” guide

### Choose **Bag of Words** when:
- you need a fast, simple baseline
- interpretability matters
- exact words matter more than deep semantics

### Choose **n-gram models** when:
- you want a small classical language model
- local context is enough
- you need a teaching/demo baseline

### Choose **Word2Vec / embeddings** when:
- semantic similarity matters
- you need dense numeric features
- you have lots of unlabeled text

### Choose **Skip-Gram** when:
- word quality matters
- rare words matter more
- you can afford a bit more training

### Choose **CBOW** when:
- speed matters
- you want fast embedding training
- you mainly want a decent approximation

### Choose **Negative Sampling / SGNS** when:
- vocabulary is large
- full softmax is too expensive
- your goal is efficient embedding learning

### Choose **RNNs** when:
- sequence order matters
- inputs have variable length
- you need context that evolves over time

---

## 23. Final takeaway

The uploaded slides and annotations together build a very clear story:

1. Text is hard because exact word matching is not enough.  
2. One-hot and bag-of-words are simple but limited.  
3. We want dense vectors where similar words are close.  
4. Word2Vec learns such vectors from context.  
5. The two main training styles are **Skip-Gram** and **CBOW**.  
6. Full-vocabulary softmax is expensive, so **negative sampling** gives a practical speedup.  
7. These embeddings are useful, but still limited because they are static and local-context-based.  
8. That limitation motivates later sequence/contextual models like **RNNs** and, beyond this lecture, transformers.

---
