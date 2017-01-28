#include "corpus.h"

namespace marian {
namespace data {

typedef std::vector<size_t> WordBatch;
typedef std::vector<float> MaskBatch;
typedef std::pair<WordBatch, MaskBatch> WordMask;
typedef std::vector<WordMask> SentBatch;

typedef std::vector<Words> SentenceTuple;

CorpusIterator::CorpusIterator() : pos_(-1) {}

CorpusIterator::CorpusIterator(Corpus& corpus)
 : corpus_(&corpus), pos_(0) {
  tup_ = corpus_->next();
}

void CorpusIterator::increment() {
  tup_ = corpus_->next();
  pos_++;
}

bool CorpusIterator::equal(CorpusIterator const& other) const
{
  return this->pos_ == other.pos_ ||
    (this->tup_.empty() && other.tup_.empty());
}

const SentenceTuple& CorpusIterator::dereference() const {
  return tup_;
}

Corpus::Corpus(const std::vector<std::string>& textPaths,
               const std::vector<std::string>& vocabPaths,
               const std::vector<int>& maxVocabs,
               size_t maxLength)
  : textPaths_(textPaths),
    maxLength_(maxLength)
{
  UTIL_THROW_IF2(textPaths.size() != vocabPaths.size(),
                 "Number of corpus files and vocab files does not agree");

  std::vector<Vocab> vocabs;
  for(int i = 0; i < vocabPaths.size(); ++i) {
    vocabs_.emplace_back(vocabPaths[i], maxVocabs[i]);
  }

  for(auto path : textPaths_) {
    files_.emplace_back(new InputFileStream(path));
  }

}

SentenceTuple Corpus::next() {
  bool cont = true;
  while(cont) {
    SentenceTuple tup;
    for(int i = 0; i < files_.size(); ++i) {
      std::string line;
      if(std::getline((std::istream&)*files_[i], line)) {
        Words words = vocabs_[i](line);
        if(words.empty())
          words.push_back(0);
        tup.push_back(words);
      }
    }
    cont = tup.size() == files_.size();
    if(cont && std::all_of(tup.begin(), tup.end(),
                           [=](const Words& words) {
                             return words.size() > 0 &&
                             words.size() <= maxLength_;
                            }))
      return tup;
  }
  return SentenceTuple();
}

void Corpus::shuffle() {
  shuffleFiles(textPaths_);
}

void Corpus::shuffleFiles(const std::vector<std::string>& paths) {
  std::cerr << "Shuffling files" << std::endl;
  std::vector<std::vector<std::string>> corpus;

  files_.clear();
  for(auto path : paths) {
    files_.emplace_back(new InputFileStream(path));
  }

  bool cont = true;
  while(cont) {
    std::vector<std::string> lines(files_.size());
    for(int i = 0; i < files_.size(); ++i) {
      cont = cont && std::getline((std::istream&)*files_[i],
                                  lines[i]);
    }
    if(cont)
      corpus.push_back(lines);
  }

  std::random_device rd;
  std::mt19937 g(rd());
  std::shuffle(corpus.begin(), corpus.end(), g);

  std::vector<UPtr<OutputFileStream>> outs;
  for(int i = 0; i < files_.size(); ++i) {
    auto path = files_[i]->path();
    outs.emplace_back(new OutputFileStream(path + ".shuf"));
  }
  files_.clear();

  for(auto& lines : corpus) {
    size_t i = 0;
    for(auto& line : lines) {
      (std::ostream&)*outs[i++] << line << std::endl;
    }

    std::vector<std::string> empty;
    lines.swap(empty);
  }

  for(int i = 0; i < outs.size(); ++i) {
    auto path = outs[i]->path();
    outs[i].reset();
    files_.emplace_back(new InputFileStream(path));
  }

  std::cerr << "Done" << std::endl;
}

}
}