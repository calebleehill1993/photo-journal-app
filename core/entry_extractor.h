#ifndef ENTRY_EXTRACTOR_H
#define ENTRY_EXTRACTOR_H

#include <vector>
#include "entry.h"

class EntryExtractor {
public:
    virtual ~EntryExtractor() = default;
    virtual std::vector<Entry> extractEntries() = 0;
};

#endif // ENTRY_EXTRACTOR_H